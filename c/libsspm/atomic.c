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
	
	asm volatile(	"lwl $r1 = [$r1];" 	// Sync to TDMA
					"lwl %0 = [$r2];"	// Load lock value
					"swl [$r2] = $r3;"	// write lock = LOCKED
                 	: "=r" (lock_value)
                 	: "{$r1}" (syncAddr), "{$r2}" (l), "{$r3}" (LOCKED)
                 	: "$r0", "$r1", "$r2", "$r3"
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
