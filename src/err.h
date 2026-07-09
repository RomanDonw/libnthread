/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef ERR_H
#define ERR_H

#include "libnthread.h"

#ifndef LIBNTHREAD_OS_WINDOWS
    #include <errno.h>

    NError __libnthread_translateerror(int err);
    #define translateerror(err) (__libnthread_translateerror(err))
#endif

#endif