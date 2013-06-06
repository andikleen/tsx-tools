![tsx-tools](http://halobates.de/tsx-tools.png)

# Useful TSX related tools

TSX (Intel Transactional Synchronization Extension) is a hardware transactional memory extension in recent Intel CPU codenamed Haswell. For more information see http://en.wikipedia.org/wiki/Transactional_Synchronization_Extensions

This package provides some tools for TSX development.

## has-tsx

Query the current

        % make
        % ./has-tsx
        RTM: Yes
        HLE: Yes

## Emulated headers for HLE and RTM intrinsics

Headers to emulate the gcc 4.8+ and the Microsoft HLE/RTM intrinsics on
older gcc compatible providers. Plus a special header to expose the control
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

        #include "hle-emulation.h"
        #include "immintrin.h"  /* for _mm_pause() */

        static volatile int lock;

        /* Take elided lock */
        while (__hle_acquire_test_and_set(&lock) == 1) {
                while (lock == 0)
                        _mm_pause();
        }
        ...
        /* Release elided lock */
        __hle_release_clear(&lock);

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

## tsx-assert 

A facility to do asserts in transactions.
Requires a special hook into the elided lock library

