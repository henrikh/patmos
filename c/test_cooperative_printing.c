#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.h"
#include "libsspm/coprint.h"

void slave(void* arg){
	int cpuid = get_cpuid();
	if(cpuid == 1){
		coprint_slave_print(1, "|1111111111111111111111111:0|\n");
		coprint_slave_print(1, "|1111111111111111111111111:1|\n");
		coprint_slave_print(1, "|1111111111111111111111111:2|\n");
	}else if(cpuid == 2){
		coprint_slave_print(2, "|2222222222222222222222222:0|\n");
		coprint_slave_print(2, "|2222222222222222222222222:1|\n");
		coprint_slave_print(2, "|2222222222222222222222222:2|\n");
	}else if(cpuid == 3){
		coprint_slave_print(3, "|3333333333333333333333333:0|\n");
		coprint_slave_print(3, "|3333333333333333333333333:1|\n");
		coprint_slave_print(3, "|3333333333333333333333333:2|\n");
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
	int msg = 2;
	while((core1<msg) && (core2<msg) && (core3<msg)){
		if(core1<msg){
			printf("Core 1:");			
			printf(coprint_wait_receive(1));
			printf("\n");
			core1++;
		}
		if(core2<msg){
			printf("Core 2:");			
			printf(coprint_wait_receive(2));
			printf("\n");
			core2++;
		}
		if(core3<msg){
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


