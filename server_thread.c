/*///////////////////////////////////////////////////////////
*
* FILE:		thread_server.c
* AUTHOR:	Warren Shenk, Ian Stainbrook
* PROJECT:	CS 3251 Project 2 - Professor Traynor
* DESCRIPTION:	Network Server Code
*
*////////////////////////////////////////////////////////////

/*Included libraries*/
//

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

#define RCVBUFSIZE 512		/* The receive buffer size */
#define SNDBUFSIZE 512		/* The send buffer size */
#define CLNT_REQ_BUFSIZE 38		/* The client request should be 38 bytes long*/

                            //  [4: LIST/PULL] [1: Space] [32: File Hash] [1: \0]
#define MAXBUFLEN 10000
#define MAXPENDING 5
#define DEBUG             // Debug flag
//#define DUMP_FBUFF        // Flag: prints out the File Buffer prior to sending to client


void DieWithErr(char *errorMessage){
    printf("%s\n", errorMessage);
    exit(EXIT_FAILURE);
}

struct ThreadArgs {
    int clntSock;
};

void *ThreadMain(void *args);
void pull_resp(int clientSock);
void send_list(int clientSock);

/* The main function */
int main(int argc, char *argv[])
{

    int serverSock;				/* Server Socket */
    int clientSock;				/* Client Socket */
    struct sockaddr_in changeServAddr;		/* Local address */
    struct sockaddr_in changeClntAddr;		/* Client address */
    unsigned short changeServPort;		/* Server port */
    unsigned int clntLen;			/* Length of address data struct */\
    
    changeServPort = 6079;

    /* Create new TCP Socket for incoming requests*/
    /*	    FILL IN	*/
    if ((serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithErr("socket() failed");
    
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
    
    char clientRequest[CLNT_REQ_BUFSIZE];/* Buff to store name from client */
    char *clientReqp = clientRequest;
    char clientArg1[5];
    char clientArg2[33];
    
    // Clear the buffers
    memset(clientRequest,0,sizeof(clientRequest));
    memset(clientArg1, 0, sizeof(clientArg1));
    memset(clientArg2, 0, sizeof(clientArg2));
    
    // Setup Pthread & get the args
    pthread_detach(pthread_self());
    clientSock = ((struct ThreadArgs *) threadArgs)->clntSock;
    free(threadArgs);
    
    
    /* Extract CLIENT REQUEST from the packet, store in clientRequest, arg1 and arg2 */
    /*	FILL IN	    */
    ssize_t numBytesRecvd = recv(clientSock, clientRequest, CLNT_REQ_BUFSIZE, 0);
    if (numBytesRecvd < 0)
        DieWithErr("recv() failed");
    else
        printf("Client Request...\n%s\n", clientRequest);
    
    memcpy(clientArg1, clientReqp, (size_t) 4);
    clientArg1[4] = '\0';
    
    
    clientReqp = clientRequest;
    memcpy(clientArg2, clientReqp + 5, (size_t) 32);
    clientArg2[32] = '\0';
    
    
    
    /* DETERMINE NEXT FUNCTION CALL */
    if ((strcmp(clientArg1, "PULL")) == 0) {
	pull_resp(clientSock);
	exit(EXIT_SUCCESS);
    }
    else if((strcmp(clientArg1, "LIST")) == 0) {
        send_list(clientSock);
        exit(EXIT_SUCCESS);
        
    }
    else {
        printf("NOT A VALID REQUEST\n");
    }
    
    return (NULL);
}

void pull_resp(int clientSock) {
	char *sendBuff;
    	FILE *file1;
	
	list_item_array *myList = get_list_items_current_dir();
	if (myList == NULL) {
		DieWithErr("get_list_items_current_dir() failed");
	}
    
	//int32_t listCount = myList->count;

	int64_t sendBuffSize;
	int64_t fileSize;
	    /* OPEN FILE */
   	if ((file1 = fopen("04 Son's Gonna Rise.mp3", "r")) == NULL){
      		DieWithErr("File I/O err: fopen() failed");
    	}
    
    	/* COMPUTE FILE SIZE */
    	if (fseek(file1, 0, SEEK_END) == 0) {
		fileSize = ftell(file1);
        	sendBuffSize = fileSize + sizeof(int64_t);
       		if (sendBuffSize == -1) {
            		DieWithErr("ftell() failed to SEEK_END");
        	}
        	printf("sendBuffSize: %" PRId64 " bytes \n", sendBuffSize);
    	}
	else {
        	DieWithErr("fseek() failed failed to SEEK_END");
    	}
    
    	sendBuff = malloc(sendBuffSize);
    	if (sendBuff == NULL) {
        	DieWithErr("malloc() failed");
    	}
    	if (fseek(file1, 0, SEEK_SET) != 0) {
        	DieWithErr("fseek() failed to SEEK_SET");
    	}
    
  
        memcpy(sendBuff, &fileSize, sizeof(int64_t));
    
    
    	/* READ FILE TO BUFFER*/
    	size_t newLen = fread(sendBuff + sizeof(int64_t), sizeof(char), fileSize, file1);
    	if (newLen == 0) {
        	DieWithErr("File I/O err: fread() failed");
    	} else {
    	    	sendBuff[++newLen] = '\0'; // Just to be safe add null terminator
    	}
   	 
  	/* Send file to client */
	/*	FILL IN	  */
 	ssize_t numBytesSent = 0;
 	   
 	while (numBytesSent < sendBuffSize) {
     		numBytesSent += send(clientSock, sendBuff + numBytesSent, sendBuffSize, 0);
    	    	printf("Number of bytes sent %zu\n", numBytesSent);
    	}
    

    
    	close(clientSock);
    	free(sendBuff);

	
}

void send_list(int clientSock) {
	char *sendBuff;
	
	list_item_array *myList = get_list_items_current_dir();

	if (myList == NULL) {
		DieWithErr("get_list_items_current_dir() failed");
	}
    
    	printf("list creation test\n");
	
	
	int32_t listCount = myList->count;

	size_t sendBuffSize = sizeof(int32_t) + sizeof(list_item)*listCount;
	printf("Size of int32_t: %zu\n", sizeof(int32_t));
	printf("Size of list_item: %zu\n", sizeof(list_item));
	printf("Send Buffer Size: %zu\n", sendBuffSize);
	printf("My list size: %i\n", myList->count);
	sendBuff = malloc(sendBuffSize);
	
	memcpy(sendBuff, &listCount, sizeof(int32_t));
	int i=0;

	while (i < myList->count)	//for each list_item
	{	
		printf("I: %i\n",i);
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

	close(clientSock);
    
    
}


 
