/* LD_PRELOAD module for glibc to tune elision configuration. */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fnmatch.h>
#include <stddef.h>
#include <cpuid.h>
#include <stdbool.h>

#include "sym.h"

/* Find file name of pthread linked to this process plus base address
 * of the writable mapping (which is different from the executable mapping) */
static char *find_pthread(unsigned long *laddr)
{
	FILE *f = fopen("/proc/self/maps", "r");
	char *line = NULL;
	size_t linelen = 0;
	char map[100];
	char *res = NULL;

	while (getline(&line, &linelen, f) > 0) {
		char perm[10];
		unsigned long offset;

		if (sscanf(line, "%lx-%*x %10s %lx %*s %*d %100s", laddr, perm, &offset, map) == 4) {
			if (strcmp(perm, "rw-p"))
				continue;
			if (fnmatch("*/libpthread*.so", map, 0) == 0) {
				*laddr -= offset;
				res = strdup(map);
				break;
			}
		}
	}
	free(line);
	fclose(f);
	return res;
}

struct lsym symbols[] = {
	{ "__elision_aconf", },
	{ "__pthread_force_elision", },
	{}
};

/* Must match table above */
enum {
	SYM_ELISION_ACONF = 0,
	SYM_PTHREAD_FORCE_ELISION,
};

struct lsym rwsymbols[] = {
	{ "__rwlock_rtm_enabled", },
	{ "__rwlock_rtm_read_retries", },
	{}
};

/* Must match table above */
enum {
	SYM_RWLOCK_RTM_ENABLED = 0,
	SYM_RWLOCK_RTM_READ_RETRIES,
};

struct elision_config {
	int skip_lock_busy;
	int skip_lock_internal_abort;
	int retry_try_xbegin;
	int skip_trylock_internal_abort;
};

struct tune {
	const char *name;
	unsigned offset;
	int len;
};

#define FIELD(x) { #x, offsetof(struct elision_config, x), sizeof(#x)-1 }

static const struct tune tunings[] = {
	FIELD(skip_lock_busy),
	FIELD(skip_lock_internal_abort),
	FIELD(retry_try_xbegin),
	FIELD(skip_trylock_internal_abort),
	{}
};

static void print_tunings(struct elision_config *ec, const struct tune *tu)
{
	int i;
	for (i = 0; tu[i].name; i++) {
		const struct tune *t = &tu[i];
		printf("%s: %d\n", t->name, ((int *)((char *)ec))[i]);
	}
}

static void aconf_setup (const char *s, struct elision_config *ec)
{
	int i;

	while (*s) {
		for (i = 0; tunings[i].name != NULL; i++) {
			int nlen = tunings[i].len;

			if (strncmp (tunings[i].name, s, nlen) == 0) {
				char *end;
				int val;

				if (s[nlen] != '=') {
					fprintf(stderr,
					"invalid GLIBC_PTHREAD_MUTEX syntax: missing =\n");
					return;
				}
				s += nlen + 1;
				val = strtoul (s, &end, 0);
				if (end == s) {
					fprintf(stderr,
					"invalid GLIBC_PTHREAD_MUTEX syntax: missing number\n");
					return;
				}
				*(int *)(((char *)ec) + tunings[i].offset) = val;
				s = end;
				if (*s == ',' || *s == ':')
					s++;
				else if (*s) {
					fprintf(stderr,
					"invalid GLIBC_PTHREAD_MUTEX syntax: garbage after number\n");
					return;
				}
				break;
			}
		}
		if (tunings[i].name == NULL) {
			fprintf(stderr, "invalid GLIBC_PTHREAD_MUTEX syntax: unknown tunable\n");
			return;
		}
	}
}

#define CPUID_RTM (1 << 11)
#define CPUID_HLE (1 << 4)

static inline int cpu_has_rtm(void)
{
	if (__get_cpuid_max(0, NULL) >= 7) {
		unsigned a, b, c, d;
		__cpuid_count(7, 0, a, b, c, d);
		return !!(b & CPUID_RTM);
	}
	return 0;
}

static int elision_available;

