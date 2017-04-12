/*

Message massing framework for message passing between core using a shared scratchpad memory
that utilises time division multiplexing.

Message passing is achieved using dedicated input channels for each core that are accessable by all cores.
The channel is a FIFO queue that does not take the sender into account.
So if core 1 wants to send to core 2, core 1 sends to core 2's input channel. If core 3 wants to send to core 2, it does the exact same as core 1.

There is no dedicated channels between cores, and all channels are active at all times.

Currently, the channels do not support blocking. There is no way for the sender to know whether the
receiving core has gotten the message except for going through its channel to look for the message.

Atomicity is assumed managed by the TDMA.

*/

#ifndef _SMP_H_
#define _SMP_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/boot.h>
#include <machine/rtc.h>
#include "atomic.h"

#define NR_CORES 3
#define TOTAL_SHARED_MEMORY 0xFFFF 		//65535 decimal bytes
#define LOWEST_SPM_ADDRESS (0xF0040000)	//The lowest address mapped to the SPM

/// \brief A type to identify a core.
typedef int coreid_t;

/*
How much memory each core's channel takes up in total
*/
extern const int total_channel_footprint;

/*
Header for each channel, used for managing the queue.
Any change in the channel should be done while having the lock on its header.
*/
struct CHANNEL_T{
	//The current amount of packets in the channel
	int size;
	//The index of the next packet in the queue.
	int head;
	lock_t lock;
};

/*
Struct for the packetss to be sent to the channel. 
Contains the payload from the sender and the core number of the same.
*/
struct PACKET_T{
	int payload;
	coreid_t sender;
};

/*
The amount of packetss each channel can have in its queue
*/
extern const int channel_capacity;


/*
Initializes all channels. Must be called exactly once at the start of the program before any channel is used.
Should be called by the main() function on the master core before any other core is started.
*/
void initialize_all_channels();

/*
Calculates the address of the header for the  channel of the given core.
*/
volatile _SPM struct CHANNEL_T* channel_of(int coreNr);

/*
Calculates the address of the packet at the given index of the channel
belonging to the given core.
If the packet_index is larger than the capacity it is modulo'd with
the capacity before the calculation. Therefore, the returned
address is guaranteed to be a valid address of a packet.
*/
volatile _SPM struct PACKET_T* packet_at_index(coreid_t coreNr, int packet_index);

/*
Sends a packet to the specified core.
If the channel does not have enough capacity to buffer the packet,
the packet is not sent and 0 is returned.
otherwise 1 is returned.
*/
int send( coreid_t receiver, struct PACKET_T packet);

/*
Used to show whether the CHANNEL_PACKET_T is a valid one.
If the valid value is 0, then the packet is not valid,
otherwise it is.
*/
struct PACKET_OPTION{
	struct PACKET_T packet;
	char valid;
};

/*
Extracts the first packet in the channel of the specified core.
If the channel does not have a packet the returned .valid  == 0
and the packet is invalid
*/
struct PACKET_OPTION receive(coreid_t receiver);


/*
just for testing linking correctly.
*/
void printsomething();

























#endif
