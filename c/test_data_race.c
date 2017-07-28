#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/boot.h>
#include <machine/rtc.h>
#include "libcorethread/corethread.c"
#include "libsspm/coprint.h"
#include "libsspm/atomic.h"
#include "libsspm/led.h"
#include "libsspm/sspm_properties.h"


volatile _UNCACHED int ready[NR_CORES];

int core_running(int coreNr){
	return boot_info->slave[coreNr].status != STATUS_RETURN;
}

int any_slave_running(){
	int running = 0;

	for(int i = 1; i<NR_CORES; i++){
		running |= core_running(i);
	}
	return running;
}

void slave(void* arg){
	int cpuid = get_cpuid();
	int lock_addr = ((int*) arg)[0];
	volatile _SPM int *contest_addr = lock_addr + 4;
	
	char hex[11] = {0,0,0,0,0,0,0,0,0,0,0};
	
	coprint_slave_print(cpuid, "Lock: ");
	sprintf(hex, "%x", lock_addr);
	coprint_slave_print(cpuid, hex);

	sprintf(hex, "%x", *((volatile _SPM int *)lock_addr));
	coprint_slave_print(cpuid, hex);

	
	coprint_slave_print(cpuid, "Contest: ");
	sprintf(hex, "%x", contest_addr);
	coprint_slave_print(cpuid, hex);

	sprintf(hex, "%x", *contest_addr);
	coprint_slave_print(cpuid, hex);	

	ready[cpuid] = 1;
	
	//Wait for main to issue start
	while(!ready[0]){
		led_off();	
	}

	int stop = 0;
	for(int contest_length = 0; contest_length<200000 && !stop; contest_length++){
		int contest;
		lock(lock_addr);	
		led_on();
		//Get the value of the contest
		contest = *contest_addr;
		if(contest != 0){
			led_on_for(2000);

			// The contest is not 0, which means another core is using it too.
			char contest_length_string[11] = {0,0,0,0,0,0,0,0,0,0,0};		
			sprintf(contest_length_string, "%x", contest_length);

			char contest_string[11] = {0,0,0,0,0,0,0,0,0,0,0};		
			sprintf(contest_string, "%x", contest);
			
			coprint_slave_print(cpuid, "Cheat1 round:");
			coprint_slave_print(cpuid, contest_length_string);
			coprint_slave_print(cpuid, "Cheat1 value:");
			coprint_slave_print(cpuid, contest_string);
			stop = 1;			
		}else{
			// Increment the contest
			*contest_addr = *contest_addr + 1;

			// Reload the value of the contest
			contest = *contest_addr;
			if(contest != 1){
				led_on_for(2000);
				
				// The contest is being used by another core too
				char contest_length_string[11] = {0,0,0,0,0,0,0,0,0,0,0};		
				sprintf(contest_length_string, "%x", contest_length);

				char contest_string[11] = {0,0,0,0,0,0,0,0,0,0,0};		
				sprintf(contest_string, "%x", contest);
			
				coprint_slave_print(cpuid, "Cheat2 round:");
				coprint_slave_print(cpuid, contest_length_string);
				coprint_slave_print(cpuid, "Cheat2 value:");
				coprint_slave_print(cpuid, contest_string);
				stop = 1;
			}else{
				*contest_addr = *contest_addr - 1;
				contest = *contest_addr;
				if(contest != 0){
					led_on_for(2000);

					char contest_length_string[11] = {0,0,0,0,0,0,0,0,0,0,0};		
					sprintf(contest_length_string, "%x", contest_length);

					char contest_string[11] = {0,0,0,0,0,0,0,0,0,0,0};		
					sprintf(contest_string, "%x", contest);
			
					coprint_slave_print(cpuid, "Cheat3 round:");
					coprint_slave_print(cpuid, contest_length_string);
					coprint_slave_print(cpuid, "Cheat3 value:");
					coprint_slave_print(cpuid, contest_string);
					stop = 1;
				}
				///*
				if(cpuid == 3 && contest_length == 60000){
					*contest_addr = *contest_addr + 1;
				}
				//*/
			}		
		}
			
		release(lock_addr);
		led_off();
		//led_off_for(1);
		//Wait for random time
		///*
		int random = get_cpu_cycles() % 5;
		for(int i = 0; i<random; i++){
			led_off();
		}
		//*/
	}
	
	if(stop){
		coprint_slave_print(cpuid, "Error.");
	}else{
		coprint_slave_print(cpuid, "Success.");
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
	
	printf("Initial lock: %x\n", *((volatile _SPM int *) coprint_end));
	printf("Initial contest: %x\n", *contestP);
	
	for(int k = 0; k<NR_CORES; k++){
		ready[k] = 0;
	}
	
	for(int i = 1; i< NR_CORES; i++){
		corethread_create(&i, &slave, &coprint_end);
	}
		
	while(any_slave_running()){
		if(!ready[0]){
			int start = 1;
			for(int k = 1; k<NR_CORES; k++){
				start &= ready[k];
			}
			ready[0] = start;
			if(!ready[0]){
				printf("Slaves not ready.\n");
			}else{
				printf("Slaves ready.\n");
			}
		}		
			
		char buffer[CHANNEL_BUFFER_SIZE];
		for(int i = 1; i<NR_CORES; i++){
			if(coprint_try_receive(i,buffer)){
			printf("Core %x:",i);
			printf(buffer);
			printf("\n");
			}
		}
	}

	char buffer[CHANNEL_BUFFER_SIZE];
	for(int i = 1; i<NR_CORES; i++){
		if(coprint_try_receive(i,buffer)){
			printf("Core %x:",i);
			printf(buffer);
			printf("\n");
		}
	}
	
	int *res;
	for(int i = 1; i < NR_CORES; i++){
		corethread_join(i, (void **) &res);
	}

	printf("All cores done: %x!\n", *contestP);
}




