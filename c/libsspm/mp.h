
#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#include <machine/spm.h>


/*
Defines a circular buffer implemented as seen in 'Array + two unmasked indices' in page:
https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/

*/

typedef enum {
	// Transmission is still in progress	
	ONGOING, 
	// Transmission has been finished by the sender.
	// Another transmission cannot begin before
	// the receiver acknowledges it.
	ENDED, 
	// Transmission end has been acknowledged by the receiver.
	// Should also be the initial state of the channel after initialization.
	ACKNOWLEDGED
} TRANSMISSION_STATE;

struct SSPM_MP_DESC{
	//Index of the head element
	unsigned int head;
	//index of the tail element
	unsigned int tail;
	//The number of elements in the buffer
	unsigned int capacity;
	//Whether a transmission is in progress
	TRANSMISSION_STATE transmission;               
};

// Returns the total amount of memory occupied by the channel.
// This includes the memory taken by this header and the associated
// buffer. Both of which, together, comprise the channel.
int sspm_mp_occupies(volatile _SPM struct SSPM_MP_DESC *channel);

// Returns the amount of data that is currently in the channel, 
// ready to be received by the receiver.
int ssmp_mp_size(volatile _SPM struct SSPM_MP_DESC *channel);

// Returns the number of free element slots in the channel.
// Guarantees that calling 'sspm_mp_try_send' can send the same
// amount of elements.
int sspm_mp_free(volatile _SPM struct SSPM_MP_DESC *channel);

// Returns whether the channel is full.
// Guarantees that calling 'sspm_mp_try_send' will result in no elements
// sent, while calling 'sspm_mp_try_receive' will be able to receive the
// same number of elements as the capacity of the channel.
int sspm_mp_full(volatile _SPM struct SSPM_MP_DESC *channel);

// Returns whether the channel is empty.
// Guarantees that if the channel is empty 'sspm_mp_try_send' will
// be able to send the same number of elements as the capacity of the channel,
// while 'sspm_mp_try_receive' will not receive any element.
// If the sender has closed a transmission, he can deduce that when the channel is
// empty the receiver has received the whole transmission
int sspm_mp_empty(volatile _SPM struct SSPM_MP_DESC *channel);

// Initialize a new message passing channel with a buffer capacity
// of 2 to the power of 'x' elements.
// Returns the first unoccupied memory location in the SSPM after the channel.
volatile _SPM void* 
sspm_mp_init(volatile _SPM struct SSPM_MP_DESC *channel_start, int x);

//Tries to send the given amount of elements from the given array with the given offset.
//Returns the amount of elements sent. 
//Should be used with care if transmissions are used.
int sspm_mp_try_send(
	volatile _SPM struct SSPM_MP_DESC *channel, 
	int *send_from, int offset, int amount);

//Tries to receive the given amount of elements into the given array at the given offset.
//Returns the amount of elements sent.
//Should be used with care if transmission are used.
int sspm_mp_try_receive(
	volatile _SPM struct SSPM_MP_DESC *channel,
	int *receive_into, int offset, int up_to);

// Returns the transmission state of the channel.
TRANSMISSION_STATE sspm_mp_transmission(volatile _SPM struct SSPM_MP_DESC *channel);

// Attempts to send the given amount of elements from the given array starting
// at the given offset.
// Can be used to either start or continue a transmission but will fail if the transmission
// has been ended but the receiver has not acknowledged the end.
// Or more formally:
// If the channel's transmission state is 'ONGOING' then the state stays the same.
// If the channel's transmission state is 'ENDED' then nothing will be sent.
// If the channel's transmission state is 'ACKNOWLEDGED' then the state will first
// be chaned to 'ONGOING' and then some data is sent.
// Returns the amount of elements sent.
int sspm_mp_transmission_try_send(
	volatile _SPM struct SSPM_MP_DESC *channel,
	int *send_from, int offset, int up_to);

// Ends a transmission setting the transmission state of the channel to 'ENDED'.
// Should not be called if the transmission state is not 'ONGOING'.
void sspm_mp_transmission_end(
	volatile _SPM struct SSPM_MP_DESC *channel);

// Attempts to receive the given amount of elements from the given array starting
// at the given offset.
// Will have not receive any elements if a transmission is 'ACKNOWLEDGED'.
// Or more formally:
// If the channel's transmission state is 'ONGOING' may receive some data.
// If the channel's transmission state is 'ENDED' may receive some data and 
// if the channel is empty afterwards, will change the state to 'ACKNOWLEDGED' and set
// the value pointed to by 'transmission_end' to 1.
// If the channel's transmission state is 'ACKNOWLEDGED' will not receive any elements.
// Returns the number of elements received. Additionally, will set the value of 'transmission_end'
// to 1 if a transmission end was acknowledged, otherwise will be set to 0.
int sspm_mp_transmission_try_receive(
	volatile _SPM struct SSPM_MP_DESC *channel,
	int *receive_into, int offset, int up_to, int *transmission_end);















#endif






















































