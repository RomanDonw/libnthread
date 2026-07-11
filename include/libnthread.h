/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef LIBNTHREAD_H
#define LIBNTHREAD_H

#ifdef __cplusplus
    extern "C"
    {
#endif

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
    #define LIBNTHREAD_OS_WINDOWS

    #ifdef LIBNTHREAD_STATIC
        #ifdef _MSC_VER
            #define LIBNTHREAD_API
        #else
            #define LIBNTHREAD_API __attribute__((visibility("default")))
        #endif
    #else
        #ifdef _MSC_VER
            #ifdef LIBNTHREAD_EXPORT
                #define LIBNTHREAD_API __declspec(dllexport)
            #else
                #define LIBNTHREAD_API __declspec(dllimport)
            #endif
        #else
            #define LIBNTHREAD_API __attribute__((visibility("default")))
        #endif
    #endif

    #include <windows.h>

    typedef CRITICAL_SECTION NTHREAD_MUTEXDESCRIPTOR;
#else
    #define LIBNTHREAD_API __attribute__((visibility("default")))

    #include <pthread.h>

    typedef pthread_mutex_t NTHREAD_MUTEXDESCRIPTOR;
#endif

#include <libncore.h>
#include <stdbool.h>
#include <stddef.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
    #define LIBNTHREAD_USEC11ATOMICS
    #include <stdatomic.h>
#elif defined(_MSC_VER)
    #define LIBNTHREAD_USEMSVCATOMICS

    #if defined(_M_ARM) || defined(_M_ARM64)
        #define LIBNTHREAD_USEDMSVCONARM
    #else
        #include <intrin.h>
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #define LIBNTHREAD_USEGCCORCLANGATOMICS
#else
    #error This compiler doesn't support atomic operations, or compiler atomic operations doesn't supported by this library.
#endif

#define LIBNTHREAD_ABI

/*
    #########################
             General
    #########################
*/

struct LibNThreadStartupOptions
{
    const NMemoryAllocators *allocators; // can be NULL.
    NPanicHandler *panichandler; // can be NULL.
    NAlertHandler *alerthandler; // can be NULL.
} typedef LibNThreadStartupOptions;

#define LIBNTHREADSTARTUPOPTIONS_DEFAULTINIT (LibNThreadStartupOptions){0}

extern const char *LIBNTHREAD_MODULENAME;

LIBNTHREAD_API bool LIBNTHREAD_ABI libnthread_initialized(void); // can be accessed without library initialization.
LIBNTHREAD_API NError LIBNTHREAD_ABI libnthread_startup(const LibNThreadStartupOptions *options);
LIBNTHREAD_API NError LIBNTHREAD_ABI libnthread_cleanup(void);

/*
    #########################
             Threads
    #########################
*/

typedef struct NThread NThread;

/*
    #########################
             Mutexes
    #########################
*/

typedef struct NThreadMutex NThreadMutex;

LIBNTHREAD_API NError LIBNTHREAD_ABI nthread_mutex_create(NThreadMutex **mutex);
LIBNTHREAD_API NError LIBNTHREAD_ABI nthread_mutex_destroy(NThreadMutex *mutex);

LIBNTHREAD_API NError LIBNTHREAD_ABI nthread_mutex_lock(NThreadMutex *mutex);
LIBNTHREAD_API NError LIBNTHREAD_ABI nthread_mutex_trylock(NThreadMutex *mutex);
LIBNTHREAD_API NError LIBNTHREAD_ABI nthread_mutex_unlock(NThreadMutex *mutex);

/*
    #########################
            Unsafe API
    #########################
*/

#if defined(LIBNTHREAD_ALLOWUNSAFEACCESS) || defined(LIBNTHREAD_EXPORT)
    LIBNTHREAD_API NTHREAD_MUTEXDESCRIPTOR LIBNTHREAD_ABI nthread_mutex_gethandle(const NThreadMutex *mutex);
#endif

/*
    #########################
           Atomic Bool
    #########################
*/

/*
    ..._cmpxhg function postfixes info:
        p - pointer (*var, *expected, desired).    if *var changed returns 'true', otherwise returns 'false' and stores *var to *expected.
        t - try (*var, expected, desired).         if *var changed returns 'true', otherwise returns 'false'.
        v - (old) value (*var, expected, desired). returns *var before performing operation.
*/

#if defined(LIBNTHREAD_USEC11ATOMICS)

    typedef atomic_bool NThreadAtomicBool;
    #define NTHREAD_ATOMICBOOLINIT(value) (ATOMIC_VAR_INIT((bool)(value)))

    static inline bool nthread_atomicbool_load(NThreadAtomicBool *variable) { return atomic_load(variable); }
    static inline void nthread_atomicbool_store(NThreadAtomicBool *variable, bool desired) { atomic_store(variable, desired); }

    static inline bool nthread_atomicbool_cmpxchgv(NThreadAtomicBool *variable, bool expected, bool desired)
    { bool exponstackcopy = expected; atomic_compare_exchange_strong(variable, &exponstackcopy, desired); return exponstackcopy; }

    static inline bool nthread_atomicbool_cmpxchgt(NThreadAtomicBool *variable, bool expected, bool desired)
    { bool exponstackcopy = expected; return atomic_compare_exchange_strong(variable, &exponstackcopy, desired); }

    static inline bool nthread_atomicbool_cmpxchgp(NThreadAtomicBool *variable, bool *expected, bool desired)
    { return atomic_compare_exchange_strong(variable, expected, desired); }

#elif defined(LIBNTHREAD_USEMSVCATOMICS)

    #if defined(_ARM64_BARRIER_ISH)
        #define LIBNTHREAD_ARMMEMORYBARRIER _ARM64_BARRIER_ISH
    #elif defined(_ARM_BARRIER_ISH)
        #define LIBNTHREAD_ARMMEMORYBARRIER _ARM_BARRIER_ISH
    #endif

    typedef volatile char NThreadAtomicBool;
    #define NTHREAD_ATOMICBOOLINIT(value) ((bool)(value))

    static inline bool nthread_atomicbool_load(NThreadAtomicBool *variable)
    {
        #ifdef LIBNTHREAD_USEDMSVCONARM
            bool ret = __iso_volatile_load8(variable);
            __dmb(LIBNTHREAD_ARMMEMORYBARRIER);
        #else
            bool ret = *(volatile bool *)variable;
            _ReadWriteBarrier();
        #endif
        return ret;
    }
    static inline void nthread_atomicbool_store(NThreadAtomicBool *variable, bool desired)
    {
        #ifdef LIBNTHREAD_USEDMSVCONARM
            __dmb(LIBNTHREAD_ARMMEMORYBARRIER);
            __iso_volatile_store8(variable, desired);
        #else
            _ReadWriteBarrier();
            *variable = desired;
        #endif
    }

    static inline bool nthread_atomicbool_cmpxchgv(NThreadAtomicBool *variable, bool expected, bool desired)
    { return _InterlockedCompareExchange8(variable, (unsigned char)desired, (unsigned char)expected); }

    static inline bool nthread_atomicbool_cmpxchgt(NThreadAtomicBool *variable, bool expected, bool desired)
    { return nthread_atomicbool_cmpxchgv(variable, expected, desired) == expected; }

    static inline bool nthread_atomicbool_cmpxchgp(NThreadAtomicBool *variable, bool *expected, bool desired)
    {
        register bool expval = *expected;
        register bool oldvarval = nthread_atomicbool_cmpxchgv(variable, expval, desired);
        if (oldvarval == expval) return true;
        *expected = oldvarval;
        return false;
    }

#elif defined(LIBNTHREAD_USEGCCORCLANGATOMICS)

    typedef bool NThreadAtomicBool;
    #define NTHREAD_ATOMICBOOLINIT(value) ((bool)(value))

    static inline bool nthread_atomicbool_load(NThreadAtomicBool *variable) { return __atomic_load_n(variable, __ATOMIC_SEQ_CST); }
    static inline void nthread_atomicbool_store(NThreadAtomicBool *variable, bool desired) { __atomic_store_n(variable, desired, __ATOMIC_SEQ_CST); }

    static inline bool nthread_atomicbool_cmpxchgv(NThreadAtomicBool *variable, bool expected, bool desired)
    {
        bool exponstackcopy = expected;
        __atomic_compare_exchange_n(variable, &exponstackcopy, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        return exponstackcopy;
    }

    static inline bool nthread_atomicbool_cmpxchgt(NThreadAtomicBool *variable, bool expected, bool desired)
    {
        bool exponstackcopy = expected;
        return __atomic_compare_exchange_n(variable, &exponstackcopy, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    }

    static inline bool nthread_atomicbool_cmpxchgp(NThreadAtomicBool *variable, bool *expected, bool desired)
    { return __atomic_compare_exchange_n(variable, expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); }

#endif

#ifdef __cplusplus
    }
#endif

#endif