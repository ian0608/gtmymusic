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
#include "gtmymusic.h"

		
#define ARG1_SIZE 4
#define ARG2_SIZE MD5_DIGEST_LENGTH
#define CLNT_REQ_BUFSIZE ARG1_SIZE + 1 + ARG2_SIZE +1 /* The client request should be 22 bytes long*/
#define MAXPENDING 5


void DieWithErr(char *errorMessage){
    printf("%s\n", errorMessage);
    exit(EXIT_FAILURE);
}

void Err(char *errorMessage) {
    printf("%s\n", errorMessage);
}

struct ThreadArgs {
    int clntSock;
};

void *ThreadMain(void *args);
void send_list(int clientSock);
void pull_resp(int clinetSock, unsigned char hash[ARG2_SIZE]);

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
    
    int clientSock;				/* Client Socket */
    int i;
    char clientRequest[CLNT_REQ_BUFSIZE];/* Buff to store name from client */
    char *clientReqp = clientRequest;
    char clientArg1[ARG1_SIZE];
    unsigned char clientArg2[ARG2_SIZE];
    
    // Setup Pthread & get the args
    pthread_detach(pthread_self());
    clientSock = ((struct ThreadArgs *) threadArgs)->clntSock;
    free(threadArgs);

//LOOP FOR CLIENT REQUESTS
while(1) {

    // Clear the buffers
    memset(clientRequest, 0, CLNT_REQ_BUFSIZE);
    memset(clientArg1, 0, ARG1_SIZE);
    memset(clientArg2, 0, ARG2_SIZE);
    
    unsigned int numBytesRecvd = 0;
    /* Extract CLIENT REQUEST ARG 1 from the packet, store in clientRequest, arg1*/ 
    while (numBytesRecvd < ARG1_SIZE) {
    	numBytesRecvd += recv(clientSock, clientRequest + numBytesRecvd, CLNT_REQ_BUFSIZE, 0);
   	if (numBytesRecvd < 0) {
        	Err("recv() failed");
		return(NULL);
	}
    	else if (numBytesRecvd == 0){
		Err("recv() closed prematurely");
		return(NULL);
	}
    }
//
    printf("Client Request: %s\n", clientRequest);
    
    memcpy(clientArg1, clientReqp, (size_t) ARG1_SIZE);
    
    /* DETERMINE NEXT FUNCTION CALL */
    if ((memcmp(clientArg1, "PULL", ARG1_SIZE)) == 0) {
	    	printf("PULL\n");
	    	/* Extract CLIENT REQUEST from the packet, store in clientRequest, arg1 and arg2 */ 
	    	while (numBytesRecvd < CLNT_REQ_BUFSIZE) {
	    		numBytesRecvd += recv(clientSock, clientRequest + numBytesRecvd, CLNT_REQ_BUFSIZE, 0);
   			if (numBytesRecvd < 0) {
				Err("recv() failed");
				return(NULL);
			}
	    		else if (numBytesRecvd == 0) {
				Err("recv() closed prematurely");
				return(NULL);
			}
	    	}
	    	clientReqp = clientRequest;
    		memcpy(clientArg2, clientReqp + ARG1_SIZE + 1, (size_t) ARG2_SIZE);

    		printf("Client Arg 2: ");
    
    		for (i = 0; i < ARG2_SIZE; i++) { 
    			printf("%02x", clientArg2[i]);
   		}
    		printf("\n");
	    	pull_resp(clientSock, clientArg2);
    }
    else if((memcmp(clientArg1, "LIST", ARG1_SIZE)) == 0) {
	printf("LIST\n");
        send_list(clientSock);
    }
    else if((memcmp(clientArg1, "QUIT", ARG1_SIZE)) == 0) {
	close(clientSock);
	return(NULL);
    }
    else {
        printf("NOT A VALID REQUEST\n");
    }
}

	
}

void pull_resp(int clientSock, unsigned char hash[ARG2_SIZE]) {
	char *sendBuff;
    	FILE *file1;
	char *filename = NULL;
	//char *test = "04 Son's Gonna Rise.mp3";
	
	list_item_array *myList = get_list_items_current_dir();
	if (myList == NULL) {
		Err("get_list_items_current_dir() failed");
	}
    
	int listCount = myList->count;
	int i;
	for (i = 0; i < listCount; i ++)		{
		int j;
		for (j=0; j < MD5_DIGEST_LENGTH; j++) {		
		        if (memcmp(myList->items[i]->hash, hash, ARG2_SIZE) == 0) {
			      printf("%s\n", myList->items[i]->filename);
			      filename = myList->items[i]->filename;
			}
			printf("%02x", myList->items[i]->hash[j]);
		}
		printf("\n");
	}
	printf("File: %s\n", filename);


	int64_t sendBuffSize;
	int64_t fileSize;

	if (filename != NULL) {
		    /* OPEN FILE */
	   	if ((file1 = fopen(filename, "r")) == NULL){
	      		Err("File I/O err: fopen() failed");
	    	}
	    
	    	/* COMPUTE FILE SIZE */
	    	if (fseek(file1, 0, SEEK_END) == 0) {
			fileSize = ftell(file1);
			sendBuffSize = fileSize + sizeof(int64_t);
	       		if (sendBuffSize == -1) {
		    		Err("ftell() failed to SEEK_END");
			}
			printf("sendBuffSize: %" PRId64 " bytes \n", sendBuffSize);
	    	}
		else {
			Err("fseek() failed failed to SEEK_END");
		}
	    
	    	sendBuff = malloc(sendBuffSize);
	    	if (sendBuff == NULL) {
			Err("malloc() failed");
	    	}
	    	if (fseek(file1, 0, SEEK_SET) != 0) {
			Err("fseek() failed to SEEK_SET");
	    	}
	    
	  
		memcpy(sendBuff, &fileSize, sizeof(int64_t));
	    
	    
	    	/* READ FILE TO BUFFER*/
	    	size_t newLen = fread(sendBuff + sizeof(int64_t), sizeof(char), fileSize, file1);
	    	if (newLen == 0) {
			Err("File I/O err: fread() failed");
	    	} else {
	    	    	sendBuff[++newLen] = '\0'; // Just to be safe add null terminator
	    	}
	}
	else{
		fileSize = -1;
		sendBuffSize = sizeof(int64_t);
		sendBuff = malloc(sendBuffSize);
		if (sendBuff == NULL) {
			Err("malloc() failed");		
		}
		memcpy(sendBuff, &fileSize, sizeof(int64_t));
	}

  	/* Send file to client */
	/*	FILL IN	  */
 	ssize_t numBytesSent = 0;
 	   
 	while (numBytesSent < sendBuffSize) {
     		numBytesSent += send(clientSock, sendBuff + numBytesSent, sendBuffSize, 0);
    	    	printf("Number of bytes sent %zu\n", numBytesSent);
    	}
    
    	free(sendBuff);

}

void send_list(int clientSock) {
	char *sendBuff;
	
	list_item_array *myList = get_list_items_current_dir();

	if (myList == NULL) {
		Err("get_list_items_current_dir() failed");
	}
	
		

	int32_t listCount = myList->count;
	size_t sendBuffSize = sizeof(int32_t) + sizeof(list_item)*listCount;
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
	printf("SENDING FILE\n");
	/* Send file to client */
	/*	FILL IN	  */
   	ssize_t numBytesSent = 0;
    
    	while (numBytesSent < sendBuffSize) {
        	numBytesSent += send(clientSock, sendBuff + numBytesSent, sendBuffSize, 0);
        	printf("Number of bytes sent %zu\n", numBytesSent);
    	}

	free(sendBuff);

    
    
}


 
