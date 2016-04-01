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
#ifdef __cplusplus
extern "C" {
#endif

#define SPIN_LOCK_INIT 1

extern void spin_init_hle(int *);
extern void spin_lock_hle(int *);
extern void spin_unlock_hle(int *);

extern void spin_init_rtm(int *);
extern void spin_lock_rtm(int *);
extern void spin_unlock_rtm(int *);

#ifdef __cplusplus
}
#endif
