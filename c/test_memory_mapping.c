#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.c"
#include "libsspm/smp.h"
#include "libsspm/led.h"
#include "libsspm/sspm_properties.h"


void slaveCompleteMemoryMapVerification(void* args){
	
	led_on();
	int cpuid = get_cpuid();
	int intervalSize = (TOTAL_SHARED_MEMORY/1);
	int startAddr = LOWEST_SPM_ADDRESS + (intervalSize * get_cpuid());
	int endAddr = startAddr + intervalSize ;
	int wordCount = intervalSize/4;
	
	int i;
	volatile _SPM int *p = (volatile _SPM int *) startAddr;

	//reset all memory addresses
	for(i = 0; i < wordCount; i++){
		p[i] = 0;
	}

	int t = 0xFFFFFFFF,j, fail = 0;
		
	for(i = 0; i<wordCount; i++){
		p[i] = t;
		
		for(j = 0; j <wordCount; j++){
			if(j!=i && p[j] != 0){
				if(cpuid == 0){
					printf("j = %x, i = %x, %x = %x\n", j,i,(int)(p+j), p[j]);
				}
				fail = 1;
			}
		}
		p[i] = 0;
	}
	
	if(fail){
		led_blink(100);
	}

	led_off();
}

int main(){

	slaveCompleteMemoryMapVerification(NULL);

	int *res;
	for(int i = 1; i< NR_CORES; i++){
		corethread_create(&i, &slaveCompleteMemoryMapVerification, NULL);
		corethread_join(i, (void **) &res);
	}
	printf("Memory verified.");
}


























