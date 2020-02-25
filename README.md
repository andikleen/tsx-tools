![tsx-tools](http://halobates.de/tsx-tools.png)

# Useful TSX related tools

TSX (Intel Transactional Synchronization Extension) is a hardware transactional memory extension in recent 4th generation Core Intel CPUs codenamed Haswell. For more information see http://en.wikipedia.org/wiki/Transactional_Synchronization_Extensions and http://www.intel.com/software/tsx

This package provides some tools and libraries for TSX development.

## Compilation notes

You may need to update your compiler's copy of `cpuid.h` depending on the version. In general, it should be sufficient to simply copy the file from a newer source distribution.

For example, on Mac OS X with LLVM version 4.2 (check with `cc --version`), you should replace `/usr/lib/clang/4.2/include/cpuid.h` with the one available at https://llvm.org/viewvc/llvm-project/cfe/trunk/lib/Headers/cpuid.h?view=co

## has-tsx

Check if the current CPU supports TSX.

        % make
        % ./has-tsx
        RTM: Yes
        HLE: Yes

## Emulated headers for HLE and RTM intrinsics

Headers to emulate the gcc 4.8+ and the Microsoft HLE/RTM intrinsics on
older gcc compatible compilers. Plus a special header to expose the control
flow of abort handlers directly using "asm goto".

### rtm.h
        
        #include "rtm.h"        /* For gcc 4.8 use immintrin.h and -mrtm */
        ..
        if (_xbegin() == _XBEGIN_STARTED) {
                /* read lock */
                /* transaction */
                _xend();
        } else 
                /* fallback to read lock */

### hle-emulation.h

Similar to the HLE extensions to the atomic intrinsics in gcc 4.8+,
but implemented for older compilers. See
http://software.intel.com/en-us/blogs/2013/05/20/using-hle-and-rtm-with-older-compilers-with-tsx-tools
for more details

        #include "hle-emulation.h"
        #include "immintrin.h"  /* for _mm_pause() */

        static volatile int lock;

        /* Take elided lock */
        while (__hle_acquire_test_and_set(&lock) == 1) {
                while (lock != 0)
                        _mm_pause();
        }
        ...
        /* Release elided lock */
        __hle_release_clear(&lock);

### hle-ms.h

Provide Microsoft C compatible HLE intrinsics for gcc.

### rtm-goto.h

An experimential RTM intrinsics implementation that directly exposes the control
flow of the abort handler. This allows to save a few instructions and may be
clearer. Requires a compiler with "asm goto" support (gcc 4.7+ or some earlier
RedHat gcc versions)

	#include "rtm-goto.h"

	XBEGIN(abort_handler);
	/* Transaction */
	XEND();	
	return;

	/* Transaction aborts come here */
	unsigned status;
	XFAIL_STATUS(status);
	/* Examine status to determine abort cause */
	/* Fallback path */

## tsx-cpuid.h

Utility functions to check if the current CPU supports HLE or RTM from a program.

	#include "tsx-cpuid.h"

	init:
		if (cpu_has_rtm())
			have_rtm = 1;

	lock code:
		if (have_rtm && _xbegin() == _XBEGIN_STARTED) {
			/* RTM code */
			_xend();
		} else { 
			/* fallback */
		}

## ignore-xend.so

When running with the RTM enabled glibc unlocking and unlocked lock causes 
a general protection fault. Normal glibc ignores those.
This LD_PRELOAD catches these faults and allows the buggy programs to run.
May also be useful with older RTM based lock libraries.

LD_PRELOAD=/path/to/ignore-xend.so program 

## remove-hle.py

Remove HLE prefixes from a binary. Useful for verifying that a problem
happens without HLE too. First run without -p and verify the patch sites,
then use with -p binary to patch.

Warning: this can destroy a binary since there can be false positives.
Always run on a backup copy.

## tsx-assert.h

Normal assert does not work in TSX transaction. The assert output
is an IO operation, which causes an abort so the assert gets
discarded (unless it happens again when re-executed non transactionally)

This can be a curse or a blessing, but there are some situations
where it is inconvenient.
 
tsx-assert.h provides a TSX aware assert. It only works with RTM, not with HLE.
It commits the transaction before executing the assert.

	#include "tsx-assert.h"

	/* in transaction */
	tsx_assert(condition);

Link the program with the tsx-assert.o object file

	gcc ... tsx-assert.o

Based on a idea from Torvald Riegel.

## glibc-tune

glibc tune allows to change the lock elision parameters in the currently running glibc.

This can be useful to experiment with elision enabled/disabled, force enable
elision on glibc builds that were not built with --enable-lock-elision=yes, 
or tune retry parameters to improve performance on larger systems.

## locks

tsxlocks is a simple library of reference TSX lock elision locks.
