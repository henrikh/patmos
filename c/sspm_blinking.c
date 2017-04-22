
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.c"
#include "libsspm/smp.h"


#define LED ( *( ( volatile _IODEV unsigned * ) 0xF0090000))


void circular_passing(void *arg);
int task3();

int main(){
	task3();
}

int task3(){
	int parameters[NR_CORES-1] = {1,0,0};
	corethread_t i;
	for(i = 1; i< NR_CORES; i++){
		corethread_create(&i, &circular_passing, parameters + (i-1));
	}

	
	int *res;
	for(i = 1; i < NR_CORES; i++){
		corethread_join(i, (void **) &res);
	}
	
	
	return *res;
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

void simpleBlinker( void* arg){
	blink(*(int *) arg);
}


void circular_passing(void *arg){
	struct PACKET_T p;

	int cpu_id = get_cpuid();

	int cpu_id_next = (cpu_id == 2)? 1: cpu_id + 1;
	
	int token = 1;
	
	int first = *((int *) arg);

	if(first){
		led_on();
		p.payload = token;
		p.sender = cpu_id;
		send(cpu_id_next, p);
	}
	led_off();
	
	struct PACKET_OPTION option;

	while(1){
		//Recieve
		while( !(option = receive(cpu_id)).valid ){}
		p = option.packet;

		//Wait
		for(int i = (400000+14117*255); i!= 0; --i) {led_on();}	

		//Send token
		p.sender = cpu_id;
		send(cpu_id_next, p);

		led_off();
	}
}
























