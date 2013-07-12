#ifndef TSX_CPUID_H
#define TSX_CPUID_H 1
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

/* Determine if the current CPU has HLE or RTM. Require a recent gcc
   with cpuid.h header. For older gccs it is possible to just copy the header
   from a newer installation. */

#include <cpuid.h>
#include <stddef.h>

#define CPUID_RTM (1 << 11)
#define CPUID_HLE (1 << 4)

static inline int cpu_has_rtm(void)
{
    if (__get_cpuid_max(0, NULL) >= 7) {
        unsigned a, b, c, d;
        __cpuid_count(7, 0, a, b, c, d);
        return !!(b & CPUID_RTM);
    }
    return 0;
}

static inline int cpu_has_hle(void)
{
    if (__get_cpuid_max(0, NULL) >= 7) {
        unsigned a, b, c, d;
        __cpuid_count(7, 0, a, b, c, d);
        return !!(b & CPUID_HLE);
    }
    return 0;
}

#endif
