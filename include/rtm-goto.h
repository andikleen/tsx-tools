#ifndef RTM_GOTO_H
#define RTM_GOTO_H 1

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

This file provide an alternative RTM intrinsics implementation using
the "asm goto" gcc extension. This will only work on gcc 4.7+ or
gcc 4.6 with backport (Fedora). It exposes the jump to the abort
handler to the programmer, which can save a few instructions.

XBEGIN(label)

Start a transaction. When an abort happens jump to the XFAIL* defined
by "label".

When the CPU does not support transactions this will unconditionally
jump to label.

XEND()

End a transaction. Must match a XBEGIN().

XFAIL(label)

Begin a fallback handler that is jumped too from XBEGIN in case of a 
transaction abort. Must be in the same function as the XBEGIN().
XFAIL acts like a label.

Note: caller must ensure to not execute the code when not jumped too, e.g.
by placing an if (0) around it, unless this is intended.
The fail handler should not execute XEND() for the transaction.

XFAIL_STATUS(label, status)

Same as XFAIL(), but supply a status code containing the reason 
for the abort. See below for the valid status bits.

XABORT(x) 

Abort a transaction.

*/

#define XABORT(status) asm volatile(".byte 0xc6,0xf8,%P0" :: "i" (status))
#define XBEGIN(label)	\
     asm volatile goto(".byte 0xc7,0xf8 ; .long %l0-1f\n1:" ::: "eax" : label)
#define XEND()	  asm volatile(".byte 0x0f,0x01,0xd5")
#define XFAIL(label) label: asm volatile("" ::: "eax")
#define XFAIL_STATUS(label, status) label: asm volatile("" : "=a" (status))

#include "rtm.h"
#define XTEST() _xtest()

/* Status bits */
#define XBEGIN_STARTED		(~0u)
#define XABORT_EXPLICIT	(1 << 0)
#define XABORT_RETRY		(1 << 1)
#define XABORT_CONFLICT	(1 << 2)
#define XABORT_CAPACITY	(1 << 3)
#define XABORT_DEBUG		(1 << 4)
#define XABORT_NESTED		(1 << 5)
#define XABORT_CODE(x)		(((x) >> 24) & 0xff)

#endif
