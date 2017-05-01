	
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.c"
#include "libsspm/smp.h"

#define LED ( *( ( volatile _IODEV unsigned * ) 0xF0090000))

void f1(){
	printf("Starting program.\n");
		
	printsomething();

	//Initialize channels. Initializes 3 channels, but we use only the first.
	initialize_all_channels();
	
	printf("Initialized channels.\n");
	
	struct PACKET_T p;
	p.payload = 3;
	p.sender = get_cpuid();
	
	
	send(get_cpuid(), p);
	
	printf("Sent: {%x,%x}\n", p.payload, p.sender);
	
	struct PACKET_OPTION option = receive(get_cpuid());
	
	if( !option.valid ){
		printf("Could not receive expected value.\n");
	}else{
		printf("Received value: {%x,%x}.\n", option.packet.payload, option.packet.sender);
	}
}

void verifyMemoryMapping(){
	
	printf("Start verifying memory mapping.\n");
	volatile _SPM int *p =  0xF00A0000;
	volatile _SPM int *p2 = 0xF00A0100;
	volatile _SPM int *p3 = 0xF00A0E00;
	volatile _SPM int *p4 = 0xF00A0F00;
	
	printf("%x:%x.\n", (int) p, *p);
	printf("%x:%x.\n", (int) p2, *p2);
	printf("%x:%x.\n", (int) p3, *p3);
	printf("%x:%x.\n", (int) p4, *p4);

	printf("Assign %x = 0\n", (int)p);
	*p = 0;
	printf("Assign %x = 0\n", (int)p2);
	*p2 = 0;
	printf("Assign %x = 0\n", (int)p3);
	*p3 = 0;
	printf("Assign %x = 0\n", (int)p4);
	*p4 = 0;

	printf("%x:%x.\n", (int) p, *p);
	printf("%x:%x.\n", (int) p2, *p2);
	printf("%x:%x.\n", (int) p3, *p3);
	printf("%x:%x.\n", (int) p4, *p4);
	
	printf("Assign %x = 0xFFFF\n", (int)p);
	*p = 0xFFFF;	

	printf("%x:%x.\n", (int) p, *p);
	printf("%x:%x.\n", (int) p2, *p2);
	printf("%x:%x.\n", (int) p3, *p3);
	printf("%x:%x.\n", (int) p4, *p4);

	printf("End of verifying\n");
}

void led_on(){
	LED = 1;
}

void led_off(){
	LED = 0;
}

void blink(int period){
	for(;;){
		for(int i = 400000+14117*period; i!= 0; --i) {led_on();}
		for(int i = 400000+14117*period; i!= 0; --i) {led_off();}
	}
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

void slave(void* args){
	led_off_for(4000 * (get_cpuid()+1));
	
	led_on_for(1000);

	volatile _SPM int *p = 0xF00A0000 + get_cpuid();
	volatile _SPM int *p2 = 0xF00A0004 + get_cpuid();
	
	*p = 0;
	*p2 = 0;

	*p = 1;

	if(*p2 == 1){
		//There is an error
		led_off_for(1000);
		led_on_for(1000);
	}
		
	led_off();
}

void verifyMemoryMappingForSlaves(){
	
	/*
		Use:
			Look at each code LED in sequence and verify the following:
			When a core starts its test it will turn on its LED for 1 second
			If the memory mapping is correct, the LED will turn off and the next core
			will continue.
			If the memory mapping is wrong, the LED will turn off for 1 second
			then turn on again for 1 second, after which it will turn off for good
			and the next core will run.
	*/

	corethread_t i;
	for(i = 1; i< NR_CORES; i++){
		corethread_create(&i, &slave, NULL);
	}
	
	slave(NULL);

	int *res;
	for(i = 1; i < NR_CORES; i++){
		corethread_join(i, (void **) &res);
	}
}

void slaveCompleteMemoryMapVerification(void* args){
	
	led_on();
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

	int t = get_cpuid()+10,j, fail = 0;
		
	for(i = 0; i<wordCount; i++){
		p[i] = t;
		
		for(j = 0; j <wordCount; j++){
			if(j!=i && p[j] != 0){
				printf("j = %x, i = %x, %x = %x\n", j,i,(int)(p+j), p[j]);
				fail = 1;
			}
		}
		p[i] = 0;
	}
	
	if(fail){
		blink(100);
	}

	led_off();
}

void completeMemoryMapVerification(){
	/*
		Use:
			
	*/

	/*corethread_t i;
	for(i = 1; i< NR_CORES; i++){
		corethread_create(&i, &slave, NULL);
	}*/
	
	slaveCompleteMemoryMapVerification(NULL);

	/*int *res;
	for(i = 1; i < NR_CORES; i++){
		corethread_join(i, (void **) &res);
	}*/
}

int main(){
	//f1();
	verifyMemoryMapping();
	//verifyMemoryMappingForSlaves();
	//completeMemoryMapVerification();
}
//
























