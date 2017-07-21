#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/boot.h>
#include "libcorethread/corethread.c"
#include "libsspm/coprint.h"
#include "libsspm/atomic.h"

#define LED ( *( ( volatile _IODEV unsigned * ) 0xF0090000))
void led_on(){
	LED = 1;
}

void led_off(){
	LED = 0;
}
#define MS_CLOCK (18000)
void led_on_for(int ms){
	int i,j;
	for(i = 0; i < ms; i++){
		for(j = 0; j < MS_CLOCK; j++){
			led_on();
		}
	}
}

void led_off_for(int ms){
	int i,j;
	for(i = 0; i < ms; i++){
		for(j = 0; j < MS_CLOCK; j++){
			led_off();
		}
	}
}

int core_running(int coreNr){
	return boot_info->slave[coreNr].status != STATUS_RETURN;
}

void slave_old(void* arg){
	led_off();
	int cpuid = get_cpuid();
	
	int lock_addr = ((int*) arg)[0];
	volatile _SPM int *contest_addr = lock_addr + 4;
	
	//char hex[8];
	//sprintf(hex, "%x", coprint_flag_address(cpuid));
	//sprintf(hex, "%x", lock_addr);
	
	//coprint_slave_print(cpuid, "lock address: ");
	//coprint_slave_print(cpuid, hex);

	//sprintf(hex, "%x", coprint_local_buffer(cpuid));
	//sprintf(hex, "%x", contest_addr);
	//coprint_slave_print(cpuid, "Contest address: ");
	//coprint_slave_print(cpuid, hex);
	
	for(int contest_length = 0; contest_length<6000; contest_length++){

		//Take contest
		int contest;
		while(1){
			lock(lock_addr);
			//led_on();
			//coprint_slave_print(cpuid,"lock");
			//led_on_for(2000);
			contest = *contest_addr;
	
	

			if(contest != 0){
				//char contest_string[4] = {0,0,0,0};		
				//sprintf(contest_string, "%x", contest);

				//coprint_slave_print(cpuid, "Contest taken:");
				//coprint_slave_print(cpuid, contest_string);

				release(lock_addr);
				//led_off();
				//coprint_slave_print(cpuid,"release");
			}else{
				*contest_addr = cpuid;
				//char contest_string[4] = {0,0,0,0};		
				//sprintf(contest_string, "%x", *contest_addr);
			
				//coprint_slave_print(cpuid, "Taking contest:");
				//coprint_slave_print(cpuid, contest_string);

				release(lock_addr);
				//led_off();
				//coprint_slave_print(cpuid,"release");			

				break;
			}
		
		}
	
	
		lock(lock_addr);
		//led_on();
		//coprint_slave_print(cpuid,"lock");
		//led_on_for(2000);
		
		//Release contest
		contest = *contest_addr;
		if(contest != cpuid){
			char contest_length_string[4] = {0,0,0,0};		
			sprintf(contest_length_string, "%x", contest_length);

			char contest_string[4] = {0,0,0,0};		
			sprintf(contest_string, "%x", contest);
			
			coprint_slave_print(cpuid, "Contest cheat at:");
			coprint_slave_print(cpuid, contest_length_string);
			coprint_slave_print(cpuid, "Contest cheat with:");
			coprint_slave_print(cpuid, contest_string);
		}else{
			*contest_addr = 0;
			//char contest_string[4] = {0,0,0,0};		
			//sprintf(contest_string, "%x", *contest_addr);
			
			//coprint_slave_print(cpuid, "Release contest:");
			//coprint_slave_print(cpuid, contest_string);
		}

		release(lock_addr);
		//led_off();
		//coprint_slave_print(cpuid,"release");	
	}
	coprint_slave_print(cpuid, "Done.");
}

