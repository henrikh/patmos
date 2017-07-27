
#ifndef _ATOMIC_H_
#define _ATOMIC_H_

struct SSPM_MP_DESC{
	// Points to the first address occupied by the message passing channel in
	// the SSPM.
	volatile _SPM int *channel;

	// The size of the buffer
	int buffer_size;
}

// Returns whether a transmission is in progress on the channel.
int sspm_mp_get_progress(struct SSPM_MP_DESC channel);

// Returns whether the channel contains data to be received.
int sspm_mp_get_free(struct SSPM_MP_DESC channel);

// Initialize a new message passing channel 
struct SSPM_MP_DESC sspm_mp_init(volatile _SPM int *channel_start, int buffer_size);


#endif
