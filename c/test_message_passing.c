#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.c"
#include "libsspm/led.h"
#include "libsspm/sspm_properties.h"
#include "libsspm/mp.h"


#define assert_eq(expected,got){if((expected) != (got)){printf("%d: Error expected %x but got %x\n", __LINE__, (expected),(got)); return 1;}}


int simple_test(){
	volatile _SPM struct SSPM_MP_DESC *channel = LOWEST_SSPM_ADDRESS;

	//Test that the correct address is returned.
	int	init_return = sspm_mp_init(channel, 2);
	int init_return_expected = (LOWEST_SSPM_ADDRESS+(sizeof(struct SSPM_MP_DESC))+(4*4));
	assert_eq(init_return_expected, init_return);
	
	//Test that the channel occupies the correct amount of memory
	assert_eq((sizeof(struct SSPM_MP_DESC))+(4*4), sspm_mp_occupies(channel));

	//Test that the channel has all it's buffer free
	int free = sspm_mp_free(channel);
	int free_expected = 4;
	assert_eq(free_expected, free);

	//Test that there is nothing to receive
	int to_receive[4] = {-1,-1,-1,-1};
	int received = sspm_mp_try_receive(channel, to_receive, 0, 4);
	int received_expected = 0;
	assert_eq(received_expected, received);

	//Test that nothing was entered in the array
	for(int expected = 0; expected<4; expected++){
		assert_eq(-1, to_receive[expected]);
	}
	int to_send[4] = {0,1,2,3};
	
	//Test that can send the same amount as the buffer capacity
	int sent = sspm_mp_try_send(channel, to_send,0,4);
	int sent_expected = 4;
	assert_eq(sent_expected,sent);

	//Test that the channel is then full
	free = sspm_mp_free(channel);
	free_expected = 0;
	assert_eq(free_expected,free);
	
	//Test that can receive everything in the buffer
	received = sspm_mp_try_receive(channel, to_receive, 0, 4);
	received_expected = 4;
	assert_eq(received_expected, received);
	
	//Test that the received values are correct
	for(int expected = 0; expected<4; expected++){
		assert_eq(expected, to_receive[expected]);
	}

	//Test that the channel is now empty
	free = sspm_mp_free(channel);
	free_expected = 4;
	assert_eq(free_expected, free);
	
	//Test that cannot receive anything
	for(int i = 0; i<4; i++){
		to_receive[i] = -1;
	}
	received = sspm_mp_try_receive(channel, to_receive, 0, 4);
	received_expected = 0;
	assert_eq(received_expected, received);
	
	//Test that nothing was changed in the array
	for(int expected = 0; expected<4; expected++){
		assert_eq(-1, to_receive[expected]);
	}
	
	//Test can send less that the max capacity
	sent = sspm_mp_try_send(channel, to_send,1,2);
	sent_expected = 2;
	assert_eq(sent_expected,sent);
	
	//Test that there are still some space left in the buffer
	free = sspm_mp_free(channel);
	free_expected = 2;
	assert_eq(free_expected, free);
	
	//Test can receive only what was sent
	received = sspm_mp_try_receive(channel, to_receive, 0, 4);
	received_expected = 2;
	assert_eq(received_expected, received);
	
	//Test the array was only changed with the received
	for(int expected = 0; expected<2; expected++){
		assert_eq((expected+1), to_receive[expected]);
	}
	for(int i = 2; i<4; i++){
		assert_eq(-1, to_receive[i]);
	}
	printf("simple_test done.\n");
	return 0;
}

const int GOAL = 64;
void slave1(void* args){
	
	volatile _SPM struct SSPM_MP_DESC *channel = (volatile _SPM struct SSPM_MP_DESC *)args;
	
	
	int progress = 0;
	
	while(progress < GOAL){
		if(sspm_mp_try_send(channel, &progress, 0,1)){
			progress++;
		}
	}
}