void slave(void* arg){
	int cpuid = get_cpuid();
	
	int lock_addr = ((int*) arg)[0];
	volatile _SPM int *contest_addr = lock_addr + 4;
	
	
	int stop = 0;
	for(int contest_length = 0; contest_length<200000 && !stop; contest_length++){
		//if(! (contest_length % 1000)){
		//	coprint_slave_print(cpuid, "Mark");
		//}
		int contest;
		
		lock(lock_addr);
		
		contest = *contest_addr;
		if(contest != 0){
			char contest_length_string[4] = {0,0,0,0};		
			sprintf(contest_length_string, "%x", contest_length);

			char contest_string[4] = {0,0,0,0};		
			sprintf(contest_string, "%x", contest);
			
			coprint_slave_print(cpuid, "Contest cheat1 at:");
			coprint_slave_print(cpuid, contest_length_string);
			coprint_slave_print(cpuid, "Contest cheat1 with:");
			coprint_slave_print(cpuid, contest_string);
			stop = 1;
		}else{
			*contest_addr = *contest_addr + 1;
			contest = *contest_addr;
			if(contest != 1){
				char contest_length_string[4] = {0,0,0,0};		
				sprintf(contest_length_string, "%x", contest_length);

				char contest_string[4] = {0,0,0,0};		
				sprintf(contest_string, "%x", contest);
			
				coprint_slave_print(cpuid, "Contest cheat2 at:");
				coprint_slave_print(cpuid, contest_length_string);
				coprint_slave_print(cpuid, "Contest cheat2 with:");
				coprint_slave_print(cpuid, contest_string);
				stop = 1;
			}else{
				*contest_addr = *contest_addr - 1;
				contest = *contest_addr;
				if(contest != 0){
					char contest_length_string[4] = {0,0,0,0};		
					sprintf(contest_length_string, "%x", contest_length);

					char contest_string[4] = {0,0,0,0};		
					sprintf(contest_string, "%x", contest);
			
					coprint_slave_print(cpuid, "Contest cheat3 at:");
					coprint_slave_print(cpuid, contest_length_string);
					coprint_slave_print(cpuid, "Contest cheat3 with:");
					coprint_slave_print(cpuid, contest_string);
					stop = 1;
				}
				//For testing, insert an error
				if(contest_length == 2 && cpuid == 2){
					*contest_addr = *contest_addr + 1;
				}
			}		
		}
			
		release(lock_addr);
	}
	if(stop){
		coprint_slave_print(cpuid, "Erroneous end.");
	}else{
		coprint_slave_print(cpuid, "Successful end.");
	}
}

int main()
{
	printf("Initializing string_to_master at %x.\n",LOWEST_SPM_ADDRESS);
	int coprint_end = coprint_initialize(LOWEST_SPM_ADDRESS);
	printf("Initializing ended at %x.\n", coprint_end);	
	
	release(coprint_end); //reset lock
	volatile _SPM int *contestP = (coprint_end+4);
	*contestP = 0;//reset the contest

	int i;	
	for(i = 1; i< NR_CORES; i++){
		corethread_create(&i, &slave, &coprint_end);
	}
	///*
	int core1 = 0, 
		core2 = 0,
		core3 = 0;	
	
	while(core_running(1) || core_running(2) || core_running(3)){
		char buffer[CHANNEL_BUFFER_SIZE];
		if(coprint_try_receive(1,buffer)){
			printf("Core 1:");
			printf(buffer);
			printf("\n");
		}
		if(coprint_try_receive(2,buffer)){
			printf("Core 2:");
			printf(buffer);
			printf("\n");
		}
		if(coprint_try_receive(3,buffer)){
			printf("Core 3:");
			printf(buffer);
			printf("\n");
		}
	}
	char buffer[CHANNEL_BUFFER_SIZE];
	if(coprint_try_receive(1,buffer)){
		printf("Core 1:");
		printf(buffer);
		printf("\n");
	}
	if(coprint_try_receive(2,buffer)){
		printf("Core 2:");
		printf(buffer);
		printf("\n");
	}
	if(coprint_try_receive(3,buffer)){
		printf("Core 3:");
		printf(buffer);
		printf("\n");
	}
	

	//*/
	int *res;
	for(i = 1; i < NR_CORES; i++){
		corethread_join(i, (void **) &res);
	}

	printf("All cores done: %x!\n", *contestP);
}




