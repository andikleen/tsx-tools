#ifndef _HLE_H
#define _HLE_H 1

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
  Emulation for gcc HLE intrinsics on older compilers.

  gcc 4.8+ implements HLE as an additional memory ordering model for the C11+
  atomic intrinsics.  gcc has its own flavour which are similar to C11,
  but use a different naming convention.

  We cannot directly emulate the full memory model.

  So the operations are mapped to __hle_acquire_<name> and __hle_release_
  without an explicit memory model parameter.

  The other problem is that C11 atomics use argument overloading
  to support different types. While that would be possible to emulate
  it would generate very ugly macros. We instead add the type size
  as a postfix.

  So for example:

  int foo;
  __atomic_or_fetch(&foo, 1, __ATOMIC_ACQUIRE|__ATOMIC_HLE_ACQUIRE)

  become

  __hle_acquire_or_fetch4(&foo, 1);

  Also C11 has some operations that do not map directly to x86 
  atomic instructions. Since HLE requires that a single instruction,
  we omit those. That includes nand, xor, and, or.  While they could
  be mapped to CMPXCHG this would require a spin loop, which is 
  better not done implicitely. There is also no HLE load.

  x86 supports HLE prefixes for all atomic operations, but not all
  can currently be generated in this scheme, as many operations
  have no support for fetch.

  A real compiler could generate them by detecting that the fetch
  value is not used, but we don't have this luxury. For this we have
  non _fetch variants. These also support and, or, xor (but not nand),
  as a extension.

  Intrinsics for sbb, adc, neg, btr, bts, btc are not supported.

  We also don't implement the non _n generic version of some operations.

  Available operations:
  (8 only valid on 64bit)

  __hle_{acquire,release}_add_fetch{1,2,4,8}
  __hle_{acquire,release}_sub_fetch{1,2,4,8}
  __hle_{acquire,release}_fetch_add{1,2,4,8}
  __hle_{acquire,release}_fetch_sub{1,2,4,8}
  __hle_{acquire,release}_{add,sub,or,xor,and}{1,2,4,8}	(extension)
  __hle_{acquire,release}_store_n{1,2,4,8}
  __hle_{acquire,release}_clear{1,2,4,8}
  __hle_{acquire,release}_exchange_n{1,2,4,8}
  __hle_{acquire,release}_compare_exchange_n{1,2,4,8}
  __hle_{acquire,release}_test_and_set{1,2,4,8}		(sets to 1)

  gcc documentation:

  http://gcc.gnu.org/onlinedocs/gcc-4.8.0/gcc/_005f_005fatomic-Builtins.html#_005f_005fatomic-Builtins

*/

#define __hle_force_inline __attribute__((always_inline)) inline

#define __HLE_ACQUIRE ".byte 0xf2 ; " 
#define __HLE_RELEASE ".byte 0xf3 ; " 

/* Since there are so many combinations we have to use macros heavily. */

#define __HLE_ADD_FETCH(type, prefix, asm_prefix, size)			\
	static __hle_force_inline type					\
	__hle_##prefix##_add_fetch##size(type *ptr, type val)		\
	{								\
		type oldval = val;					\
		asm volatile(asm_prefix " ; lock ; xadd %0,%1"		\
			     : "+q" (val), "+m" (*ptr) :: "memory");	\
		return val + oldval;					\
	} 								\
	static __hle_force_inline type					\
	__hle_##prefix##_sub_fetch##size(type *ptr, type val)		\
	{								\
		type oldval = val;					\
		val = -val;						\
		asm volatile(asm_prefix " ; lock ; xadd %0,%1"		\
			     : "+q" (val), "+m" (*ptr) :: "memory");	\
		return val - oldval;					\
	} 


#define __HLE_FETCH_ADD(type, prefix, asm_prefix, size)			\
	static __hle_force_inline type					\
	__hle_##prefix##_fetch_add##size(type *ptr, type val)		\
	{								\
		asm volatile(asm_prefix " ; lock ; xadd %0,%1"		\
			     : "+q" (val), "+m" (*ptr) :: "memory");	\
		return val;						\
	} 								\
	static __hle_force_inline type					\
	__hle_##prefix##_fetch_sub##size(type *ptr, type val)		\
	{								\
		val = -val;						\
		asm volatile(asm_prefix " ; lock ; xadd %0,%1"		\
			     : "+q" (val), "+m" (*ptr) :: "memory");	\
		return val;						\
	} 

