
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.c"
#include "libsspm/smp.h"


#define LED ( *( ( volatile _IODEV unsigned * ) 0xF0090000))
#define COUNT_LIMIT (100)

typedef enum {RCV, SEND, MULTI_RCV, MULTI_SEND} msg_t;

void circular_passing(void *arg);
int task3();
void masterPrinter();
void slavePrinter();

int main(){
	task3();
}

int task3(){
	initialize_all_channels();
	int parameters[NR_CORES] = {0,1,0,0};
	corethread_t i;
	for(i = 1; i< NR_CORES; i++){
		corethread_create(&i, &circular_passing, parameters + i);
		//corethread_create(&i, &slavePrinter, parameters + i);
	}
	
	//circular_passing(parameters);
	masterPrinter();

	int *res;
	for(i = 1; i < NR_CORES; i++){
		corethread_join(i, (void **) &res);
	}
	
	
	return *res;
}

void slavePrinter(){
	struct PACKET_T p;

	p.sender = get_cpuid();
	p.payload = SEND;

	while(!send(0, p)){}
	while(!send(0, p)){}
	while(!send(0, p)){}
}

void masterPrinter(){
	struct PACKET_OPTION option;
	struct PACKET_T p;
	int count;
	while(1){
		count = 0;
		while(!(option = receive(get_cpuid())).valid){
			if(count == COUNT_LIMIT){
				printf("MAIN:MULTI_RCV\n");
				count = 0;
			}
			count++;
		}

		p = option.packet;
	
		printf("%x:", p.sender);
		if(p.payload == RCV){
			printf("RCV") ;
		}else if(p.payload == SEND){
			printf("SEND") ;
		}else if(p.payload == MULTI_RCV){
			printf("MULTI_RCV") ;
		}else if(p.payload == MULTI_SEND){
			printf("MULTI_SEND") ;
		}else{
			printf("unknown(%x)", p.payload);
		}
		printf("\n");
	}
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

void circular_passing(void *arg){
	struct PACKET_T p;

	int cpu_id = get_cpuid();

	int cpu_id_next = (cpu_id == (3))? 1: cpu_id + 1;
	
	int token = 1;
	
	int first = *((int *) arg);

	if(first){
		led_on_for(3000);
		p.payload = token;
		p.sender = cpu_id;
		send(cpu_id_next, p);
	}
	led_off();
	
	struct PACKET_OPTION option;
	
	int rcvCount, sendCount;
	struct PACKET_T masterP;
	masterP.sender = cpu_id;
	while(1){
		//Recieve
		rcvCount = 0;
		while(!(option = receive(cpu_id)).valid){
			///*			
			if(rcvCount == COUNT_LIMIT){
				masterP.payload = MULTI_RCV;
				while(!send(0, masterP)){}
				rcvCount = 0;
			}
			rcvCount++;
			//*/
		}
		///*
		masterP.payload = RCV;
		while(!send(0, masterP)){}
		//*/
		
		p = option.packet;

		//Wait
		led_on_for(1000);	

		//Send token
		p.sender = cpu_id;
		///*		
		sendCount = 0;
		//*/
		while(!send(cpu_id_next, p)){
			///*
			if(sendCount == COUNT_LIMIT){
				masterP.payload = MULTI_SEND;
				while(!send(0, masterP)){}
				sendCount = 0;
			}
			sendCount++;
			//*/	
		}
		///*
		masterP.payload = SEND;
		while(!send(0, masterP)){}
		//*/
		led_off();
	}
}
























