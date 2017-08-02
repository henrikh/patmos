#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "libcorethread/corethread.c"
#include "libsspm/led.h"
#include "libsspm/sspm_properties.h"

const int SENDER_RECEIVER_PAIRS = 2;

const int CHANNEL_BUFFER_CAPACITY = 54;

volatile _UNCACHED int ready[1+(2*SENDER_RECEIVER_PAIRS)];

volatile _UNCACHED int rwtime[1+(2*SENDER_RECEIVER_PAIRS)];

volatile _UNCACHED int intervals[1+(2*SENDER_RECEIVER_PAIRS)];

typedef enum {
	// Transmission has been finished by the sender.
	// Another transmission cannot begin before
	// the receiver acknowledges it.
	ENDED, 
	// Transmission end has been acknowledged by the receiver.
	// Should also be the initial state of the channel after initialization.
	ACKNOWLEDGED
} TRANSMISSION_STATE;

struct SSPM_MP_DESC{
	//The number of elements in the buffer
	unsigned int size;
	//The number of elements in the buffer
	unsigned int capacity;
	//Whether a transmission is in progress
	TRANSMISSION_STATE transmission;               
};

int sspm_mp_occupies(volatile _SPM struct SSPM_MP_DESC *channel){
	return sizeof(struct SSPM_MP_DESC) + (sizeof(int) * (channel->capacity));
}

volatile _SPM void* 
sspm_mp_init(
	volatile _SPM struct SSPM_MP_DESC *channel_start, 
	int capacity
){
	channel_start->size = 0;
	channel_start->capacity = capacity;
	channel_start->transmission = ACKNOWLEDGED;

	return ((int)channel_start) + sspm_mp_occupies(channel_start);
}

void sender_slave(void* args){
	volatile _SPM struct SSPM_MP_DESC *channel = (volatile _SPM struct SSPM_MP_DESC *)args;
	volatile _SPM int *buffer = channel + 1;

	int data = 0;
	
	ready[get_cpuid()] = 1;

	while(!ready[0]){led_off();}
	
	led_on();

	unsigned long long start;
	unsigned long long end;
	unsigned long long sent;
	unsigned long long received;
	
	if(channel->transmission == ACKNOWLEDGED) {
		const int size = channel->capacity;
		start = get_cpu_cycles();
		
		for(int i = 0; i < size; i++){
			buffer[i] = 1;
		}
		end = get_cpu_cycles();
	}
	
	sent = get_cpu_cycles();
	channel->transmission = ENDED;
	while(channel->transmission == ENDED) {}
	received = get_cpu_cycles();
	
	led_off();
	rwtime[get_cpuid()] = (int) end - start;
	intervals[get_cpuid()] = (int) received - sent;
}

void receiver_slave(void* args){
	volatile _SPM struct SSPM_MP_DESC *channel = (volatile _SPM struct SSPM_MP_DESC *)args;
	volatile _SPM int *buffer = channel + 1;

	int data[channel->capacity];
	
	ready[get_cpuid()] = 1;
	while(!ready[0]){led_off();}
	led_on();
	
	unsigned long long start;
	unsigned long long end;
	while(1) {
		if(channel->transmission == ENDED) {
			channel->transmission = ACKNOWLEDGED;
	
			int size = channel->size;
			start = get_cpu_cycles();
	
			asm volatile ("" : : : "memory");
			for(int i = 0; i < CHANNEL_BUFFER_CAPACITY; i++){
				data[i] = buffer[i];
			}
			asm volatile ("" : : : "memory");
			end = get_cpu_cycles();
			
			break;
		}
	}
	
	led_off();
	rwtime[get_cpuid()] = (int) end - start;
}

int main(){
	led_off();

	volatile _SPM struct SSPM_MP_DESC *channel[2*SENDER_RECEIVER_PAIRS];
	channel[0] = LOWEST_SSPM_ADDRESS;
	
	int size = sspm_mp_init(channel[0], CHANNEL_BUFFER_CAPACITY);

	for(int i = 1; i < SENDER_RECEIVER_PAIRS; i++){
		channel[i] = size + 4;
		size = sspm_mp_init(channel[i], CHANNEL_BUFFER_CAPACITY);
		printf("Buffer size: %x.\n", size);
	}

	for(int i = 0; i< (1+(2*SENDER_RECEIVER_PAIRS)); i++){
		ready[i] = 0;
	}

	for(int i = 1; i <= SENDER_RECEIVER_PAIRS; i++){
		int core = i*2;
		corethread_create(&core, &sender_slave, (void*)channel[i-1]);
		core--;
		corethread_create(&core, &receiver_slave, (void*)channel[i-1]);
	}

	while(!ready[0]){
		int start = 1;
		for(int k = 1; k<(1+(2*SENDER_RECEIVER_PAIRS)); k++){
			start &= ready[k];
		}
		ready[0] = start;
	}
	led_on();
	
	int start = get_cpu_cycles();
	int *res;
	for(int i = 1; i<(1+(2*SENDER_RECEIVER_PAIRS)); i++){
		corethread_join(i, (void **) &res);
	}
	int end = get_cpu_cycles();

	printf("Cycles spent: %d.\n", (end - start));
	
	for(int i = 1; i <= 2*SENDER_RECEIVER_PAIRS; i += 2){
		printf("Receive\t(%d): %d.\n", i, rwtime[i]);
		printf("Send\t(%d): %d.\n", i+1, rwtime[i+1]);
	}
	
	for(int i = 0; i <= 2*SENDER_RECEIVER_PAIRS; i++){
		printf("Cycles spent (%d): %d.\n", i, intervals[i]);
	}
	
	return 0;
}
























