
#include "string_to_master.h"

int initAddress;
char char_buffers[NR_CORES*CHANNEL_BUFFER_SIZE];

int initialize_messaging(int initAt){
	initAddress = initAt;

	int i;
	volatile _SPM int *flagP;
	for(i = 0; i< NR_CORES; i++){
		flagP = flag_address(i);
		//printf("flag: %x\n",flagP);
		*flagP = 0;
	}

	return flag_address(NR_CORES);
}


int can_receive_from_core(int coreNr){
	volatile _SPM int *flagP = flag_address(coreNr);
	return  *flagP;
}


char* receive_from_core(int coreNr){
	volatile _SPM int *sspmBufferP = buffer_start(coreNr);
	char *localBufferP = local_buffer(coreNr);
	
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


void free_received_from_core(int coreNr){
	volatile _SPM int *flagP = flag_address(coreNr);
	*flagP = 0;
}


int can_send_from(int coreNr){
	volatile _SPM int *flagP = flag_address(coreNr);
	return *flagP == 0;
}


void send_from_core(int coreNr, char *text){
	volatile _SPM int *sspmBufferP = buffer_start(coreNr);

	int i = 0;
	while(i<(CHANNEL_BUFFER_SIZE-1) && text[i]!=0){
		int c = text[i];
		*sspmBufferP = c;
		sspmBufferP = sspmBufferP + 4;
		i++;
	}
	*sspmBufferP = 0;
	volatile _SPM int *flagP = flag_address(coreNr);
	*flagP = 1;
}

int flag_address(int coreNr){
	
	return initAddress + (((1+CHANNEL_BUFFER_SIZE)*4)*coreNr);
}

int buffer_start(int coreNr){
	return flag_address(coreNr) + 4;	
}

char* local_buffer(int coreNr){
	return (char *) (char_buffers+(CHANNEL_BUFFER_SIZE*NR_CORES));
}


