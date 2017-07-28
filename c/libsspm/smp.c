
#include "smp.h"
#include "atomic.h"
#include <stdio.h>
#include <machine/spm.h>

const int total_channel_footprint = TOTAL_SHARED_MEMORY / NR_CORES;

const int channel_capacity = (total_channel_footprint - sizeof(struct CHANNEL_T)) / sizeof(struct PACKET_T);

void printsomething(){
	printf("Confirmed correct linking.\n");
}

void initialize_all_channels(){
	
	//printf("Enter 'initialize_all_channels'.\n");
	//printf("Total channel footprint: %x.\n", total_channel_footprint);
	//printf("sizeof(CHANNEL_T) = %x.\n", sizeof(struct CHANNEL_T));
	//printf("sizeof(PACKET_T) = %x.\n", sizeof(struct PACKET_T));
	
	int i;
	
	volatile _SPM struct CHANNEL_T *chan;

	//Lock all channels as soon as possible to minimize 
	//The time it is possible for other cores to access the channels
	for(i = 0; i< NR_CORES; i++){
		//Calculate the address of the channel header
		chan = channel_of(i);
		//lock channel
		//printf("Channel %x address: %x.\n", i, chan);
		//printf("Channel %x lock address: %x.\n", i, &(chan->lock));
		chan->lock = LOCKED;
		//printf("Channel %x locked: %x.\n", i, chan->lock);
	}

	//Reset channel headers and release the channels
	for(i = 0; i< NR_CORES; i++){
		chan = channel_of(i);
		chan->size = 0;
		chan->head = 0;
		release( &(chan->lock) );
		//printf("Channel %x reset.\n", i);
	}

	/*printf("Initialising channels:\n");
	for(i= 0; i<NR_CORES; i++){
		chan = channel_of(i);
		printf("Channel %x: lock=%x, size=%x, head=%x.\n", i, chan->lock, chan->size, chan->head);
	}*/
}

volatile _SPM struct CHANNEL_T* channel_of(int coreNr){
	volatile _SPM struct CHANNEL_T* address = LOWEST_SSPM_ADDRESS + 
												(coreNr * total_channel_footprint);
	return address;
}

volatile _SPM struct PACKET_T* packet_at_index(coreid_t coreNr, int packet_index){
	//printf("packet_at_index(coreNr=%x, packet_index=%x).\n", coreNr, packet_index);

	int channel_addr = (int) channel_of(coreNr);
	//printf("channel_addr=%x.\n", channel_addr);
	
	int channel_size = sizeof(struct CHANNEL_T);
	//printf("channel_size = %x.\n", channel_size);
	
	int address_of_first_packet =
			 channel_addr + channel_size;
	//printf("address_of_first_packet = %x.\n", address_of_first_packet);

	int corrected_packet_index = packet_index % channel_capacity;
	//printf("corrected_packet_index = %x.\n", corrected_packet_index);
	
	int offset_to_indexed_packet = corrected_packet_index * sizeof(struct PACKET_T);
	//printf("offset_to_indexed_packet = %x.\n", offset_to_indexed_packet);
	
	volatile _SPM struct PACKET_T* address = 
			address_of_first_packet + offset_to_indexed_packet;

	//printf("address = %x.\n", address);

	return address;
}

int send( coreid_t receiver, struct PACKET_T packet){
	//printf("Enter sender(%x, {payload=%x, sender=%x}).\n", receiver, packet.payload, packet.sender);
	int result;

	//get channel header
	volatile _SPM struct CHANNEL_T *chan = channel_of(receiver);
	
	//printf("Channel %x is receiver, address = %x.\n", receiver, chan);
	
	//printf("Locking channel %x.\n", receiver);
	//Lock channel
	lock(&(chan->lock));
	
	//printf("Channel %x lock = %x.\n", receiver, chan->lock);

	if( chan->size == channel_capacity ){
		//If the channel is full do nothing and return 0
		result = 0;
		//printf("Channel %x is full, size = %x.\n", receiver, chan->size);
	}else{
		//Find the address to put packet
		volatile _SPM struct PACKET_T *nextP = 
						packet_at_index(receiver, (chan->head + chan->size));
		//printf("Address of packet in channel = %x\n", nextP);

		//insert the packet
		nextP->payload = packet.payload;
		nextP->sender = packet.sender;
		
		//printf("Inserted into packet: payload = %x, sender = %x.\n", nextP->payload, nextP->sender);		
		volatile _SPM struct PACKET_T *p2 = 0xF004000c;
		//printf("-----------------payload=%x, sender=%x.\n", p2->payload, p2->sender);
		//Update the channel header
		chan->size = chan->size + 1;
		volatile _SPM struct PACKET_T *p1 = 0xF004000c;
		//printf("-----------------payload=%x, sender=%x.\n", p1->payload, p1->sender);

		//printf("Address of chan->size = %x.\n", &(chan->size));
		//printf("Address of nextP->sender = %x.\n", &(nextP->sender));

		//printf("Updated channel %x to {size=%x, head=%x}.\n", receiver, chan->size, chan->head);
		
		//return 1
		result = 1;
	}
	
	//Release the channel
	release(&(chan->lock));
	
	//printf("channel %x lock released, lock = %x.\n", receiver, chan->lock);
	
	//printf("Exit sender() = %x.\n", result);
	return result;
}

struct PACKET_OPTION receive(coreid_t receiver){
	printf("Enter receive(%x).\n", receiver);
	
	struct PACKET_OPTION p;	

	//get channel header
	volatile _SPM struct CHANNEL_T *chan = channel_of(receiver);	
	//printf("Channel %x address=%x.\n", receiver, chan);	
	//printf("Locking channel %x.\n", receiver);
	//Lock channel
	lock(&(chan->lock));
	//printf("Channel %x lock=%x.\n", receiver, chan->lock);
	
	
	if(chan->size == 0){
		//Channel is empty, return an invalid packet
		p.valid = 0;
		//printf("Channel %x is empty, size=%x.\n", receiver, chan->size);
	}else{
		//Find the address of the packet to extract
		volatile _SPM struct PACKET_T *packet_address = 
			packet_at_index(receiver, chan->head);
		
		//printf("Address of packet = %x.\n", packet_address);		
		//printf("Contents of packet {payload=%x, sender=%x}.\n", packet_address->payload, packet_address->sender);
		
		//Extract the packet
		p.packet.payload = packet_address->payload;
		p.packet.sender = packet_address->sender;
		p.valid = 1;

		//printf("Extracted packet {payload=%x, sender=%x}.\n", p.packet.payload, p.packet.sender);

		//update channel header
		(chan->size)--;
		chan->head = (chan->head + 1) % channel_capacity;
		//printf("Updated channel %x to {size=%x, head=%x}.\n", receiver, chan->size, chan->head);
	}

	//Release the channel
	release(&(chan->lock));
	
	printf("Exit receive() = {packet={payload=%x, sender=%x}, valid=%x}.\n", p.packet.payload, p.packet.sender, p.valid);
	return p;
}


