/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "init.h"

#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "util.h"

#ifndef LIBNTHREAD_OS_WINDOWS
    #if defined(__USE_UNIX98) || defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500
        #define PTHREAD_MUTEXTYPE_RECURSIVE PTHREAD_MUTEX_RECURSIVE
    #else
        #define PTHREAD_MUTEXTYPE_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
    #endif
#endif

static NThreadAtomicBool inited = NTHREAD_ATOMICBOOLINIT(false);
static NThreadAtomicBool funcslock = NTHREAD_ATOMICBOOLINIT(false);

#define ISLIBRARYINITED() (nthread_atomicbool_load(&inited))

bool libnthread_initialized(void) { return ISLIBRARYINITED(); }

NError libnthread_startup(const LibNThreadStartupOptions *options)
{
    if (nthread_atomicbool_cmpxchgv(&funcslock, false, true)) return NError_OperationInProgress;
    if (ISLIBRARYINITED()) { nthread_atomicbool_store(&funcslock, false); return NError_AlreadyInitialized; }

    NError nerr;

    static const LibNThreadStartupOptions defaultopts = LIBNTHREADSTARTUPOPTIONS_DEFAULTINIT;
    if (!options) options = &defaultopts;

    if (options->allocators) allocs = *options->allocators;
    else
    {
        allocs.malloc = malloc;
        allocs.realloc = realloc;
        allocs.free = free;
    }

    if (options->panichandler) __panichandler = *options->panichandler;
    else __panichandler = __defaultpanichandler;

    if (options->alerthandler) __alerthandler = *options->alerthandler;
    else __alerthandler = __defaultalerthandler;

    if ((nerr = n_unorderedset_create(&mutexlist, allocs, sizeof(NThreadMutex *))) != NError_Success) goto errorquit_generic;

    // =============================================================================

    #ifndef LIBNTHREAD_OS_WINDOWS
        if ((nerr = translateerror(pthread_mutexattr_init(&recursivemutexattr)) != NError_Success)) goto errorquit_aftercreatemtxlist;
        if ((nerr = translateerror(pthread_mutexattr_settype(&recursivemutexattr, PTHREAD_MUTEXTYPE_RECURSIVE))) != NError_Success) goto errorquit_afterinitmtxattr;
    #endif

    if ((nerr = __createmutex(&mutexlistmutex)) != NError_Success) goto errorquit_oncreatemutexformtxlist;

    // =============================================================================

    nthread_atomicbool_store(&inited, true);
    nthread_atomicbool_store(&funcslock, false);
    return NError_Success;

    // =============================================================================

    {
        NError tmpnerr;

        errorquit_oncreatemutexformtxlist:
            if ((tmpnerr = __destroymutex(mutexlistmutex)) != NError_Success) panic_general(tmpnerr, "Unable to destroy mutex of mutexes list when handling error.");
            mutexlistmutex = NULL;
        #ifndef LIBNTHREAD_OS_WINDOWS
            errorquit_afterinitmtxattr:
                if ((tmpnerr = translateerror(pthread_mutexattr_destroy(&recursivemutexattr))) != NError_Success)
                { panic_general(tmpnerr, "Unable to destroy pthread mutex attributes structure when handling error."); }
                memset(&recursivemutexattr, 0, sizeof(pthread_mutexattr_t));
            errorquit_aftercreatemtxlist:
        #endif
            n_unorderedset_destroy(mutexlist);
            mutexlist = NULL;
        errorquit_generic:
            __alerthandler = NULL;
            __panichandler = NULL;
            memset(&allocs, 0, sizeof(allocs));
    }

    nthread_atomicbool_store(&funcslock, false);
    return nerr;
    
}

NError libnthread_cleanup(void)
{
    if (nthread_atomicbool_cmpxchgv(&funcslock, false, true)) return NError_OperationInProgress;
    if (!ISLIBRARYINITED()) { nthread_atomicbool_store(&funcslock, false); return NError_NotInitialized; }

    // =============================================================================

    NError nerr;

    #ifndef LIBNTHREAD_OS_WINDOWS
        if ((nerr = translateerror(pthread_mutexattr_destroy(&recursivemutexattr))) != NError_Success)
        { nthread_atomicbool_store(&funcslock, false); return nerr; }
        memset(&recursivemutexattr, 0, sizeof(pthread_mutexattr_t));
    #endif

    // =============================================================================

    NThreadMutex *mutex;
    for (size_t i = 0; i < n_unorderedset_getlength(mutexlist); i++)
    {
        if (((nerr = n_unorderedset_getelement(mutexlist, i, &mutex)) != NError_Success) || ((nerr = __destroymutex(mutex)) != NError_Success))
        { panic_general(nerr, n_panicmsg_mutexdestroyduringlibrarycleanup); }
    }

    if ((nerr = __destroymutex(mutexlistmutex)) != NError_Success) panic_general(nerr, n_panicmsg_mutexdestroyduringlibrarycleanup);
    mutexlistmutex = NULL;

    n_unorderedset_destroy(mutexlist);
    mutexlist = NULL;

    __alerthandler = NULL;
    __panichandler = NULL;
    memset(&allocs, 0, sizeof(allocs));

    nthread_atomicbool_store(&inited, false);
    nthread_atomicbool_store(&funcslock, false);
    return NError_Success;
}