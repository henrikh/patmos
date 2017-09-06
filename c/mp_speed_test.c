#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "libcorethread/corethread.c"
#include "libsspm/led.h"
#include "libmp/mp.h"

const int SENDER_RECEIVER_PAIRS = 4;

const int DATA_TO_SEND = 512;

//The channel capacity will be 2 to the power of this number
const int CHANNEL_BUFFER_CAPACITY = 32;

volatile _UNCACHED int ready[1+(2*SENDER_RECEIVER_PAIRS)];

volatile _UNCACHED int intervals[1+(2*SENDER_RECEIVER_PAIRS)];

void sender_slave(void* args){
	volatile _SPM struct SSPM_MP_DESC *channel = (volatile _SPM struct SSPM_MP_DESC *)args;
	volatile _SPM int *buffer = channel + 1;

	int data = 0;
	int sent_bytes = 0;
	
	ready[get_cpuid()] = 1;

	while(!ready[0]){led_off();}
	
	led_on();

	int start = get_cpu_cycles();
	
	while(1) {
		if(channel->transmission == ACKNOWLEDGED) {
			if(DATA_TO_SEND - sent_bytes >= channel->capacity) {
				channel->size = channel->capacity;
				sent_bytes += channel->capacity;
			} else {
				channel->size = DATA_TO_SEND - sent_bytes;
				sent_bytes += DATA_TO_SEND - sent_bytes;
			}
			
			for(int i = 0; i < channel->capacity; i++){
				buffer[i] = 1;
			}
			
			if(sent_bytes < DATA_TO_SEND) {
				channel->transmission = ONGOING;
				continue;
			} else {
				channel->transmission = ENDED;
				break;
			}
		}
	}
	
	int end = get_cpu_cycles();
	led_off();
	intervals[get_cpuid()] = end - start;
}

void receiver_slave(void* args){
	ready[get_cpuid()] = 1;
	while(!ready[0]){led_off();}
	led_on();
	
	qpd_t * chan = mp_create_qport(get_cpuid(), SINK, CHANNEL_BUFFER_CAPACITY, 1);
	mp_init_ports();
	
	int start = get_cpu_cycles();
	mp_recv(chan, 0);
	rnd_nr_rd = *(( volatile int _SPM * ) ( chan2->read_buf ));
	mp_ack(chan, 0);
	
	int end = get_cpu_cycles();
	led_off();
	intervals[get_cpuid()] = end - start;
}

int main(){
	led_off();

	qpd_t *channel[2*SENDER_RECEIVER_PAIRS];

	for(int i = 0; i < SENDER_RECEIVER_PAIRS; i++){
		printf("Buffer size: %x.\n", size);
	}

	for(int i = 0; i< (1+(2*SENDER_RECEIVER_PAIRS)); i++){
		ready[i] = 0;
	}

	for(int i = 1; i <= SENDER_RECEIVER_PAIRS; i++){
		int core = i*2;
		corethread_create(&core, &sender_slave, (void*)channel[i-1]);
		channel[i] = 
		core--;
		corethread_create(&core, &receiver_slave, (void*)channel[i-1]);
		channel[i] = mp_create_qport(core, SOURCE, CHANNEL_BUFFER_CAPACITY, 1);
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
	
	intervals[0] = 0;
	for(int i = 0; i <= 2*SENDER_RECEIVER_PAIRS; i++){
		printf("Cycles spent (%d): %d.\n", i, intervals[i]);
	}
	
	return 0;
}
























