#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.c"
#include "libsspm/led.h"
#include "libsspm/sspm_properties.h"


void slave(void* args){
	int turn = *((int*) args);
	int main = !turn;
	volatile _SPM int *current = LOWEST_SSPM_ADDRESS;

	while(current < (LOWEST_SSPM_ADDRESS + TOTAL_SHARED_MEMORY)){
		if(turn){
			while(*current <50){
				*current = *current + 1;
				if(main){
					printf("Current1: %x\n", *current);
				}else{
					led_on_for(1);
					led_off_for(3);				
				}
			}
			
			while(*current != 0){
				if(main){
					printf("Current2: %x\n", *current);
				}else{
					led_on_for(100);
					led_off_for(100);				
				}		
			}
		}else{
			while(*current < 50){
				if(main){
					printf("Current3: %x\n", *current);
				}else{
					led_on_for(1);
					led_off_for(3);				
				}
			}
			*current = 0;
			/*
			int random = get_cpu_cycles() % 5;
			int sum = 0;
			for(int i = 0; i<random; i++){
				sum = *current;
			}
			//*/
			
		}	
		turn = !turn;
		if(main){
			printf("Current address %x: %x\n", current, *current);
		}
		current++;	
		if(main){
			printf("Current address %x: %x\n", current, *current);
		}
			
	}
}

int main(){
	led_off();

	//Reset SSPM
	volatile _SPM int *sspm = LOWEST_SSPM_ADDRESS;
	for(int i = 0; (i*4)<TOTAL_SHARED_MEMORY;i++){
		//printf("%x:%x",((int)sspm)+(i*4),sspm[i]);
		sspm[i] = 0;
		//printf(" -> %x\n", sspm[i]);
	}

	printf("Starting memory contention test.\n");

	int *res;
	int turn = 1;
	int slaveId = 1;
	corethread_create(&slaveId, &slave, &turn);

	printf("Slave started\n");

	int myTurn = 0;
	slave(&myTurn);
	
	printf("Main done");

	corethread_join(slaveId, (void **) &res);
	
	printf("Memory contention verified.\n");
	return 0;
}


























