

#include "mp.h"
#include "led.h"

int mask(int capacity, int index){
	return index & (capacity-1);
}

int sspm_mp_occupies(volatile _SPM struct SSPM_MP_DESC *channel){
	return sizeof(struct SSPM_MP_DESC) + (sizeof(int) * (channel->capacity));
}

int ssmp_mp_size(volatile _SPM struct SSPM_MP_DESC *channel){
	return channel->tail - channel->head;
}

int sspm_mp_free(volatile _SPM struct SSPM_MP_DESC *channel){
	return channel->capacity - ssmp_mp_size(channel);
}

int sspm_mp_full(volatile _SPM struct SSPM_MP_DESC *channel){
	return !sspm_mp_free(channel);
}

int sspm_mp_empty(volatile _SPM struct SSPM_MP_DESC *channel){
	return sspm_mp_free(channel) == channel->capacity;
}

volatile _SPM void* 
sspm_mp_init(
	volatile _SPM struct SSPM_MP_DESC *channel_start, 
	int x
){
	channel_start->head = 0;
	channel_start->tail = 0;
	channel_start->capacity = 2 << (x-1);
	channel_start->transmission = ACKNOWLEDGED;

	return ((int)channel_start) + sspm_mp_occupies(channel_start);
}

int sspm_mp_try_send(
	volatile _SPM struct SSPM_MP_DESC *channel, 
	int *send_from, int offset, int amount)
{
	int free_space = sspm_mp_free(channel);

	if(free_space){
		volatile _SPM int *buffer = channel + 1;

		int i;
		for(i = 0; (i<free_space) && (i<amount); i++){
			int masked = mask(channel->capacity, channel->tail);
			int index = offset+i;
			buffer[masked] = send_from[index];
			channel->tail++;
		}
		return i;
	}else{
		return 0;
	}
}

int sspm_mp_try_receive(
	volatile _SPM struct SSPM_MP_DESC *channel,
	int *receive_into, int offset, int up_to)
{
	int size = ssmp_mp_size(channel);

	if(size){
		volatile _SPM int *buffer = channel + 1;
		int i;
		for(i = 0; i<size && i<up_to; i++){
			receive_into[offset + i] = buffer[mask(channel->capacity, channel->head)];
			channel->head++;
		}
		return i;
	}else{
		return 0;
	}
}

int sspm_mp_transmission_try_send(
	volatile _SPM struct SSPM_MP_DESC *channel,
	int *send_from, int offset, int up_to)
{
	//led_on_for(1);	
	if(channel->transmission == ENDED){
		//led_off_for(1);
		return 0;
	}else{
		//If it is a new transmission, begin it
		if(channel->transmission == ACKNOWLEDGED){
			channel->transmission = ONGOING;
		}// Else channel is already in ONGOING, then continue transmission
		//led_off_for(1);
		return sspm_mp_try_send(channel, send_from, offset, up_to);
	}
	
}

void sspm_mp_transmission_end(
	volatile _SPM struct SSPM_MP_DESC *channel)
{
	channel->transmission = ENDED;
}

int sspm_mp_transmission_try_receive(
	volatile _SPM struct SSPM_MP_DESC *channel,
	int *receive_into, int offset, int up_to, int *transmission_end)
{	
	//led_on_for(1);	
	if(channel->transmission == ACKNOWLEDGED){
		*transmission_end = 0;
		//led_off_for(1);		
		return 0;
	}else{
		int received = sspm_mp_try_receive(channel, receive_into, offset, up_to);
		
		if(	(channel->transmission == ENDED) &&
			(sspm_mp_empty(channel))
		){
			channel->transmission = ACKNOWLEDGED;
			*transmission_end = 1;
		}else{
			*transmission_end = 0;
		}
		//led_off_for(1);
		return received;
	}
}
















