
#include "atomic.h"



int try_lock( volatile _SPM lock_t *l){
	
	long long syncAddr = SCHEDULE_SYNC;
	int s0;
	int intWasOn;
	lock_t lock_value;
	
	//Get register s0 value
	asm volatile(
					"mfs %0 = $s0;"
					: "=r" (s0)
					: 
					:
	);
	
	//If interrupts are enabled, disable them
	intWasOn = s0 % 2;
	if(intWasOn){
		s0++;
		asm volatile(
					"mts $s0 = %0;"
					: 
					: "r" (s0)
					: "$s0"
		);
	}

	asm volatile(	"lwl %0 = [%1 + 0];" 	// Sync to TDMA
					"lwl %0 = [%2 + 0];"	// Load lock value
					"swl [%2 + 0] = %3;"	// write lock = LOCKED
                 	: "=r" (lock_value)
                 	: "r" (syncAddr), "r" (l), "r" (LOCKED)
                 	: 
	);
	
	//If interrupts were enabled before then enable them
	if(intWasOn){
		s0--;
		asm volatile(
					"mts $s0 = %0;"
					: 
					: "r" (s0)
					: "$s0"
		);
	}

	//Return to the caller whether he now has the lock
	if(lock_value == OPEN){
		printf("MAIN:got lock\n");
	}else{
		printf("MAIN:no lock\n");
	}
	return  lock_value == OPEN ;
}

void lock(volatile _SPM lock_t *l){
	while( !try_lock(l)){}
}

void release( volatile _SPM lock_t *l ){
	*l = OPEN;
}
