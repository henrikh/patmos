
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/boot.h>
#include "libcorethread/corethread.c"
#include "libsspm/coprint.h"
#include "libsspm/atomic.h"
#include "libsspm/led.h"


int main() {
        volatile _SPM int *lock_p = LOWEST_SSPM_ADDRESS;
        
        *lock_p = 0;
        if( *lock_p != 0){
                printf("lock not reset");
        }
        
        lock(lock_p);
        if(*lock_p != 1){
                printf("Not locking.");
        } else {
        	printf("Locked");
        }
        
        release(lock_p);
        if(*lock_p != 0){
                printf("Not releasing.");
        }
        
        
        lock(lock_p);
        
        lock(lock_p);
}



