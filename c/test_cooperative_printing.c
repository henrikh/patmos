#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.c"
#include "libsspm/string_to_master.h"

void slave(void* arg){
	int cpuid = get_cpuid();
	
	if(cpuid == 1){
		int i = 0;
		send_from_core(1, "|core 1:0|\n");
		while(i<2){
			if(can_send_from(1)){
				if(i==0){
					send_from_core(1, "|core 1:1|\n");
				}
				if(i == 1){
					send_from_core(1, "|core 1:2|\n");
				}
				i++;
			}
		}
	}else if(cpuid == 2){
		int i = 0;
		send_from_core(2, "|core 2:0|\n");
		while(i<2){
			if(can_send_from(2)){
				if(i==0){
					send_from_core(2, "|core 2:1|\n");
				}
				if(i == 1){
					send_from_core(2, "|core 2:2|\n");
				}
				i++;
			}
		}
	}else if(cpuid == 3){
		int i = 0;
		send_from_core(3, "|core 3:0|\n");
		while(i<2){
			if(can_send_from(3)){
				if(i==0){
					send_from_core(3, "|core 3:1|\n");
				}
				if(i == 1){
					send_from_core(3, "|core 3:2|\n");
				}
				i++;
			}
		}
	}	
}

int main()
{
	printf("Initializing string_to_master at %x.\n",LOWEST_SPM_ADDRESS);
	int end = initialize_messaging(LOWEST_SPM_ADDRESS);
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
		if(core1<3 && can_receive_from_core(1)){
			printf("Core 1:");
			printf(receive_from_core(1));
			printf("\n");
			free_received_from_core(1);
			core1++;
		}
		if(core2<3 && can_receive_from_core(2)){
			printf("Core 2:");			
			printf(receive_from_core(2));
			printf("\n");
			free_received_from_core(2);
			core2++;
		}
		if(core3<3 && can_receive_from_core(3)){
			printf("Core 3:");			
			printf(receive_from_core(3));
			printf("\n");
			free_received_from_core(3);
			core3++;
		}
	}
	//*/
	int *res;
	for(i = 1; i < NR_CORES; i++){
		corethread_join(i, (void **) &res);
	}

	printf("All cores done!\n");
	/*
	printf("Can receive: %x\n", can_receive_from_core(1));
	printf("Received: '");
	printf(receive_from_core(1));
	printf("'\n");
	printf("Can receive: %x\n", can_receive_from_core(1));
	printf("Clear.\n");
	free_received_from_core(1);
	printf("Can receive: %x\n", can_receive_from_core(1));
	*/
}


