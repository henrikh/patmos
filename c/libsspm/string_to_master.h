#ifndef _STRING_TO_MASTER_H_
#define _STRING_TO_MASTER_H_

#include <machine/spm.h>
#include "sspm_properties.h"

#define CHANNEL_BUFFER_SIZE (25)


/*
Initializes the communication channels starting at the given memory
address (in the SSPM). 

Returns the first unused memory address after the communication channels.
*/
int initialize_messaging(int initAt){
	
}

/*
Checks the communication channel to see if there is anything to receive
*/
int can_receive_from_core(int coreNr){

}

/*

*/
char[] receive_from_core(int coreNr){

}

/*
Allows the communication channel to discard anything recieved from the core,
readying the channel for the next transmision.
*/
void free_received_from_core(int coreNr){

}

/*
Checks the communication channel of the core to see if it is ready to send.
*/
int can_send_from(int coreNr){

}

/*
Send the given text from the core.
*/
void send_from_core(int coreNr, char[] text){
	
}


#endif
