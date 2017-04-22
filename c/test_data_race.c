#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.c"
#include "libsspm/smp.h"

struct PACKET_LINK_T{
	struct PACKET_T packet;
	struct PACKET_LINK_T *next;
};

void slave(void* arg){

}

int main()
{
	int i;	
	for(i = 1; i< NR_CORES; i++){
		corethread_create(&i, &slave, NULL);
	}
	
	const int NR_SLAVES = NR_CORES-1;
	const int STOP_VALUE = 5;

	int lastSent[NR_SLAVES];
	int lastRecieved[NR_SLAVES];

	for(i = 0; i<NR_SLAVES; i++){
		lastSent[i] = -1;
		lastRecieved[i] = -1;
	}
	
	int next_slave = 1;
	int highest_sent = -1;
	int highest_received = -1;

	while(highest_received < STOP_VALUE){
		
		//Send a value to the next slave in line

		//Receive a value from input

	}

	//Find all remaining packets that were not caught by the previous while-loop
	struct PACKET_LINK_T *head = NULL;
	struct PACKET_LINK_T *tail = NULL;
	struct PACKET_OPTION current;

	while((current = receive(0)).valid){
		//Allocate stack memory for new packet
		char allocate[sizeof(struct PACKET_LINK_T)];
		
		if(head == NULL){
			head = ((struct PACKET_LINK_T *)allocate);
		}
		//Make tail point to allocated memory
		tail->next = ((struct PACKET_LINK_T *)allocate);

		//Set packet contentsin allocated memory
		(tail->next)->packet = current.packet;
		(tail->next)->next = NULL;

		//Assign allocated memory as new tail.
		tail = tail->next;
	}

	//Go through all the packets and see if they can be fitted somewhere

}


