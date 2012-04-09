#ifndef _TXN_ASSERT_H
#define _TXN_ASSERT_H 1

#include <assert.h>

#define TXN_ABORT() \
	asm volatile \
           ("    jmp 1f\n"			\
            "2:\n"				\
            "    .section .txn_abort,\"ax\"\n" 	\
            "1:  .byte 0xc6,0xf8,0xfe\n" 	\
	    "    jmp 2b\n"			\
            "    .previous" ::: "memory")

#define TXN_ASSERT(x) \
	do if (!(x)) { TXN_ABORT(); assert(0); } while(0)

/* Only assert when in a transction. nop when not */
#define TSX_ASSERT_ONLY(x) \
	do if (!(x)) TXN_ABORT(); while(0)


#endif