int simple_multicore_test(){
	
	volatile _SPM struct SSPM_MP_DESC *channel = LOWEST_SSPM_ADDRESS+4;
	
	
	//Test that the correct address is returned.
	int channel_end = sspm_mp_init(channel,5);
	int channel_end_expected = (LOWEST_SSPM_ADDRESS+4+(sizeof(struct SSPM_MP_DESC))+(4*32));
	assert_eq(channel_end_expected, channel_end);
	
	//Test that the channel occupies the correct amount of memory
	assert_eq((sizeof(struct SSPM_MP_DESC))+(4*32), sspm_mp_occupies(channel));
	
	//Test that the channel has all it's buffer free
	int free = sspm_mp_free(channel);
	int free_expected = 32;
	assert_eq(free_expected, free);

	//Start a slave
	int slaveId = 1;
	corethread_create(&slaveId, &slave1, (void*)channel);
	
	int receive_into[16];
	int progress = 0;
	int result = 0;	

	while(progress < GOAL){
		int received = sspm_mp_try_receive(channel, receive_into, 0, 16);
		if(received > 16){
			printf("%d: Error expected %x or less but got %x\n",__LINE__, 16, received);
			result |= 1;
		}
		if(received < 0){
			printf("%d: Error expected %x or larger but got %x\n",__LINE__, 0, received);
			result |= 1;
		}	

		for(int i=0; i<received; i++){
			if(progress !=  receive_into[i]){
				printf("%d: Error expected %x but got %x\n",__LINE__, progress, receive_into[i]);
				result |= 1;
			}
			progress++;
		}
	}
	if(progress !=  GOAL){
		printf("%d: Error expected %x but got %x\n",__LINE__, GOAL, progress);
		result |= 1;
	}

	//Join with the slave
	int *res;
	corethread_join(slaveId, (void **) &res);

	//Make sure the channel is empty
	int received = sspm_mp_try_receive(channel, receive_into, 0, 16);
	if(received !=  0){
		printf("%d: Error expected %x but got %x\n",__LINE__, 0, received);
		result |= 1;
	}
	printf("simple_multicore_test done.\n");
	return result;
}

