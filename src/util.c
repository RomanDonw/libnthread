/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "err.h"
#include "types.h"

const char *LIBNTHREAD_MODULENAME = "libnthread";

NMemoryAllocators __libnthread_allocators = {0};
NUnorderedSet *__libnthread_mutexlist = NULL;
NThreadMutex *__libnthread_mutexlistmutex = NULL;
NPanicHandler *__libnthread_panichandler = NULL;
NAlertHandler *__libnthread_alerthandler = NULL;

#ifndef LIBNTHREAD_OS_WINDOWS
    pthread_mutexattr_t __libnthread_recursivemutexattr = {0};
#endif

NError __libnthread_createmutex(NThreadMutex **mutex)
{
    NThreadMutex *ret = allocs.malloc(sizeof(NThreadMutex));
    if (!ret) return NError_MemoryAllocationFailed;

    #ifdef LIBNTHREAD_OS_WINDOWS
        InitializeCriticalSection(&ret->desc);
    #else
        int err = pthread_mutex_init(&ret->desc, &recursivemutexattr);
        if (err) { allocs.free(ret); return translateerror(err); }
    #endif

    *mutex = ret;
    return NError_Success;
}

NError __libnthread_destroymutex(NThreadMutex *mutex)
{
    #ifdef LIBNTHREAD_OS_WINDOWS
        DeleteCriticalSection(&mutex->desc);
    #else
        int err = pthread_mutex_destroy(&mutex->desc);
        if (err) return translateerror(err);
    #endif

    allocs.free(mutex);
    return NError_Success;
}

// =============================================================================

void __libnthread_defaultpanichandler(const char *module, const char *file, long long line, const char *function, const char *description, NError error)
{
    fprintf(stderr, "\n\n\n###################\n# LIBNTHREAD PANIC #\n###################\n\n");

    fprintf(stderr, "In \"%s\" at line %lld (%s):\n", file, line, function);

    if (error != PANIC_NOERRORCODE) fprintf(stderr, "    \"%s\" because\n    ", n_strerror(error));

    fprintf(stderr, "    %s\n\n###################\n\n", description);
}

void __libnthread_defaultalerthandler(const char *module, const char *file, long long line, const char *function, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "\n\n\n###################\n# LIBNTHREAD ALERT #\n###################\n\n");

    fprintf(stderr, "In \"%s\" at line %lld (%s):\n    ", file, line, function);
    vfprintf(stderr, format, args);

    fputs("\n\n###################\n", stderr);
    va_end(args);
}