#define __HLE_STORE(type, prefix, asm_prefix, size)			\
	static __hle_force_inline void					\
	__hle_##prefix##_store_n##size(type *ptr, unsigned val)		\
	{								\
		asm volatile(asm_prefix "mov %1,%0" : 			\
				"=m" (*ptr) : "q" (val)			\
				: "memory");				\
	}								\
	static __hle_force_inline void					\
	__hle_##prefix##_clear##size(type *ptr)				\
	{								\
		__hle_##prefix##_store_n##size(ptr, 0);			\
	}

#define __HLE_EXCHANGE(type, prefix, asm_prefix, size) 			\
	static __hle_force_inline type					\
	__hle_##prefix##_exchange_n##size(type *ptr, type val)		\
	{								\
		asm volatile(asm_prefix " ; lock ; xchg %0,%1"		\
			     : "+q" (val), "+m" (*ptr) :: "memory");	\
		return val;						\
	} 								\
	static __hle_force_inline int					\
	__hle_##prefix##_test_and_set##size(type *ptr)			\
	{								\
		return __hle_##prefix##_exchange_n##size(ptr, 1) == 1;	\
	}								\
	static __hle_force_inline int					\
	__hle_##prefix##_compare_exchange_n##size(type *ptr, type *oldp, \
			type newv)					\
	{								\
		unsigned char res;					\
		asm volatile(asm_prefix " ; lock ; cmpxchg %3,%1"	\
			     " ; setz %2"				\
			     : "+a" (*oldp), "+m" (*ptr), "=r" (res) 	\
			     : "r" (newv) 				\
			     : "memory");				\
		return res;						\
	} 

#define __HLE_NONFETCH_OP(type, prefix, asm_prefix, size, op)	\
	static __hle_force_inline void				\
	__hle_##prefix##_##op##size(type *ptr, type val)		\
	{								\
		asm volatile(asm_prefix " ; lock ; " #op " %1,%0"	\
			     : "+m" (*ptr) : "q" (val) : "memory");	\
	}

#define __HLE_OP(type, size) \
__HLE_ADD_FETCH(type, acquire, __HLE_ACQUIRE, size)	\
__HLE_ADD_FETCH(type, release, __HLE_RELEASE, size)	\
__HLE_FETCH_ADD(type, acquire, __HLE_ACQUIRE, size)	\
__HLE_FETCH_ADD(type, release, __HLE_RELEASE, size)	\
__HLE_EXCHANGE(type, acquire, __HLE_ACQUIRE, size)	\
__HLE_EXCHANGE(type, release, __HLE_RELEASE, size)	\
__HLE_STORE(type, acquire, __HLE_ACQUIRE, size)		\
__HLE_STORE(type, release, __HLE_RELEASE, size)		\
__HLE_NONFETCH_OP(type, acquire, __HLE_ACQUIRE, size, add)	\
__HLE_NONFETCH_OP(type, acquire, __HLE_ACQUIRE, size, sub)	\
__HLE_NONFETCH_OP(type, acquire, __HLE_ACQUIRE, size, or)	\
__HLE_NONFETCH_OP(type, acquire, __HLE_ACQUIRE, size, and)	\
__HLE_NONFETCH_OP(type, acquire, __HLE_ACQUIRE, size, xor)	\
__HLE_NONFETCH_OP(type, release, __HLE_RELEASE, size, add)	\
__HLE_NONFETCH_OP(type, release, __HLE_RELEASE, size, sub)	\
__HLE_NONFETCH_OP(type, release, __HLE_RELEASE, size, or)	\
__HLE_NONFETCH_OP(type, release, __HLE_RELEASE, size, and)	\
__HLE_NONFETCH_OP(type, release, __HLE_RELEASE, size, xor)

#if __SIZEOF_POINTER__ == 8
__HLE_OP(unsigned long long, 8)
#endif
__HLE_OP(unsigned, 	 4)
__HLE_OP(unsigned short, 2)
__HLE_OP(unsigned char,  1)

#endif