int transmission_test(){
	volatile _SPM struct SSPM_MP_DESC *channel = LOWEST_SSPM_ADDRESS+32;

	//Test that the correct address is returned.
	int	init_return = sspm_mp_init(channel, 7);
	int init_return_expected = (LOWEST_SSPM_ADDRESS+32+(sizeof(struct SSPM_MP_DESC))+(4*128));
	assert_eq(init_return_expected, init_return);
	
	//Test that the channel occupies the correct amount of memory
	assert_eq((sizeof(struct SSPM_MP_DESC))+(4*128), sspm_mp_occupies(channel));
	
	//Test transmission state
	assert_eq(ACKNOWLEDGED, channel->transmission);
	
	//Test that there is nothing to receive
	int to_receive[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
	int transmission_end;
	int received = sspm_mp_transmission_try_receive(channel, to_receive, 0, 8,&transmission_end);
	int received_expected = 0;
	int transmission_end_expected = 0;
	assert_eq(received_expected, received);
	assert_eq(transmission_end_expected, transmission_end);
	
	//Test that can send
	int to_send[8] = {0,1,2,3,4,5,6,7};
	int sent = sspm_mp_transmission_try_send(channel, to_send,0,4);
	int sent_expected = 4;
	assert_eq(sent_expected,sent);

	//Test the channel is now ongoing
	assert_eq(ONGOING, channel->transmission);

	//Test the channel has less free
	int free = sspm_mp_free(channel);
	int free_expected = 124;
	assert_eq(free_expected, free);
	
	//Test can send more
	sent = sspm_mp_transmission_try_send(channel, to_send,4,4);
	sent_expected = 4;
	assert_eq(sent_expected,sent);

	//Test the channel is still ongoing
	assert_eq(ONGOING, channel->transmission);
	
	//Test the channel has less free
	free = sspm_mp_free(channel);
	free_expected = 120;
	assert_eq(free_expected, free);
	
	//Test can recieve all sent
	received = sspm_mp_transmission_try_receive(channel, to_receive, 0, 8,&transmission_end);
	received_expected = 8;
	transmission_end_expected = 0;
	assert_eq(received_expected, received);
	assert_eq(transmission_end_expected, transmission_end);
	assert_eq(ONGOING, channel->transmission);

	//Test received the right values
	for(int expected = 0; expected<8; expected++){
		assert_eq(expected, to_receive[expected]);
	}

	//Test can send more
	sent = sspm_mp_transmission_try_send(channel, to_send,0,4);
	sent_expected = 4;
	assert_eq(sent_expected,sent);
	assert_eq(ONGOING, channel->transmission);

	//Test can end transmission
	sspm_mp_transmission_end(channel);
	assert_eq(ENDED, channel->transmission);

	//Test can receive and acknowledge transmission end
	for(int i=0; i<8; i++){
		to_receive[i] = -1;
	}
	received = sspm_mp_transmission_try_receive(channel, to_receive, 0, 8,&transmission_end);
	received_expected = 4;
	transmission_end_expected = 1;
	assert_eq(received_expected, received);
	assert_eq(transmission_end_expected, transmission_end);
	assert_eq(ACKNOWLEDGED, channel->transmission);
	
	//Test receive the right values
	for(int expected = 0; expected<4; expected++){
		assert_eq(expected, to_receive[expected]);
	}
	for(int i=4; i<8; i++){
		assert_eq(-1, to_receive[i]);
	}	
	printf("transmission_test done.\n");
	return 0;
}

const int TRANSMISSION_MULTICORE_TEST_TRANSMISSIONS = 4;
const int TRANSMISSION_MULTICORE_TEST_GOAL = 3;
void transmission_multicore_test_slave(void* args){
	
	volatile _SPM struct SSPM_MP_DESC *channel = (volatile _SPM struct SSPM_MP_DESC *)args;
	
	int transmission_progress = 0;
	int goal_progress = 0;
	int goal_progress_buffer[TRANSMISSION_MULTICORE_TEST_GOAL];
	for(int i = 0; i < TRANSMISSION_MULTICORE_TEST_GOAL; i++){
		goal_progress_buffer[i] = i;
	}
	
	while(transmission_progress <TRANSMISSION_MULTICORE_TEST_TRANSMISSIONS){
		while(goal_progress < TRANSMISSION_MULTICORE_TEST_GOAL){
			int sent = sspm_mp_transmission_try_send(channel, 
						goal_progress_buffer, goal_progress, 
						TRANSMISSION_MULTICORE_TEST_GOAL-goal_progress);
			goal_progress += sent;
		}
		sspm_mp_transmission_end(channel);
		goal_progress = 0;
		transmission_progress++;
	}
}

int transmission_multicore_test_main(){
	
	volatile _SPM struct SSPM_MP_DESC *channel = LOWEST_SSPM_ADDRESS;
	
	sspm_mp_init(channel, 4);
	
	//Start slave
	int slaveId = 1;
	corethread_create(&slaveId, &transmission_multicore_test_slave, (void*)channel);
		
	int result = 0;
	int receive_into[64];
	int transmission_end;
	int transmission_progress = 0;
	int goal_progress = 0;

	while(transmission_progress < TRANSMISSION_MULTICORE_TEST_TRANSMISSIONS){
		while(goal_progress < TRANSMISSION_MULTICORE_TEST_GOAL){
			int received = sspm_mp_transmission_try_receive(
							channel, receive_into,0,64,transmission_end);
			if(received > 64){
				printf("%d: Error expected %x or less but got %x\n",__LINE__, 64, received);
				result |= 1;
			}
			if(received < 0){
				printf("%d: Error expected %x or larger but got %x\n",__LINE__, 0, received);
				result |= 1;
			}	
			for(int i=0; i<received; i++){
				if(goal_progress !=  receive_into[i]){
					printf("%d: Error expected %x but got %x\n",__LINE__, 
						goal_progress, receive_into[i]);
					result |= 1;
				}
				goal_progress++;
			}
			if(transmission_end){
				break;
			}
		}
		if(goal_progress !=  TRANSMISSION_MULTICORE_TEST_GOAL){
			printf("%d: Error expected %x but got %x\n",__LINE__, 
						TRANSMISSION_MULTICORE_TEST_GOAL, goal_progress);
			result |= 1;
		}
		goal_progress = 0;
		transmission_progress++;
	}
	if(transmission_progress !=  TRANSMISSION_MULTICORE_TEST_TRANSMISSIONS){
		printf("%d: Error expected %x but got %x\n",__LINE__, 
				TRANSMISSION_MULTICORE_TEST_TRANSMISSIONS, transmission_progress);
		result |= 1;
	}
	//Join with the slave
	int *res;
	corethread_join(slaveId, (void **) &res);
	
	//Make sure the channel is empty
	int received = sspm_mp_try_receive(channel, receive_into, 0, 16);
	if(received !=  0){
		printf("%d: Error expected %x but got %x\n",__LINE__, 0, received);
		result |= 1;
	}
	printf("transmission_multicore_test_main done.\n");
	return result;
}

int main(){
	led_off_for(2000);
	if(
			!simple_test()
		&& 	!simple_multicore_test()
		&&	!transmission_test()
		&&	!transmission_multicore_test_main()
	){
		printf(">>>>>>>>>>>>>>>>>Success<<<<<<<<<<<<<<<<<<<<<<<");
	}else{
		printf(">>>>>>>>>>>>>>>>>Failure<<<<<<<<<<<<<<<<<<<<<<<");
	}
	return 0;
}






































