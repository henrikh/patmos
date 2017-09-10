#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "libcorethread/corethread.c"
#include "libmp/mp.h"
#include "libmp/mp_internal.h"
#include "libsspm/sspm_properties.h"

#define MP_CHAN_NUM_BUF 2
#define MP_CHAN_BUF_SIZE 40

const int NOC_MASTER = 0;

const int TIMES = 1000;
volatile _UNCACHED int ready;


void slave(void* args){
	volatile _SPM int* contention = (volatile _SPM int*) LOWEST_SPM_ADDRESS;

	while(!ready){led_off();}

	int v = 0;

	while(ready){
		for(int i = 0; i<(TIMES/2); i++){
			*contention = v;
			v = *contention;
		}
	}
}

int main(){
	
	int start, end;
	ready = 0;
	volatile _SPM int* contention = (volatile _SPM int*) LOWEST_SPM_ADDRESS;

	for(int i = 0; i<NOC_CORES; i++){
		
		for(int c = 1; c <= i; c++){
			corethread_create(&c, &slave, NULL);
		}
		ready = 1;
		asm volatile ("" : : : "memory");
		start = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		
		for(int k = 0; k<TIMES; k++){
			*contention = i;
		}

		asm volatile ("" : : : "memory");
		end = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		ready = 0;
		
		int res;
		for(int c = 1; c <= i; c++){
			corethread_join(c, (void **) &res);
		}
		printf("Traffik cores: %d\n", i);
		printf("Cycles: %d\n", end-start);
	}
	
	return 0;
}








