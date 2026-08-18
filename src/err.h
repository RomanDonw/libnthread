/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef ERR_H
#define ERR_H

#include "libnthread.h"

NError __libnthread_translateERRNOerror(int err);
#define translateERRNOerror(err) (__libnthread_translateERRNOerror(err))

#ifdef LIBNTHREAD_OS_WINDOWS
    NError __libnthread_translateWINAPIerror(DWORD err);
    #define translateWINAPIerror(err) (__libnthread_translateWINAPIerror(err))
#endif

#endif