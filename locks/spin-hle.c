/* Author: Andi Kleen */
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
