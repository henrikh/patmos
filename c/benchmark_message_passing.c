#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "libcorethread/corethread.c"
#include "libsspm/led.h"
#include "libsspm/sspm_properties.h"
#include "libsspm/mp.h"


const int SENDER_RECEIVER_PAIRS = 1;

const int DATA_TO_SEND = 512;

//The channel capacity will be 2 to the power of this number
const int CHANNEL_BUFFER_CAPACITY = 5;

const int SENDER_BUFFER_CAPACITY = 128;

const int RECEIVER_BUFFER_CAPACITY = 128;

int ready[1+(2*SENDER_RECEIVER_PAIRS)];

void sender_slave(void* args){

	volatile _SPM struct SSPM_MP_DESC *channel = (volatile _SPM struct SSPM_MP_DESC *)args;

	int progress = 0;
	int send_buffer[SENDER_BUFFER_CAPACITY];
	int sender_buffer_sent = SENDER_BUFFER_CAPACITY;	
	
	ready[get_cpuid()] = 1;

	while(!ready[0]){led_off();}

	while((progress < DATA_TO_SEND) || (sender_buffer_sent < SENDER_BUFFER_CAPACITY) ){
		if(sender_buffer_sent == SENDER_BUFFER_CAPACITY){
			for(int i = 0; i<SENDER_BUFFER_CAPACITY; i++){
				send_buffer[i] = progress++;
			}
			sender_buffer_sent = 0;
		}
		int sent = sspm_mp_try_send(channel, &progress,
						sender_buffer_sent,SENDER_BUFFER_CAPACITY - sender_buffer_sent);
		sender_buffer_sent += sent;
	}
}

void receiver_slave(void* args){
	
	volatile _SPM struct SSPM_MP_DESC *channel = (volatile _SPM struct SSPM_MP_DESC *)args;

	int progress = 0;
	int receiver_buffer[RECEIVER_BUFFER_CAPACITY];
	
	ready[get_cpuid()] = 1;

	while(!ready[0]){led_off();}

	while( progress < DATA_TO_SEND){
		int received = sspm_mp_try_receive(channel, receiver_buffer, 0, RECEIVER_BUFFER_CAPACITY);
				
		for(int i=0; i<received; i++){
			receiver_buffer[i] = 0;
			progress++;
		}	
	}
}

void main(){

	volatile _SPM struct SSPM_MP_DESC *channel = LOWEST_SSPM_ADDRESS;
	
	sspm_mp_init(channel, CHANNEL_BUFFER_CAPACITY);

	for(int i = 0; i< (1+(2*SENDER_RECEIVER_PAIRS)); i++){
		ready[i] = 0;
	}
	
	for(int i = 0; i< (2*SENDER_RECEIVER_PAIRS);i++){
		if(i%2){
			corethread_create(&i, &sender_slave, (void*)channel);
		}else{
			corethread_create(&i, &receiver_slave, (void*)channel);
		}
	}
	

	
	while(!ready[0]){
		int start = 1;
		for(int k = 1; k<(1+(2*SENDER_RECEIVER_PAIRS)); k++){
			start &= ready[k];
		}
		ready[0] = start;
	}
	
	int start = get_cpu_cycles();

	int *res;
	for(int i = 1; i<(1+(2*SENDER_RECEIVER_PAIRS)); i++){
		corethread_join(i, (void **) &res);
	}
	int end = get_cpu_cycles();

	printf("Cycles spent: %x.\n", (end - start));
}
























