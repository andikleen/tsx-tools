#ifndef HLE_MS_H
#define HLE_MS_H 1

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

/*
 * Please note HLE is deprecated. Please switch to RTM.
 */

/* 
   Microsoft compiler compatible HLE intrinsics for gcc.

   The *64 variants are currently only implemented for 64bit code.

*/
 
#define __hle_force_inline __attribute__((always_inline)) inline
#define __HLE_ACQUIRE ".byte 0xf2 ; " 
#define __HLE_RELEASE ".byte 0xf3 ; " 

/* May be worth changing on a LP64 system like real Win64 */
#define __hle_int int
#define __hle_int64 long long

/* XXX returns old or new? */

/* We use int instead of long to provide a 32bit version on 64bit
   (gcc/Linux is normally LLP64 instead of LP64) */

static __hle_force_inline __hle_int
_InterlockedCompareExchange_HLEAcquire(__hle_int volatile *ptr,
				       __hle_int newv, __hle_int oldv)
{
	asm volatile(__HLE_ACQUIRE " ; lock ; cmpxchg %2,%1"
		     : "+a" (oldv), "+m" (*ptr)
		     : "r" (newv)
		     : "memory");
	return oldv;
}

static __hle_force_inline __hle_int
_InterlockedCompareExchange_HLERelease(__hle_int volatile *ptr,
				       __hle_int newv, __hle_int oldv)
{
	asm volatile(__HLE_RELEASE " ; lock ; cmpxchg %2,%1"
		     : "+a" (oldv), "+m" (*ptr)
		     : "r" (newv)
		     : "memory");
	return oldv;
}

#if __SIZEOF_POINTER__ == 8

/* XXX implement 32bit version */

static __hle_force_inline __hle_int64
_InterlockedCompareExchange64_HLEAcquire(__hle_int64 volatile *ptr,
				         __hle_int64 newv, __hle_int64 oldv)
{
	asm volatile(__HLE_ACQUIRE " ; lock ; cmpxchg %2,%1"
		     : "+a" (oldv), "+m" (*ptr)
		     : "r" (newv)
		     : "memory");
	return oldv;
}

static __hle_force_inline __hle_int64
_InterlockedCompareExchange64_HLERelease(__hle_int64 volatile *ptr,
				         __hle_int64 newv, __hle_int64 oldv)
{
	asm volatile(__HLE_RELEASE " ; lock ; cmpxchg %2,%1"
		     : "+a" (oldv), "+m" (*ptr)
		     : "r" (newv)
		     : "memory");
	return oldv;
}

#endif

static __hle_force_inline void *
_InterlockedCompareExchangePointer_HLEAcquire(void * volatile *ptr,
					      void *newv, void *oldv)
{
	asm volatile(__HLE_ACQUIRE " ; lock ; cmpxchg %2,%1"
		     : "+a" (oldv), "+m" (*ptr)
		     : "r" (newv)
		     : "memory");
	return oldv;
}

static __hle_force_inline void *
_InterlockedCompareExchangePointer_HLERelease(void *volatile *ptr,
					      void *newv, void *oldv)
{
	asm volatile(__HLE_RELEASE " ; lock ; cmpxchg %2,%1"
		     : "+a" (oldv), "+m" (*ptr)
		     : "r" (newv)
		     : "memory");
	return oldv;
}

static __hle_force_inline __hle_int
_InterlockedExchangeAdd_HLEAcquire(__hle_int volatile *ptr, __hle_int val)
{
	asm volatile(__HLE_ACQUIRE " ; lock ; xadd %0,%1" 
		     : "+q" (val), "+m" (*ptr) :: "memory");
	return val;
}

static __hle_force_inline __hle_int
_InterlockedExchangeAdd_HLERelease(__hle_int volatile *ptr, __hle_int val)
{
	asm volatile(__HLE_RELEASE " ; lock ; xadd %0,%1" 
		     : "+q" (val), "+m" (*ptr) :: "memory");
	return val;
}

#if __SIZEOF_POINTER__ == 8

static __hle_force_inline __hle_int64
_InterlockedExchangeAdd64_HLEAcquire(__hle_int64 volatile *ptr, 
				     __hle_int64 val)
{
	asm volatile(__HLE_ACQUIRE " ; lock ; xadd %0,%1" 
		     : "+q" (val), "+m" (*ptr) :: "memory");
	return val;
}

static __hle_force_inline __hle_int64
_InterlockedExchangeAdd64_HLERelease(__hle_int64 volatile *ptr, 
				     __hle_int64 val)
{
	asm volatile(__HLE_RELEASE " ; lock ; xadd %0,%1" 
		     : "+q" (val), "+m" (*ptr) :: "memory");
	return val;
}

#endif

static __hle_force_inline void
_Store_HLERelease(__hle_int volatile *ptr, __hle_int val)
{
	asm volatile(__HLE_RELEASE " ; mov %1,%0" 
		     : "+m" (*ptr) : "q" (val));
}

#if __SIZEOF_POINTER__ == 8

static __hle_force_inline void
_Store64_HLERelease(__hle_int64 volatile *ptr, __hle_int64 val)
{
	asm volatile(__HLE_RELEASE " ; mov %1,%0" 
		     : "+m" (*ptr) : "q" (val));
}

#endif

static __hle_force_inline void
_StorePointer_HLERelease(void  * volatile *ptr, void *val)
{
	asm volatile(__HLE_RELEASE " ; mov %1,%0" 
		     : "+m" (*ptr) : "q" (val));
}

#endif
