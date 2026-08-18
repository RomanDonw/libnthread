/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef UTIL_H
#define UTIL_H

#include "libnthread.h"

#include <stdlib.h>

extern NMemoryAllocators __libnthread_allocators;
#define allocs __libnthread_allocators

extern NUnorderedSet *__libnthread_threadlist;
#define threadlist (__libnthread_threadlist)
extern NThreadMutex *__libnthread_threadlistmutex;
#define threadlistmutex (__libnthread_threadlistmutex)

extern NUnorderedSet *__libnthread_mutexlist;
#define mutexlist (__libnthread_mutexlist)
extern NThreadMutex *__libnthread_mutexlistmutex;
#define mutexlistmutex (__libnthread_mutexlistmutex)

// =============================================================================

#ifndef LIBNTHREAD_OS_WINDOWS
    extern pthread_mutexattr_t __libnthread_recursivemutexattr;
    #define recursivemutexattr (__libnthread_recursivemutexattr)
#endif

NError __libnthread_createmutex(NThreadMutex **mutex);
#define __createmutex (__libnthread_createmutex)

NError __libnthread_destroymutex(NThreadMutex *mutex);
#define __destroymutex (__libnthread_destroymutex)

#define SAFE_MUTEX_LOCK(mutex) \
    { NError nerr = nthread_mutex_lock(mutex); if (nerr != NError_Success) panic_general(nerr, n_panicmsg_mutexlock); }

#define SAFE_MUTEX_UNLOCK(mutex) \
    { NError nerr = nthread_mutex_unlock(mutex); if (nerr != NError_Success) panic_general(nerr, n_panicmsg_mutexunlock); }

// =============================================================================

extern NPanicHandler *__libnthread_panichandler;
#define __panichandler (__libnthread_panichandler)

NPanicHandler __libnthread_defaultpanichandler;
#define __defaultpanichandler (__libnthread_defaultpanichandler)

#define PANIC_NOERRORCODE (NError_Success)

#define panic_general(errorcode, descripton) \
    {\
        __panichandler(LIBNTHREAD_MODULENAME, __FILE__, __LINE__, __func__, (descripton), (errorcode));\
        abort();\
    }

// =============================================================================

extern NAlertHandler *__libnthread_alerthandler;
#define __alerthandler (__libnthread_alerthandler)

NAlertHandler __libnthread_defaultalerthandler;
#define __defaultalerthandler (__libnthread_defaultalerthandler)

#ifdef LIBNTHREAD_DEBUG
    #define alert(format, ...) (__alerthandler(LIBNTHREAD_MODULENAME, __FILE__, __LINE__, __func__, format, __VA_ARGS__))
#else
    #define alert(format, ...)
#endif

// =============================================================================

#endif