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

/* Simple non-adaptive, non backoff RTM lock. */
#include "rtm.h"
#include "spin.h"

#define pause() asm volatile("pause" ::: "memory")

#define RETRY_CON 3
#define RETRY_CAP 1
#define RETRY_OTHER 3

void __attribute__((noinline, weak)) trace_abort(unsigned status) {}

static inline int lock_is_free(int *lock)
{
	return *lock == 1;
}

void spin_lock_rtm(int *lock)
{
	int i;
	unsigned status;
	unsigned retry = RETRY_OTHER;

	for (i = 0; i < retry; i++) {
		if ((status = _xbegin()) == _XBEGIN_STARTED) {
			if (lock_is_free(lock))
				return;
			_xabort(0xff);
		}
		trace_abort(status);
		if ((status & _XABORT_EXPLICIT) && _XABORT_CODE(status) == 0xff) {
			while (!lock_is_free(lock))
				pause();
		} else if (!(status & _XABORT_RETRY) && !(status & _XABORT_CAPACITY))
			break;

		if (status & _XABORT_CONFLICT) {
			retry = RETRY_CON;
			while (!lock_is_free(lock))
				pause();
			/* Could do various kinds of backoff here. */
		} else if (status & _XABORT_CAPACITY) {
			retry = RETRY_CAP;
		} else {
			retry = RETRY_OTHER;
		}
	}
	/* Could do adaptation here */

	while (__sync_sub_and_fetch(lock, 1) < 0) {
		do
			pause();
		while (!lock_is_free(lock));
		/* Could do respeculation here */
	}
}

void spin_unlock_rtm(int *lock)
{
	if (lock_is_free(lock))
		_xend();
	else
		*lock = 1;
}

/* Add trylock */
