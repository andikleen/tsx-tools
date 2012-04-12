#define _GNU_SOURCE 1
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include "hle.h"

#define PAIR(x) x, sizeof(x) - 1

typedef void (*tsx_assert_hook_t)(unsigned);
extern tsx_assert_hook_t __tsx_set_abort_hook(tsx_assert_hook_t);

extern char *txn_assert_table[];
extern int txn_assert_table_size;

void txn_assert_abort_hook(unsigned status)
{
	char *msg;
	if ((status & XABORT_EXPLICIT_ABORT) && 
		XABORT_STATUS(status) < txn_assert_table_size) {
        	write(2, PAIR("txn assert failure at "));
		msg = txn_assert_table[XABORT_STATUS(status)],
		write(2, msg, strlen(msg));
		write(2, "\n", 1);
		abort();
	} 
}

typedef void (*abort_hook)(unsigned);
extern abort_hook __tsx_set_abort_hook(abort_hook);

static void __attribute__((constructor)) init_txn_assert(void)
{
#if 0
	__tsx_set_abort_hook(txn_assert_abort_hook);
#else
	abort_hook (*set_abort_hook)(abort_hook) = 
		dlsym(RTLD_DEFAULT, "__set_abort_hook");
	if (!set_abort_hook) {
		write(2, PAIR("__set_abort_hook not supported by glibc\n"));
		return;
	}
	set_abort_hook(txn_assert_abort_hook);
#endif
}

