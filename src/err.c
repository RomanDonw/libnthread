/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "err.h"

#include "util.h"

NError __libnthread_translateerror(int err)
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
            alert("Got unhandled system error: %i.", err);
            return NError_InternalUnknownError;
    }
}