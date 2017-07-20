#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.c"
#include "libsspm/string_to_master.h"

#define FLAG_ADDRESS 	(LOWEST_SPM_ADDRESS)
#define LOCK_ADDRESS 	(LOWEST_SPM_ADDRESS + 4)
#define CONTEST_ADDRESS 	(LOWEST_SPM_ADDRESS + 8)

volatile _SPM int const *flag = FLAG_ADDRESS;
volatile _SPM int const *lock = LOCK_ADDRESS;
volatile _SPM int const *contest = CONTEST_ADDRESS;


void slave(void* arg){
	int cpuid = get_cpuid();
	
	if(cpuid == 1){
		int i = 0;
		send_from_core(1, "|core 1:0|\n");
		///*
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
		//*/
		
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
	i = 0;
	while((i<3)){
		if(can_receive_from_core(1)){
			printf(receive_from_core(1));
			free_received_from_core(1);
			i++;
		}
	}
	//*/
	int *res;
	for(i = 1; i < NR_CORES; i++){
		corethread_join(i, (void **) &res);
	}
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


