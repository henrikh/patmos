
#include "coprint.h"

int initAddress;
char char_buffers[NR_CORES*CHANNEL_BUFFER_SIZE];

int coprint_initialize(int initAt){
	initAddress = initAt;

	int i;
	volatile _SPM int *flagP;
	for(i = 0; i< NR_CORES; i++){
		flagP = coprint_flag_address(i);
		//printf("flag: %x\n",flagP);
		*flagP = 0;
	}

	return coprint_flag_address(NR_CORES);
}


int coprint_can_receive(int coreNr){
	volatile _SPM int *flagP = coprint_flag_address(coreNr);
	return  *flagP;
}


char* coprint_receive(int coreNr){
	volatile _SPM int *sspmBufferP = coprint_buffer_start(coreNr);
	char *localBufferP = coprint_local_buffer(coreNr);
	
	int i = 0;
	while( *sspmBufferP != 0 && i<(CHANNEL_BUFFER_SIZE-1)){
		char c = (char) *sspmBufferP;
		localBufferP[i] = c;
		sspmBufferP = sspmBufferP + 4;
		i++;
	}
	localBufferP[i] = 0;
	return localBufferP;
}

char* coprint_wait_receive(int coreNr){
	while(!coprint_can_receive(coreNr)){}
	char* ret = coprint_receive(coreNr);
	coprint_free_received(coreNr);
	return ret;
}


void coprint_free_received(int coreNr){
	volatile _SPM int *flagP = coprint_flag_address(coreNr);
	*flagP = 0;
}


int coprint_can_send(int coreNr){
	volatile _SPM int *flagP = coprint_flag_address(coreNr);
	return *flagP == 0;
}


void coprint_send(int coreNr, char *text){
	volatile _SPM int *sspmBufferP = coprint_buffer_start(coreNr);

	int i = 0;
	while(i<(CHANNEL_BUFFER_SIZE-1) && text[i]!=0){
		int c = text[i];
		*sspmBufferP = c;
		sspmBufferP = sspmBufferP + 4;
		i++;
	}
	*sspmBufferP = 0;
	volatile _SPM int *flagP = coprint_flag_address(coreNr);
	*flagP = 1;
}

void coprint_wait_send(int coreNr, char *text){
	while(!coprint_can_send(coreNr)){}
	coprint_send(coreNr, text);
}

int coprint_flag_address(int coreNr){
	
	return initAddress + (((1+CHANNEL_BUFFER_SIZE)*4)*coreNr);
}

int coprint_buffer_start(int coreNr){
	return coprint_flag_address(coreNr) + 4;	
}

char* coprint_local_buffer(int coreNr){
	return (char *) (char_buffers+(CHANNEL_BUFFER_SIZE*NR_CORES));
}


