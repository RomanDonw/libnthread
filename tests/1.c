/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libnthread.h"

#include <stdio.h>

static NThreadReturnType t1_worker(void)
{
    for (int i = 0; i < 20; i++) putchar('a');
    return 9;
}

static NThreadReturnType t2_worker(void)
{
    for (int i = 0; i < 10; i++) putchar('b');
    return 3;
}

int main(void)
{
    NError nerr = libnthread_startup(NULL);
    if (nerr != NError_Success) { puts("failed to initialize the libnthread"); return 1; }

    puts("starting 2 threads");
    NThread *t1, *t2;
    if ((nerr = nthread_create(&t1, (void *)t1_worker, NULL)) != NError_Success) { puts("failed to create thread 1"); return 1; }
    if ((nerr = nthread_create(&t2, (void *)t2_worker, NULL)) != NError_Success) { puts("failed to create thread 2"); return 1; }

    NThreadReturnType ret;
    nthread_join(t1, &ret);
    printf("\nt1 ret: %hhu\n", ret);
    nthread_join(t2, &ret);
    printf("\nt2 ret: %hhu\n", ret);

    libnthread_cleanup();
    return 0;
}