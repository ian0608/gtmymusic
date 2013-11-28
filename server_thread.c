/*///////////////////////////////////////////////////////////
*
* FILE:		server.c
* AUTHOR:	Warren Shenk, Ian Stainbrook
* PROJECT:	CS 3251 Project 2 - Professor Traynor
* DESCRIPTION:	Network Server Code
*
*////////////////////////////////////////////////////////////

/*Included libraries*/

#include <stdio.h>	  /* for printf() and fprintf() */
#include <sys/socket.h>	  /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>	  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>	  /* supports all sorts of functionality */
#include <unistd.h>	  /* for close() */
#include <string.h>	  /* support any string ops */
#include <openssl/evp.h>  /* for OpenSSL EVP digest libraries/SHA256 */
#include <pthread.h>
#include <stdint.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <inttypes.h>
//#include <libxml/parser.h>
#include "gtmymusic.h"

#define _MULTI_THREADED		
#define ARG1_SIZE 4
#define CLNT_REQ_CAP_BUFSIZE ARG1_SIZE + 4 /* Client CAP request is 8 bytes: "CAP "+ int32_t */
#define CLNT_REQ_PULL_BUFSIZE ARG1_SIZE + 4
#define CAP_ACK_SIZE 5
#define FILENAME_LENGTH 257
#define MAXPENDING 5

pthread_mutex_t logLock = PTHREAD_MUTEX_INITIALIZER;
__thread int32_t cap = -1;


struct ThreadArgs {
    int clntSock;
};

void *ThreadMain(void *args);
void cap_resp(int clientSock, int32_t clientCap);
void send_list(int clientSock);
void send_list2(int clientSock);
void pull_resp(int clientSock, int bufferCount, unsigned char *buffer);
void send_file_from_hash(int clientSock, unsigned char hash[MD5_DIGEST_LENGTH]);
void logger(int socket, char *string);

