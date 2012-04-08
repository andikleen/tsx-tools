/* Skip SIGSEGVs on XEND for broken programs that free unlocked locks */
/* gcc -fPIC -shared -g -o ignore-xend.so ignore-xend.c */
#define _GNU_SOURCE 1
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

volatile int bad_xends;

static void handler(int sig, siginfo_t *si, void *ctx_ptr)
{
	ucontext_t *ctx = ctx_ptr;
	uint64_t addr = (uint64_t) ctx->uc_mcontext.gregs[REG_RIP];

	if (ctx->uc_mcontext.gregs[REG_TRAPNO] != 13)
		return;
	/* Avoid nested non GP faults */
	if (addr >> 48 != 0 && addr >> 48 != 0xffff)
		return;
	uint8_t *ip = (void *) ctx->uc_mcontext.gregs[REG_RIP];
	if (ip[0] == 0x0f && ip[1] == 0x01 && ip[2] == 0xd5) { /* XEND */
		__sync_fetch_and_add(&bad_xends, 1);
		ctx->uc_mcontext.gregs[REG_RIP] += 3;
	}
}

static void end_count(void)
{
	if (bad_xends > 0) 
		printf("%d bad xends\n", bad_xends);
}

static void __attribute__((constructor)) init_sig(void)
{
	struct sigaction sa = { 
		.sa_sigaction = handler,
		.sa_flags = SA_SIGINFO
	};
	atexit(end_count);
	sigaction(SIGSEGV, &sa, NULL);
}
