#include "libnthread.h"

#include <stdio.h>

#define PRINTBOOL(value) (puts((value) ? "true" : "false"))

static NThreadAtomicBool flag2 = NTHREAD_ATOMICBOOLINIT(false);

int main(void)
{
    puts("1:");
    NThreadAtomicBool flag = NTHREAD_ATOMICBOOLINIT(false);
    PRINTBOOL(nthread_atomicbool_load(&flag));
    
    nthread_atomicbool_store(&flag, true);
    PRINTBOOL(nthread_atomicbool_load(&flag));

    puts("\n2:");
    PRINTBOOL(nthread_atomicbool_load(&flag2));
    
    nthread_atomicbool_store(&flag2, true);
    PRINTBOOL(nthread_atomicbool_load(&flag2));

    return 0;
}