#include "txn-assert.h"

int foo;

f2()
{
	TXN_ASSERT_ONLY(foo);
}

main()
{
	TXN_ASSERT_ONLY(!foo);
}
