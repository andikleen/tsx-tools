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

#define _GNU_SOURCE 1
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include "rtm.h"

#define PAIR(x) x, sizeof(x) - 1

typedef void (*tsx_assert_hook_t)(unsigned);
extern tsx_assert_hook_t __tsx_set_abort_hook(tsx_assert_hook_t);

extern char *txn_assert_table[];
extern int txn_assert_table_size;

void txn_assert_abort_hook(unsigned status)
{
	char *msg;
	if ((status & _XABORT_EXPLICIT) && 
		_XABORT_CODE(status) < txn_assert_table_size) {
        	write(2, PAIR("txn assert failure at "));
		msg = txn_assert_table[_XABORT_CODE(status)],
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
		write(2, PAIR("__set_abort_hook not supported by lock library\n"));
		return;
	}
	set_abort_hook(txn_assert_abort_hook);
#endif
}

