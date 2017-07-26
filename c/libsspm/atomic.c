#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "atomic.h"
#include "led.h"


int try_lock( volatile _SPM lock_t *l){
	
	int syncAddr = SCHEDULE_SYNC;
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
	
	asm volatile(	"lwl $r0 = [%[sync]];" 	// Sync to TDMA
				"lwl %[lock_value] = [%[lock]];"	// Load lock value
				"swl [%[lock]] = %[LOCKED];"	// write lock = LOCKED
				: [lock_value] "=r" (lock_value), [lock] "+rm" (l)
				: [sync] "r" (syncAddr), [LOCKED] "r" (LOCKED)
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

	return  lock_value == OPEN ;
}

void lock(volatile _SPM lock_t *l){
	while( !try_lock(l)){}
}

void release( volatile _SPM lock_t *l ){
	*l = OPEN;
}
