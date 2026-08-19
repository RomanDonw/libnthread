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

struct NThreadHandlerData
{
    NThreadHandler handler;
    void *userdata;
} typedef NThreadHandlerData;

#ifdef LIBNTHREAD_OS_WINDOWS
    #include <errno.h>

    typedef DWORD threadroutinerettype;
    static threadroutinerettype __stdcall
#else
    typedef void *threadroutinerettype;
    static threadroutinerettype
#endif
threadroutine(NThreadHandlerData *handlerdata)
{
    register NThreadHandler handler = handlerdata->handler;
    register void *userdata = handlerdata->userdata;
    allocs.free(handlerdata);
    return (threadroutinerettype)((uintptr_t)handler(userdata));
}

NError nthread_create(NThread **thread, NThreadHandler handler, void *userdata)
{
    ENSURE_INIT;

    NThread *ret = allocs.malloc(sizeof(NThread));
    if (!ret) return NError_MemoryAllocationFailed;

    NThreadHandlerData *data = allocs.malloc(sizeof(NThreadHandlerData));
    if (!data) { allocs.free(ret); return NError_MemoryAllocationFailed; }
    data->handler = handler;
    data->userdata = userdata;

    ret->joining = NTHREAD_ATOMICBOOLINIT(false);
    #ifdef LIBNTHREAD_OS_WINDOWS
        if (!(ret->desc = _beginthreadex(NULL, 0, (void *)(&threadroutine), data, 0, NULL)))
        { free(data); free(ret); return translateERRNOerror(errno); }
    #else
        int err = pthread_create(&ret->desc, NULL, (void *)threadroutine, data);
        if (err) { free(data); free(ret); return translateERRNOerror(err); }
    #endif

    SAFE_MUTEX_LOCK(threadlistmutex);
    NError nerr = n_unorderedset_addelement(threadlist, &ret);
    if (nerr != NError_Success) panic_general(nerr, "Unable to add thread to threads list after factual creation.");
    SAFE_MUTEX_UNLOCK(threadlistmutex);

    *thread = ret;
    return NError_Success;
}

NError nthread_join(NThread *thread, NThreadReturnType *exitcode)
{
    ENSURE_INIT;

    NError nerr = nthread_mutex_lock(threadlistmutex);
    if (nerr != NError_Success) return nerr;
    
    bool has = n_unorderedset_haselement(threadlist, &thread);
    if (!has) { nerr = NError_Fault; goto errorquit_onlyafterlistmtxlock; }

    if (nthread_atomicbool_cmpxchgv(&thread->joining, false, true)) { nerr = NError_Already; goto errorquit_onlyafterlistmtxlock; }
    SAFE_MUTEX_UNLOCK(threadlistmutex);

    threadroutinerettype res;
    #ifdef LIBNTHREAD_OS_WINDOWS
        WaitForSingleObject(thread->desc, INFINITE);
        if (exitcode && !GetExitCodeThread(thread->desc, &res)) panic_general(translateWINAPIerror(GetLastError()), "Unable to get thread exit code after factual stopping thread.");
        CloseHandle(thread->desc);
    #else
        int err = pthread_join(thread->desc, &res);
        if (err) return translateERRNOerror(err);
    #endif
    free(thread);

    SAFE_MUTEX_LOCK(threadlistmutex);
    if ((nerr = n_unorderedset_removeelement(threadlist, &thread)) != NError_Success) panic_general(nerr, "Unable to remove thread from threads list after factual destroying.");
    SAFE_MUTEX_UNLOCK(threadlistmutex);

    if (exitcode) *exitcode = (NThreadReturnType)((uintptr_t)res);
    return NError_Success;

    errorquit_onlyafterlistmtxlock:
        SAFE_MUTEX_UNLOCK(threadlistmutex);
    return nerr;
}

/*
void nthread_exit(NThreadReturnType exitcode)
{
    #ifdef LIBNTHREAD_OS_WINDOWS
        _endthreadex(exitcode);
    #else
        pthread_exit()
    #endif
}
*/