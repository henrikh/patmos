	
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.c"
#include "libsspm/smp.h"

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
	initialize_all_channels();
	volatile _SPM int *p = 0xF0040000;
	volatile _SPM int *p2 = 0xF0040010;

	*p = 0;
	*p2 = 0;

	printf("0xF0040000:%x.\n", *p);
	printf("0xF0040010:%x.\n", *p2);

	*p = 1;	

	printf("0xF0040000:%x.\n", *p);
	printf("0xF0040010:%x.\n", *p2);
}

int main(){
	f1();
	//verifyMemoryMapping();
	
}
//
























