#include "txn-assert.h"
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int foo;

int main(void)
{
	pthread_mutex_lock(&lock);
	TXN_ASSERT_ONLY(foo);
	pthread_mutex_unlock(&lock);
	return 0;
}
