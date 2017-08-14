#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "libcorethread/corethread.c"
#include "libsspm/led.h"
#include "libmp/mp.h"
#include "libmp/mp_internal.h"

#define MP_CHAN_NUM_BUF 2
#define MP_CHAN_BUF_SIZE 40

const int NOC_MASTER = 0;

const int SENDER_RECEIVER_PAIRS = 1;

const int CHANNEL_BUFFER_CAPACITY = 1;

volatile _UNCACHED int ready[1+(2*SENDER_RECEIVER_PAIRS)];

volatile _UNCACHED int rwtime[1+(2*SENDER_RECEIVER_PAIRS)];

volatile _UNCACHED int intervals[1+(2*SENDER_RECEIVER_PAIRS)];

void sender_slave(void* args){
	int cpuid = get_cpuid();
		
	qpd_t * chan = mp_create_qport(1, SOURCE, CHANNEL_BUFFER_CAPACITY*sizeof(int), MP_CHAN_NUM_BUF);
	mp_init_ports();
	ready[get_cpuid()] = 1;
	while(!ready[0]){led_off();}
	led_on();

	unsigned long long start;
	unsigned long long end;
	unsigned long long sent;
	unsigned long long received;

	if(*(chan->send_recv_count) == 0){

		start = get_cpu_cycles();

		for(int i = 0; i<CHANNEL_BUFFER_CAPACITY; i++){
			(( volatile int _SPM * ) ( chan->write_buf ))[i] = -1;
		}
;
		end = get_cpu_cycles();
	}	
	
	sent = get_cpu_cycles();
	
	mp_nbsend(chan);
	while(*(chan->send_recv_count) != 1){}

	received = get_cpu_cycles();

	led_off();
	rwtime[get_cpuid()] = (int) end - start;
	intervals[get_cpuid()] = (int) received - sent;
}

void receiver_slave(void* args){
	int cpuid = get_cpuid();

	qpd_t * chan = mp_create_qport(1, SINK, CHANNEL_BUFFER_CAPACITY*sizeof(int), MP_CHAN_NUM_BUF);
	mp_init_ports();
	
	int data[CHANNEL_BUFFER_CAPACITY];
	
	for(int i = 0; i<CHANNEL_BUFFER_CAPACITY; i++){
		data[i] = 0;
	}

	ready[get_cpuid()] = 1;
	while(!ready[0]){led_off();}
	led_on();

	unsigned long long start;
	unsigned long long end;
	unsigned long long sent;
	unsigned long long received;

	
	mp_recv(chan,0);

	start = get_cpu_cycles();
	asm volatile ("" : : : "memory");

	for(int i = 0; i< CHANNEL_BUFFER_CAPACITY; i++){
		data[i] = (( volatile int _SPM * ) ( chan->read_buf ))[i];
	}
	
	asm volatile ("" : : : "memory");
	end = get_cpu_cycles();


	asm volatile ("" : : : "memory");
	mp_ack(chan,0);
	asm volatile ("" : : : "memory");

	received = get_cpu_cycles();


	led_off();

	rwtime[get_cpuid()] = (int) end - start;
	
	intervals[get_cpuid()] = received - end;
	
}

int main(){	;
	
	led_off();
	
	printf("Starting\n");

	for(int i = 0; i< (1+(2*SENDER_RECEIVER_PAIRS)); i++){
		ready[i] = 0;
	}
	
	for(int i = 1; i <= SENDER_RECEIVER_PAIRS; i++){
		int core = i*2;
		corethread_create(&core, &sender_slave, NULL);
		core--;
		corethread_create(&core, &receiver_slave, NULL);
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
		printf("Receiver extract\t\t(%d): %d.\n", i, rwtime[i]);
		printf("Sender deposit\t\t\t(%d): %d.\n", i+1, rwtime[i+1]);
	}
	
	for(int i = 1; i <= 2*SENDER_RECEIVER_PAIRS; i+=2){
		printf("Receiver ack\t\t\t(%d): %d.\n", i, intervals[i]);
		printf("Sender send and wait for ack\t(%d): %d.\n", i+1, intervals[i+1]);
	}
	
	return 0;
}









