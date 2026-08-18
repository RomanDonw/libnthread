/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "err.h"

#include <errno.h>
#include "util.h"

NError __libnthread_translateERRNOerror(int err)
{
    switch (err)
    {
        case 0:
            return NError_Success;

        case ENOMEM:
            return NError_MemoryAllocationFailed;

        case EBUSY:
            return NError_MutexBusy;
        
        case EDEADLOCK:
            return NError_MutexDeadlock;

        case EINVAL:
            return NError_IncorrectArgumentValue;

        case EPERM:
            return NError_OperationNotPermitted;

        default:
            alert("Got unhandled errno system error: %i.", err);
            return NError_InternalUnknownError;
    }
}

#ifdef LIBNTHREAD_OS_WINDOWS
    NError __libnthread_translateWINAPIerror(DWORD err)
    {
        switch (err)
        {
            case ERROR_INVALID_HANDLE:
                return NError_InvalidDescriptor;

            case ERROR_ACCESS_DENIED:
                return NError_AccessDenied;

            case ERROR_INVALID_PARAMETER:
                return NError_IncorrectArgumentValue;

            default:
                alert("Got unhandled errno system error: %i.", err);
                return NError_InternalUnknownError;
        }
    }
#endif