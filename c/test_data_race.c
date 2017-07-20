#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.c"
#include "libsspm/smp.h"

#define FLAG_ADDRESS 	(LOWEST_SPM_ADDRESS)
#define LOCK_ADRESS 	(LOWEST_SPM_ADDRESS + 4)
#define CONTEST_ADRESS 	(LOWEST_SPM_ADDRESS + 8)

volatile _SPM int const *flag = FLAG_ADDRESS;
volatile _SPM int const *lock = LOCK_ADDRESS;
volatile _SPM int const *contest = CONTEST_ADDRESS;

struct PACKET_LINK_T{
	struct PACKET_T packet;
	struct PACKET_LINK_T *next;
};

void slave(void* arg){

}

int main()
{
	//Ensure flag is 0	
	*flag = 0;	

	int i;	
	for(i = 1; i< NR_CORES; i++){
		corethread_create(&i, &slave, NULL);
	}
	
	const int NR_SLAVES = NR_CORES-1;
	
	//Flag the cores to start
	*flag = 1;

}


