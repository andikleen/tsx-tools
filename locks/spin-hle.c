/*
 * Copyright (c) 2016 Intel Corporation
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
/* Simple HLE spinlock */

#include "hle-emulation.h"

void spin_lock_hle(int *lock)
{
	while ((int)__hle_acquire_sub_fetch4((unsigned *)lock, 1) < 0) {
		do
			asm volatile("pause":::"memory");
		while (*lock!= 1);
	}
}

void spin_unlock_hle(int *lock)
{
	__hle_release_store_n4((unsigned *)lock, 1);
}
