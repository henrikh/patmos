
#ifndef _COPRINT_H_
#define _COPRINT_H_

#include <machine/spm.h>
#include "sspm_properties.h"

#define CHANNEL_BUFFER_SIZE (25)


/*
Initializes the communication channels for the cooperative printing
starting at the given memory address (in the SSPM). 

Returns the first unused memory address after the communication channels.
*/
int coprint_initialize(int initAt);

/*
Checks the communication channel to see if there is anything to receive
from the given core.
*/
int coprint_can_receive(int coreNr);

/*
Loads the contents
*/
char* coprint_receive(int coreNr);

/*
Receive from given core.
Busy waits until there is something in the channel.
*/
char* coprint_wait_receive(int coreNr);

/*
Allows the communication channel to discard anything recieved from the core,
readying the channel for the next transmision.
*/
void coprint_free_received(int coreNr);

/*
Checks the communication channel of the core to see if it is ready to send.
*/
int coprint_can_send(int coreNr);

/*
Send the given text from the core.
*/
void coprint_send(int coreNr, char *text);

/*
Send the given text when the channel is ready.
Busy waits until the channel is ready.
*/
void coprint_wait_send(int coreNr, char *text);

/*
Get the address of the core's channel flag
*/
int coprint_flag_address(int coreNr);

/*
Get the address of the first address in the cores buffer in the SSPM
*/
int coprint_buffer_start(int coreNr);

/*
Get the address of the local channel buffer.
*/
char* coprint_local_buffer(int coreNr);



#endif
