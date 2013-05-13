/* Test program for MS flavoured HLE intrinsics */
#include "hle-ms.h"
#include <assert.h>

int main(void)
{
	{
		static int lock;
		int res;

		res = _InterlockedCompareExchange_HLEAcquire(&lock, 1, 0);
		assert(res == 0);
		assert(lock == 1);
		res = _InterlockedCompareExchange_HLERelease(&lock, 0, 1);
		assert(lock == 0);
		assert(res == 1);

		res = _InterlockedExchangeAdd_HLEAcquire(&lock, 1);
		assert(res == 0);
		assert(lock == 1);
		_Store_HLERelease(&lock, 0);
		assert(lock == 0);
	}

	{
		static __hle_int64 lock;
		__hle_int64 res;

		res = _InterlockedCompareExchange64_HLEAcquire(&lock, 1, 0);
		assert(res == 0);
		assert(lock == 1);
		res = _InterlockedCompareExchange64_HLERelease(&lock, 0, 1);
		assert(lock == 0);
		assert(res == 1);

		res = _InterlockedExchangeAdd64_HLEAcquire(&lock, 1);
		assert(res == 0);
		assert(lock == 1);
		_Store64_HLERelease(&lock, 0);
		assert(lock == 0);

	}

	{
		static void *lock;
		void *res;

		res = _InterlockedCompareExchangePointer_HLEAcquire(&lock,
								    (void*)1L,
								    0);
		assert(res == 0);
		assert(lock == (void *)1L);
		res = _InterlockedCompareExchangePointer_HLERelease(&lock,
								    0,
								    (void *)1L);
		assert(lock == 0);
		assert(res == (void *)1L);

		res = _InterlockedCompareExchangePointer_HLEAcquire(&lock, 
								    (void *)1L,
								    0);
		assert(res == 0);
		assert(lock == (void *)1L);
		_StorePointer_HLERelease(&lock, 0);
		assert(lock == 0);
	}

	return 0;
}
