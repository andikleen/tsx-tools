#include "hle.h"

#define PAIR(x) x, sizeof(x) - 1

typedef void (*tsx_assert_hook_t)(unsigned);
extern tsx_assert_hook_t __tsx_set_abort_hook(tsx_assert_hook_t);

extern char *txn_assert_table[];
extern int txn_assert_table_size;

static void assert_hook(unsigned status)
{
	if ((status & XABORT_EXPLICIT_ABORT) && 
		XABORT_STATUS(status) < txn_assert_table_size) {
        	write(2, PAIR("txn assert failure at "));
		write(2, txn_assert_table[XABORT_STATUS(status)]);
	} 
}

static void __attribute__((constructor)) init_txn_assert(void)
{
	__tsx_set_abort_hook(assert_hook);
}