static void tune_mutex(struct elision_config *ec, char *s)
{
	if (!strncmp(s, "elision", 7) && (s[7] == 0 || s[7] == ':')) {
		*(int *)(symbols[SYM_PTHREAD_FORCE_ELISION].addr) = elision_available;
		if (s[7] == ':')
			aconf_setup(s + 8, ec);
	} else if (!strcmp(s, "none") || !strcmp(s, "no_elision")) {
		*(int *)(symbols[SYM_PTHREAD_FORCE_ELISION].addr) = 0;
	} else {
		fprintf(stderr, "Unknown setting for GLIBC_PTHREAD_MUTEX\n");
	}
}

static void tune_rwlock(const char *s)
{
	if (strncmp (s, "elision", 7) == 0) {
		*(int *)rwsymbols[SYM_RWLOCK_RTM_ENABLED].addr = elision_available;
		if (s[7] == ':') {
			char *end;
			int n;

			n = strtoul (s + 8, &end, 0);
			if (end == s + 8)
				fprintf(stderr,
					"Bad retry number for GLIBC_PTHREAD_RWLOCK\n");
			else
				*(int *)rwsymbols[SYM_RWLOCK_RTM_READ_RETRIES].addr = n;
		}
	} else if (strcmp (s, "none") == 0 || strcmp (s, "no_elision") == 0)
		*(int *)rwsymbols[SYM_RWLOCK_RTM_ENABLED].addr = 0;
	else
		fprintf(stderr, "Unknown setting for GLIBC_PTHREAD_RWLOCK\n");
}

static void print_symbols(struct lsym *sym)
{
	for (; sym->name; sym++)
		if (sym->addr)
			printf("%s: %d\n", sym->name, *(int *)(sym->addr));
}

static void __attribute__((constructor)) start(void)
{
	unsigned long baseaddr;
	char *pt = find_pthread(&baseaddr);
	bool verbose = (getenv("GLIBC_PTHREAD_VERBOSE") != NULL);
	struct elision_config *ec = NULL;

	elision_available = cpu_has_rtm();

	if (find_symbol(pt, symbols, baseaddr) < 0) {
		fprintf(stderr, "Cannot find elision symbols: ");
		struct lsym *ls;
		for (ls = symbols; ls->name; ls++)
			if (!ls->addr)
				fprintf(stderr, "%s ", ls->name);
		fprintf(stderr, "\n");
		goto out;
	}

	ec = (struct elision_config *) symbols[SYM_ELISION_ACONF].addr;

	char *mutex = getenv("GLIBC_PTHREAD_MUTEX");
	if (mutex) {
		tune_mutex(ec, mutex);
	}

	if (verbose) {
		print_symbols(symbols + 1); // skip aconf
		print_tunings(ec, tunings);
	}

	if (find_symbol(pt, rwsymbols, baseaddr) < 0) {
		/* Ignore to handle 2.18 without rwlock elision */
		goto out;
	}

	char *rwlock = getenv("GLIBC_PTHREAD_RWLOCK");
	if (rwlock)
		tune_rwlock(rwlock);
	if (verbose)
		print_symbols(rwsymbols);
out:
	free(pt);
}

#ifdef TESTMAIN
int main(void)
{
	unsigned long baseaddr;
	char *pt = find_pthread(&baseaddr);
	printf("pt: %s\n", pt);
	if (find_symbol(pt, symbols, baseaddr) < 0)
		printf("find_symbol failed\n");
	struct elision_config *ec = (struct elision_config *) symbols[SYM_ELISION_ACONF].addr;
	if (ec)
		printf("skip_lock_busy %d\n", ec->skip_lock_busy);
	if (symbols[SYM_PTHREAD_FORCE_ELISION].addr)
		printf("__pthread_force_elision %d\n", *(int *)symbols[SYM_PTHREAD_FORCE_ELISION].addr);
	if (find_symbol(pt, rwsymbols, baseaddr) >= 0) {
		printf("rwlock_rtm_enabled %d\n", *(int *)rwsymbols[SYM_RWLOCK_RTM_ENABLED].addr);
		printf("rwlock_rtm_read_replies %d\n", *(int *)rwsymbols[SYM_RWLOCK_RTM_READ_RETRIES].addr);
	}
	free(pt);
	return 0;
}
#endif
