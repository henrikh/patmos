#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.c"
#include "libsspm/coprint.h"

void slave(void* arg){
	int cpuid = get_cpuid();
	
	if(cpuid == 1){
		coprint_wait_send(1, "|core 1:0|\n");
		coprint_wait_send(1, "|core 1:1|\n");
		coprint_wait_send(1, "|core 1:2|\n");
	}else if(cpuid == 2){
		coprint_wait_send(2, "|core 2:0|\n");
		coprint_wait_send(2, "|core 2:1|\n");
		coprint_wait_send(2, "|core 2:2|\n");
	}else if(cpuid == 3){
		coprint_wait_send(3, "|core 3:0|\n");
		coprint_wait_send(3, "|core 3:1|\n");
		coprint_wait_send(3, "|core 3:2|\n");
	}	
}

int main()
{
	printf("Initializing string_to_master at %x.\n",LOWEST_SPM_ADDRESS);
	int end = coprint_initialize(LOWEST_SPM_ADDRESS);
	printf("Initializing ended at %x.\n", end);	


	int i;	
	for(i = 1; i< NR_CORES; i++){
		corethread_create(&i, &slave, NULL);
	}
	///*
	int core1 = 0, 
		core2 = 0,
		core3 = 0;

	while((core1<3) && (core2<3) && (core3<3)){
		if(core1<3){
			printf("Core 1:");			
			printf(coprint_wait_receive(1));
			printf("\n");
			core1++;
		}
		if(core2<3){
			printf("Core 2:");			
			printf(coprint_wait_receive(2));
			printf("\n");
			core2++;
		}
		if(core3<3){
			printf("Core 3:");			
			printf(coprint_wait_receive(3));
			printf("\n");
			core3++;
		}
	}
	//*/
	int *res;
	for(i = 1; i < NR_CORES; i++){
		corethread_join(i, (void **) &res);
	}

	printf("All cores done!\n");
}


