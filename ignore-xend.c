/*
 * Copyright (c) 2012,2013 Intel Corporation
 * Author: Andi Kleen
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

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
