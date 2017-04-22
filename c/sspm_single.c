	
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
	volatile _SPM int *p = 0xF00A0000;
	volatile _SPM int *p2 = 0xF00A0010;
	volatile _SPM int *p3 = 0xF00A0004;
	
	printf("%x:%x.\n", (int) p, *p);
	printf("%x:%x.\n", (int) p2, *p2);
	printf("%x:%x.\n", (int) p3, *p3);

	printf("Assign %x = 0\n", (int)p);
	*p = 0;
	printf("Assign %x = 0\n", (int)p2);
	*p2 = 0;
	printf("Assign %x = 0\n", (int)p3);
	*p2 = 0;

	printf("%x:%x.\n", (int) p, *p);
	printf("%x:%x.\n", (int) p2, *p2);
	printf("%x:%x.\n", (int) p3, *p3);
	
	printf("Assign %x = 2\n", (int)p);
	*p = 0xFFFF;	

	printf("%x:%x.\n", (int) p, *p);
	printf("%x:%x.\n", (int) p2, *p2);
	printf("%x:%x.\n", (int) p3, *p3);

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

int main(){
	//f1();
	//verifyMemoryMapping();
	verifyMemoryMappingForSlaves();
}
//
























