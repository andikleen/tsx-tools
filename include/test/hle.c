/* HLE emulated intrinsics tester. This does not test actual transactions. */
#include "hle-emulation.h"
#include <assert.h>

int lock;

#define TEST_OP_FETCH(type, sz)						\
	{								\
		static type lock;					\
		type res;						\
		res = __hle_acquire_add_fetch##sz(&lock, 1);		\
		assert(res == 1);				\
		res = __hle_release_sub_fetch##sz(&lock, 1);	\
		assert(res == 0);			\
		assert(lock == 0);			\
	}

#define TEST_FETCH_OP(type, sz)						\
	{								\
		static type lock;					\
		type res;						\
		res = __hle_acquire_fetch_add##sz(&lock, 1);		\
		assert(res == 0);					\
		res = __hle_release_fetch_sub##sz(&lock, 1);		\
		assert(res == 1);			\
		assert(lock == 0);			\
	}

#define TEST_XCHG(type, sz)						\
	{								\
		static type lock;					\
		type res;						\
		res = __hle_acquire_exchange_n##sz(&lock, 1);		\
		assert(res == 0);				\
		__hle_release_clear##sz(&lock);			\
		assert(lock == 0);			\
	}

#define TEST_TAS(type, sz)						\
	{								\
		static type lock;					\
		type res;						\
		res = __hle_acquire_test_and_set##sz(&lock);		\
		assert(res == 0);				\
		__hle_release_clear##sz(&lock);			\
		assert(lock == 0);			\
	}


#define TEST_CMP_XCHG(type, sz)						\
	{								\
		static type lock;					\
		type res;						\
		type oldv = 0;						\
		res = __hle_acquire_compare_exchange_n##sz(&lock, &oldv, 1);\
		assert(res == 1);				\
		assert(lock == 1);				\
		oldv = 1;					\
		res = __hle_release_compare_exchange_n##sz(&lock, &oldv, 0);\
		assert(res == 1);			\
		assert(lock == 0);			\
	}

#define TEST_NONFETCH(type, sz) 	\
	{							\
		static type lock;				\
		__hle_acquire_add##sz(&lock, 1);		\
		assert(lock == 1);				\
		__hle_release_sub##sz(&lock, 1);		\
		assert(lock == 0);				\
		__hle_acquire_or##sz(&lock, 1);			\
		assert(lock == 1);				\
		__hle_release_and##sz(&lock, 0);		\
		assert(lock == 0);				\
		__hle_acquire_xor##sz(&lock, 1);		\
		assert(lock == 1);				\
		__hle_release_xor##sz(&lock, 1);			\
		assert(lock == 0);				\
	}


int main(void)
{
#if __SIZEOF_POINTER__==8
	TEST_OP_FETCH(unsigned long long, 8);
	TEST_FETCH_OP(unsigned long long, 8);
	TEST_XCHG(unsigned long long, 8);
	TEST_TAS(unsigned long long, 8);
	TEST_CMP_XCHG(unsigned long long, 8);
	TEST_NONFETCH(unsigned long long, 8);
#endif
	TEST_OP_FETCH(unsigned, 4);
	TEST_OP_FETCH(unsigned short, 2);
	TEST_OP_FETCH(unsigned char, 1);

	TEST_FETCH_OP(unsigned, 4);
	TEST_FETCH_OP(unsigned short, 2);
	TEST_FETCH_OP(unsigned char, 1);

	TEST_XCHG(unsigned, 4);
	TEST_XCHG(unsigned short, 2);
	TEST_XCHG(unsigned char, 1);

	TEST_TAS(unsigned, 4);
	TEST_TAS(unsigned short, 2);
	TEST_TAS(unsigned char, 1);

	TEST_CMP_XCHG(unsigned, 4);
	TEST_CMP_XCHG(unsigned short, 2);
	TEST_CMP_XCHG(unsigned char, 1);

	TEST_NONFETCH(unsigned, 4);
	TEST_NONFETCH(unsigned short, 2);
	TEST_NONFETCH(unsigned char, 1);

	return 0;
}
