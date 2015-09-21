#ifndef _RTM_H
#define _RTM_H 1

/*
 * Copyright (c) 2013 Intel Corporation
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

/* Official RTM intrinsics interface matching gcc/icc, but works
   on older gcc compatible compilers and binutils.

   This is a special version of the intrinsics that automatically
   get patched in and out at runtime, depending on the system supporting
   TSX. When it does not support TSX the intrinsics become no-ops.
   _xbegin() always jumps to the abort handler. _xtest() always returns
   false. _xabort() are noops. _xend() is not patched, because it should
   not executed in the fallback path.

   tsx-patch.o needs to be linked into each individual shared library
   using this. The code is self modifying, so the code pages will
   not be shared. Self-modifying code has some disadvantages, as it
   can make debugging harder (the debugger may not see the code that
   actually runs). Normally TSX code is quite limited though, so
   this should not be a significant problem. */

#define _XBEGIN_STARTED		(~0u)
#define _XABORT_EXPLICIT	(1 << 0)
#define _XABORT_RETRY		(1 << 1)
#define _XABORT_CONFLICT	(1 << 2)
#define _XABORT_CAPACITY	(1 << 3)
#define _XABORT_DEBUG		(1 << 4)
#define _XABORT_NESTED		(1 << 5)
#define _XABORT_CODE(x)		(((x) >> 24) & 0xff)

#define _XBEGIN_SOFTWARE        -2 /* non architectural */

#define ASM_NOP3 " .byte 0x8d,0x76,0x00"
#define ASM_NOP4 " .byte 0x8d,0x74,0x26,0x00"
#define ASM_NOP6 " .byte 0x8d,0xb6,0x00,0x00,0x00,0x00"

#define __rtm_force_inline __attribute__((__always_inline__)) inline

#if __SIZEOF_POINTER__ == 8
#define ASM_POINTER(x) ".quad " x "; "
#else
#define ASM_POINTER(x) ".long " x "; "
#endif

#define ORIG_CODE "0: "
#define ALT_CODE  				\
	"\n1:\n\t"  				\
	".section \".text.rtmpatch\" ; "	\
	ASM_POINTER("0b") ASM_POINTER("3f") ASM_POINTER("1b-0b") \
	".previous ; " \
        ".section \".text.rtmalt\",\"ax\" ; 3: "
#define ALT_CODE_END "\n\t.previous"

static __rtm_force_inline int _xbegin(void)
{
	int ret;
	asm volatile(
		ORIG_CODE
		"mov %1,%0 ; " ASM_NOP6
		ALT_CODE
		"mov %2,%0 ; " 
		".byte 0xc7,0xf8 ; .long 0 # XBEGIN 0"
		ALT_CODE_END 
		: "=a" (ret) : "i" (_XBEGIN_SOFTWARE), "i" (_XBEGIN_STARTED) 
		: "memory");
	return ret;
}

static __rtm_force_inline void _xend(void)
{
	/* Not patched, as it should not be executed outside transactions. */
	asm volatile (".byte 0x0f,0x01,0xd5" ::: "memory");
}

static __rtm_force_inline void _xabort(const unsigned int status)
{
	asm volatile(ORIG_CODE
		     ASM_NOP3
		     ALT_CODE
		     ".byte 0xc6,0xf8,%P0" 
		     ALT_CODE_END :: "i" (status) : "memory");
}

static __rtm_force_inline int _xtest(void)
{
	unsigned char out;
	asm volatile(ORIG_CODE
		     "xor %0,%0 ; " ASM_NOP4
		     ALT_CODE
		     ".byte 0x0f,0x01,0xd6 ; setnz %0" 
		     ALT_CODE_END
		     : "=r" (out)
		     :: "memory");
	return out;
}

#endif
