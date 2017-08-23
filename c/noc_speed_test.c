#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "libcorethread/corethread.c"
#include "libsspm/led.h"
#include "libmp/mp.h"
#include "libmp/mp_internal.h"
#include "libsspm/sspm_benchmark.h"

#define MP_CHAN_NUM_BUF 2
#define MP_CHAN_BUF_SIZE 40

const int NOC_MASTER = 0;

const int TIMES_TO_SEND = 1000;
volatile _UNCACHED int send_clock[TIMES_TO_SEND];
volatile _UNCACHED int recv_clock[TIMES_TO_SEND];
volatile _UNCACHED int ack_recv_clock[TIMES_TO_SEND];


int sender_slave(void* args){
	int cpuid = get_cpuid();
	qpd_t * chan = mp_create_qport(1, SOURCE, CHANNEL_BUFFER_CAPACITY*sizeof(int),MP_CHAN_NUM_BUF);

	mp_init_ports();

	led_on();

	for(int k = 0; k< TIMES_TO_SEND; k++){
		
		send_clock[k] = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		mp_send(chan,0);
		while(*(chan->send_recv_count) != (k+1)){}
		asm volatile ("" : : : "memory");
		ack_recv_clock[k] = get_cpu_cycles();
		
	}
	
	led_off();
	return 0;
}

int receiver_slave(void* args){
	int cpuid = get_cpuid();
	qpd_t * chan = mp_create_qport(1, SINK, CHANNEL_BUFFER_CAPACITY*sizeof(int), MP_CHAN_NUM_BUF);
	
	mp_init_ports();

	led_on();
	
	for(int k = 0; k<TIMES_TO_SEND; k++){
		mp_recv(chan,0);
		asm volatile ("" : : : "memory");
		recv_clock[k] = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		mp_ack(chan,0);
	}
	
	return 0;
}


int main(){	;
	
	led_off();
	
	printf("Starting\n");
	led_on();

	int core1 = 1;
	int core2 = 2;
	corethread_create(&core1, &sender_slave, NULL);
	corethread_create(&core2, &receiver_slave, NULL);
		
	int res;
	corethread_join(core1, (void **) &res);
	corethread_join(core2, (void **) &res);

	printf("Send clocks:\n");
	for(int i = 1; i < TIMES_TO_SEND; i++){
		printf("%d\n", (recv_clock[i] - send_clock[i]));
	}
	
	printf("Ack clocks:\n");
	for(int i = 1; i < TIMES_TO_SEND; i++){
		printf("%d\n", (ack_recv_clock[i] - recv_clock[i]));
	}

	return 0;
}








