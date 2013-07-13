#ifndef TSX_ASSERT_H
#define TSX_ASSERT_H 1

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

/* TSX aware assert
 * 
 * Normal assert does not work in TSX transaction. The assert output
 * is an IO operation, which causes an abort so the assert gets
 * discarded (unless it happens again when re-executed non transactionally)
 * 
 * This adds a TSX aware assert. It only works with RTM, not with HLE.
 * 
 * Operation is like normal assert.
 * Link the tsx-assert.o file to the program.
 *
 * The basic idea is to force all transactions to commit before doing the
 * the assert.
 *
 * Based on a idea by Torvald Riegel.
 */

extern void tsx_assert_fail(const char *file, int line, const char *expr);
#define tsx_assert(x) \
	do { if (!(x)) tsx_assert_fail(__FILE__, __LINE__, #x); } while (0)

#endif
