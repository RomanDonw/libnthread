/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

const char *LIBNTHREAD_MODULENAME = "libnthread";

NMemoryAllocators __libnthread_allocators = {0};

#ifndef LIBNTHREAD_OS_WINDOWS
    pthread_mutexattr_t __libnthread_recursivemutexattr;
#endif

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