/* The main function */
int main(int argc, char *argv[])
{

    int serverSock;				/* Server Socket */
    int clientSock;				/* Client Socket */
    struct sockaddr_in changeServAddr;		/* Local address */
    struct sockaddr_in changeClntAddr;		/* Client address */
    unsigned short changeServPort;		/* Server port */
    unsigned int clntLen;			/* Length of address data struct */
    
    changeServPort = 6079;

    /* Create new TCP Socket for incoming requests*/
    /*	    FILL IN	*/
    if ((serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithErr("socket() failed");

	int on = 1;
	setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    
    /* Construct local address structure*/
    /*	    FILL IN	*/
    memset(&changeServAddr, 0, sizeof(changeServAddr));
    changeServAddr.sin_family = AF_INET;
    changeServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    changeServAddr.sin_port = htons(changeServPort);
    
    
    /* Bind to local address structure */
    /*	    FILL IN	*/
    if (bind(serverSock, (struct sockaddr*) &changeServAddr, sizeof(changeServAddr)) < 0)
        DieWithErr("bind() failed");

    
    /* Listen for incoming connections */
    /*	    FILL IN	*/
    if (listen(serverSock, MAXPENDING) < 0)
        DieWithErr("listen() failed");
    

    /* Loop server forever*/
    while(1)
    {
        clntLen = sizeof(changeClntAddr);
        
        /* Accept incoming connection */
        /*	FILL IN	    */
        clientSock = accept(serverSock, (struct sockaddr *) &changeClntAddr, &clntLen);
        if (clientSock < 0)
            DieWithErr("accept failed");
        
        //Construct ThreadArgs
        struct ThreadArgs *threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        if (threadArgs == NULL)
            DieWithErr("malloc() failed");
        threadArgs->clntSock = clientSock;
        
        
        pthread_t threadID;
        int returnValue = pthread_create(&threadID, NULL, ThreadMain, threadArgs);
        if (returnValue != 0)
            DieWithErr("pthread_create() failed");
        printf("with thread %lu\n", (unsigned long int) threadID);

    }

}

void *ThreadMain(void *threadArgs) {
    int clientSock;		
    char clientArg1[ARG1_SIZE];
    int32_t clientCapArg2 = 0;
    int32_t clientPullArg2 = 0;
    unsigned char *clientPullArg3;
    unsigned int numBytesRecvd = 0;
    // Setup Pthread & get the args
    pthread_detach(pthread_self());
    clientSock = ((struct ThreadArgs *) threadArgs)->clntSock;
    free(threadArgs);

//LOOP FOR CLIENT REQUESTS
while(1) {

    // Clear the buffers
    memset(clientArg1, 0, ARG1_SIZE);
    clientPullArg2 = 0;
    
    numBytesRecvd = 0;
    /* Extract CLIENT REQUEST ARG 1 from the packet, store in clientArg1, arg1*/ 
    while (numBytesRecvd < ARG1_SIZE) {
    	numBytesRecvd += recv(clientSock, clientArg1 + numBytesRecvd, ARG1_SIZE, MSG_WAITALL);
   	if (numBytesRecvd < 0) {
        	Err("recv() failed");
		return(NULL);
	}
    	else if (numBytesRecvd == 0){
		Err("recv() closed prematurely 1");
		return(NULL);
	}
    }
    printf("ARG 1 NUM BYTES RECV: %i\n", numBytesRecvd);
    
    
    /* DETERMINE NEXT FUNCTION CALL */
    if ((memcmp(clientArg1, "PULL", ARG1_SIZE)) == 0) {
	printf("PULL\n");
	numBytesRecvd = 0;
    	while (numBytesRecvd < sizeof(int32_t)) {
    		numBytesRecvd += recv(clientSock, &clientPullArg2 + numBytesRecvd, sizeof(int32_t), MSG_WAITALL);
		if (numBytesRecvd < 0) {
			Err("recv() failed");
			return(NULL);
		}
    		else if (numBytesRecvd == 0) {
			Err("recv() closed prematurely");
			return(NULL);
		}
    	}
	printf("ARG 2 NUM BYTES RECV: %i", numBytesRecvd);
	clientPullArg2 = ntohl(clientPullArg2);
	printf("           Number of Requested Files: %i\n", clientPullArg2);
	clientPullArg3 = malloc(MD5_DIGEST_LENGTH*clientPullArg2);
	if (clientPullArg3 == NULL)
		DieWithErr("malloc() Pull Arg 3 failed");
	memset(clientPullArg3, 0, MD5_DIGEST_LENGTH*clientPullArg2);
	numBytesRecvd = 0;
	while (numBytesRecvd < MD5_DIGEST_LENGTH*clientPullArg2) {
    		numBytesRecvd += recv(clientSock, clientPullArg3 + numBytesRecvd, MD5_DIGEST_LENGTH*clientPullArg2, MSG_WAITALL);
		if (numBytesRecvd < 0) {
			Err("recv() failed");
			return(NULL);
		}
    		else if (numBytesRecvd == 0) {
			Err("recv() closed prematurely");
			return(NULL);
		}
    	}
	printf("ARG 3 NUM BYTES RECV: %i\n", numBytesRecvd);
	pull_resp(clientSock, clientPullArg2, clientPullArg3);
	free(clientPullArg3);
    }
    else if((memcmp(clientArg1, "LIST", ARG1_SIZE)) == 0) {
	printf("LIST\n");
        send_list2(clientSock);
    }
    else if ((memcmp(clientArg1, "CAP ", ARG1_SIZE)) == 0) {
	printf("CAP\n");
    	while (numBytesRecvd < CLNT_REQ_CAP_BUFSIZE) {
    		numBytesRecvd += recv(clientSock, &clientCapArg2 + numBytesRecvd, sizeof(int32_t), MSG_WAITALL);
		if (numBytesRecvd < 0) {
			Err("recv() failed");
			return(NULL);
		}
    		else if (numBytesRecvd == 0) {
			Err("recv() closed prematurely");
			return(NULL);
		}
    	}
	printf("ARG 2 NUM BYTES RECV: %i   ", numBytesRecvd);
	clientCapArg2 = ntohl(clientCapArg2);
	printf("  Cap: %i\n", clientCapArg2);
 	cap_resp(clientSock, clientCapArg2);

    }
    else if((memcmp(clientArg1, "QUIT", ARG1_SIZE)) == 0) {
	close(clientSock);
	return(NULL);
    }
    else {
        printf("\nNOT A VALID REQUEST\n\n");
    }

}

	
}

void cap_resp(int clientSock, int32_t clientCap){ 
	cap = 1049000*clientCap;
	char ackBuffer[CAP_ACK_SIZE] = "CAPOK";
	ssize_t numBytesSent = 0;
 	   
 	while (numBytesSent < CAP_ACK_SIZE) {
     		numBytesSent += send(clientSock, ackBuffer + numBytesSent, CAP_ACK_SIZE, 0);
    	    	printf("Number of bytes sent %zu\n", numBytesSent);
    	}
}

void pull_resp(int clientSock, int bufferCount, unsigned char *buffer) {
	unsigned char *sendBuff = NULL;
    	FILE *file1;
	int i;
	int j;
	int match;
	//char *test = "04 Son's Gonna Rise.mp3";
	

	// Get list of mp3s in directory
	list_item_array *myList = get_list_items_current_dir();
	if (myList == NULL) {
		Err("get_list_items_current_dir() failed");
	}

	/*/ Prints hash buffer containing client requested hashes
	for (j=0; j < MD5_DIGEST_LENGTH*bufferCount; j++)				
		printf("%02x", buffer[j]);
	printf("\n");*/

	// Iterates through list and removes mp3s with unrequested hashes
	for(i = 0; i < myList->count; i++) {
		match = 0;
		for(j = 0; j < bufferCount; j++){
			//printf("Check %i %i\n", i, j);
			if(memcmp(myList->items[i]->hash, buffer + MD5_DIGEST_LENGTH*j, MD5_DIGEST_LENGTH) == 0) {
				match++;
				//printf("MATCH\n");
			}
		}
		if (match <= 0) {
			printf("DELETE\n");
			delete_index_from_array(&myList, i);
			i--;
		}
	}


	/*	
	for(i = 0; i < myList->count; i++) {
		printf("%i Filename: %s Playcount:%i\n", i, myList->items[i]->filename, myList->items[i]->playcount);
	}*/

	// Check for a cap
	if (cap > 0) {	
		
		// If cap exists sort the array in descending popularity
		sort_descending_playcount(&myList);

		// Get the cap and decrement for CAP_ACK and # files
		int tempCap = cap - CAP_ACK_SIZE - sizeof(int32_t);

		// Iterate through list and check each filesize
		for (i = 0; i < myList->count; i++) {
			// Calculate bytes required to send file 
			int32_t bytesRequiredToSend = (myList->items[i]->filesize + sizeof(int32_t));
			
			// If small enough, do not delete file from list
			if (bytesRequiredToSend < tempCap) {
				tempCap -= bytesRequiredToSend;			
			}
			
			// If too large, delete file from list
			else {
				delete_index_from_array(&myList, i);
				i--;
			}
		}
	}
	
	// Create variable listCount and its network counterpart
	int listCount = myList->count;
	int networkListCount = htonl(listCount);
	
	// Send # of outgoing files to client
	int numBytesSent = 0;   
 	while (numBytesSent < sizeof(int32_t)) {
     		numBytesSent += send(clientSock, &networkListCount + numBytesSent, sizeof(int32_t), 0);
    	    	printf("Number of bytes sent %zu ", numBytesSent);
    	}
	
	int32_t networkFileSize;
	int32_t fileSize;
	unsigned char nameBuffer[FILENAME_LENGTH];

	// Iterate through each item in the list and send to client
	for (i = 0; i < listCount; i ++)		{
		if (myList->items[i]->filename != NULL) {
		    
			// Get the filename into nameBuffer
			memset(nameBuffer,0, FILENAME_LENGTH);
			memcpy(nameBuffer, myList->items[i]->filename, FILENAME_LENGTH);

			// Send namebuffer to client
			numBytesSent = 0;
		 	while (numBytesSent < FILENAME_LENGTH) {
		     		numBytesSent += send(clientSock, nameBuffer + numBytesSent, FILENAME_LENGTH, 0);
		    	    	printf("%zu\n", numBytesSent);
		    	}
			

			// Get the filesize and its network counterpart
			fileSize = myList->items[i]->filesize;
			printf("sendBuffSize: %i bytes \n", fileSize);						
			networkFileSize = htonl(fileSize);			
	
			// Send filesize to client
			numBytesSent = 0;
		 	while (numBytesSent < sizeof(int32_t)) {
		     		numBytesSent += send(clientSock, &networkFileSize + numBytesSent, sizeof(int32_t), 0);
		    	    	printf("Number of bytes sent %zu ", numBytesSent);
		    	}

			// Allocate memory for filebuffer
		    	sendBuff = realloc(sendBuff, fileSize);
		    	if (sendBuff == NULL) {
				Err("malloc() failed");
		    	}

			// OPEN FILE 
		   	if ((file1 = fopen(myList->items[i]->filename, "r")) == NULL){
		      		Err("File I/O err: fopen() failed");
		    	}

		    	// READ FILE TO BUFFER
		    	size_t newLen = fread(sendBuff, fileSize, 1, file1);
		    	if (newLen == 0) {
				Err("File I/O err: fread() failed");
		    	} 

			// Close the file
			fclose(file1);

			// Send filebuffer to client
			numBytesSent = 0;
			while (numBytesSent < fileSize) {
		     		numBytesSent += send(clientSock, sendBuff + numBytesSent, fileSize, 0);
		    	    	printf("%zu     ", numBytesSent);
		    	}

			// Log the transaction
			printf("File: %s\n", myList->items[i]->filename);
			logger(clientSock, myList->items[i]->filename);

		}
		else{
			fileSize = -1;
			networkFileSize = htonl(fileSize);
			numBytesSent = 0;
		 	while (numBytesSent < sizeof(int32_t)) {
		     		numBytesSent += send(clientSock, &networkFileSize + numBytesSent, sizeof(int32_t), 0);
		    	    	printf("Number of bytes sent %zu\n", numBytesSent);
		    	}
		}  
	}
	if(sendBuff != NULL){
		free(sendBuff);	
	}
    	
	teardown_list_item_array(myList);

}


void send_list(int clientSock) {
	logger(clientSock, "list");

	char *sendBuff;
	int32_t listCount; 	
	size_t sendBuffSize;
	ssize_t numBytesSent;
	
	list_item_array *myList = get_list_items_current_dir();

	if (myList == NULL) {
		Err("get_list_items_current_dir() failed");
		sendBuffSize = sizeof(int32_t);
		listCount = 0;
		sendBuff = malloc(sendBuffSize);
		if (sendBuff == NULL) {
			Err("malloc failed");
		}
		memcpy(sendBuff, &listCount, sizeof(int32_t));
		printf("SENDING LIST ITEMS\n");
		/* Send file to client */
		/*	FILL IN	  */
	   	numBytesSent = 0;
	    
	    	while (numBytesSent < sendBuffSize) {
			numBytesSent += send(clientSock, sendBuff + numBytesSent, sendBuffSize, 0);
			printf("Number of bytes sent %zu\n", numBytesSent);
	    	}

		free(sendBuff);
	}
	else {
	
		

		listCount = myList->count;
		sendBuffSize = sizeof(int32_t) + sizeof(list_item)*listCount;
		printf("Send Buffer Size: %zu\n", sendBuffSize);
		sendBuff = malloc(sendBuffSize);
	
		memcpy(sendBuff, &listCount, sizeof(int32_t));
		int i=0;

		while (i < listCount)	//for each list_item
		{	
			printf("i: %i\n",i);
			printf("List Item Name: %s\n", myList->items[i]->filename);
			int j = 0;
			for (j=0; j < MD5_DIGEST_LENGTH; j++)				//and print the hash
				printf("%02x", myList->items[i]->hash[j]);
			printf("\n");
			memcpy(sendBuff + sizeof(int32_t) + i*sizeof(list_item), myList->items[i], sizeof(list_item));
	    		i++;
		}
		printf("SENDING LIST ITEMS\n");
		/* Send file to client */
		/*	FILL IN	  */
	   	numBytesSent = 0;
	    
	    	while (numBytesSent < sendBuffSize) {
			numBytesSent += send(clientSock, sendBuff + numBytesSent, sendBuffSize, 0);
			printf("Number of bytes sent %zu\n", numBytesSent);
	    	}

		free(sendBuff);

    	}
    
}

void send_list2(int clientSock) {
	logger(clientSock, "list");

	char *sendBuff;
	int32_t listCount; 	
	size_t sendBuffSize;
	ssize_t numBytesSent;
	
	list_item_array *myList = get_list_items_current_dir();

	if (myList == NULL) {
		Err("get_list_items_current_dir() failed");
		sendBuffSize = sizeof(int32_t);
		listCount = 0;
		sendBuff = malloc(sendBuffSize);
		if (sendBuff == NULL) {
			Err("malloc failed");
		}
		memcpy(sendBuff, &listCount, sizeof(int32_t));
		printf("SENDING LIST ITEMS\n");
		/* Send file to client */
		/*	FILL IN	  */
	   	numBytesSent = 0;
	    
	    	while (numBytesSent < sendBuffSize) {
			numBytesSent += send(clientSock, sendBuff + numBytesSent, sendBuffSize, 0);
			printf("Number of bytes sent %zu\n", numBytesSent);
	    	}

		free(sendBuff);
	}
	else {
	
		

		listCount = myList->count;
		sendBuffSize = sizeof(int32_t) + (MD5_DIGEST_LENGTH + FILENAME_LENGTH)*listCount;
		printf("Send Buffer Size: %zu\n", sendBuffSize);
		sendBuff = malloc(sendBuffSize);
	
		int32_t listCountNetwork = htonl(listCount);
		memcpy(sendBuff, &listCountNetwork, sizeof(int32_t));
		int i=0;

		while (i < listCount)	//for each list_item
		{	
			printf("i: %i\n",i);
			printf("List Item Name: %s\n", myList->items[i]->filename);
			int j = 0;
			for (j=0; j < MD5_DIGEST_LENGTH; j++)				//and print the hash
				printf("%02x", myList->items[i]->hash[j]);
			printf("\n");
			memcpy(sendBuff + sizeof(int32_t) + i*(MD5_DIGEST_LENGTH + FILENAME_LENGTH), myList->items[i]->hash, MD5_DIGEST_LENGTH);
			char filenameBuff[FILENAME_LENGTH];
			memset(&filenameBuff, 0, FILENAME_LENGTH);
			memcpy(filenameBuff, myList->items[i]->filename, strlen(myList->items[i]->filename));
			memcpy(sendBuff + sizeof(int32_t) + i*(MD5_DIGEST_LENGTH + FILENAME_LENGTH) + MD5_DIGEST_LENGTH, filenameBuff, FILENAME_LENGTH);
			
	    		i++;
		}
		printf("SENDING LIST ITEMS\n");
		/* Send file to client */
		/*	FILL IN	  */
	   	numBytesSent = 0;
	    
	    	while (numBytesSent < sendBuffSize) {
			numBytesSent += send(clientSock, sendBuff + numBytesSent, sendBuffSize, 0);
			printf("Number of bytes sent %zu\n", numBytesSent);
	    	}

		free(sendBuff);

    	}
    
}


 void logger(int socket, char *string) {

	char * hostip = 0;
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	int err = getpeername(socket, (struct sockaddr *) &addr, &addr_len);
	if (err != 0) {
   		// error
	}
	else {
		time_t ltime; /* calendar time */
	    	ltime=time(NULL); /* get current cal time */
	  

		hostip = inet_ntoa(addr.sin_addr);

		printf("IP: %s    Accessed Item: %s\n", hostip, string); 
		
		pthread_mutex_lock(&logLock);

		FILE * fp;

	   	fp = fopen ("log.txt", "a+");
	   	fprintf(fp, "IP: %s    Accessed Item: %s     Time: %s\n", hostip, string, asctime(localtime(&ltime)));
	   
	   	fclose(fp);	

		pthread_mutex_unlock(&logLock);
	}



}
