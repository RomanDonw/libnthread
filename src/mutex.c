/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libnthread.h"

#include "init.h"
#include "util.h"
#include "err.h"
#include "types.h"

NError nthread_mutex_create(NThreadMutex **mutex)
{
    ENSURE_INIT;

    NThreadMutex *ret;
    NError nerr = __createmutex(&ret);
    if (nerr != NError_Success) return nerr;

    SAFE_MUTEX_LOCK(mutexlistmutex);
    if ((nerr = n_unorderedset_addelement(mutexlist, &ret)) != NError_Success) panic_general(nerr, "Unable to add mutex to mutexes list after factual creation.");
    SAFE_MUTEX_UNLOCK(mutexlistmutex);
    
    *mutex = ret;
    return NError_Success;
}

NError nthread_mutex_destroy(NThreadMutex *mutex)
{
    ENSURE_INIT;

    NError nerr = __destroymutex(mutex);
    if (nerr != NError_Success) return nerr;

    SAFE_MUTEX_LOCK(mutexlistmutex);
    if ((nerr = n_unorderedset_removeelement(mutexlist, &mutex)) != NError_Success) panic_general(nerr, "Unable to remove mutex from mutexes list after factual destroying.");
    SAFE_MUTEX_UNLOCK(mutexlistmutex);

    return NError_Success;
}

NError nthread_mutex_lock(NThreadMutex *mutex)
{
    ENSURE_INIT;

    #ifdef LIBNTHREAD_OS_WINDOWS
        EnterCriticalSection(&mutex->desc);
        return NError_Success;
    #else
        return translateerror(pthread_mutex_lock(&mutex->desc));
    #endif
}

NError nthread_mutex_trylock(NThreadMutex *mutex)
{
    ENSURE_INIT;

    #ifdef LIBNTHREAD_OS_WINDOWS
        return TryEnterCriticalSection(&mutex->desc) ? NError_Success : NError_MutexBusy;
    #else
        return translateerror(pthread_mutex_trylock(&mutex->desc));
    #endif
}

NError nthread_mutex_unlock(NThreadMutex *mutex)
{
    #ifdef LIBNTHREAD_OS_WINDOWS
        LeaveCriticalSection(&mutex->desc);
        return NError_Success;
    #else
        return translateerror(pthread_mutex_unlock(&mutex->desc));
    #endif
}