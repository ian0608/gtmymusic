/*///////////////////////////////////////////////////////////
 *
 * FILE:		client.c
 * AUTHOR:	Warren Shenk and Ian Stainbrook
 * PROJECT:	CS 3251 Project 2 - Professor Traynor
 * DESCRIPTION:	Network Client Code
 *
 *////////////////////////////////////////////////////////////

/* Included libraries */

#include <stdio.h>		    /* for printf() and fprintf() */
#include <sys/socket.h>		    /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>		    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <openssl/evp.h>	    /* for OpenSSL EVP digest libraries/SHA256 */
#include <fcntl.h>
#include <stdint.h>
#include "gtmymusic.h"

/* Constants */
#define RCVBUFSIZE 10000	    /* The receive buffer size */
#define SNDBUFSIZE 10000		    /* The send buffer size */
#define MDLEN 32



int f1;

void DieWithErr(char *errorMessage){
    printf("%s\n", errorMessage);
    exit(EXIT_FAILURE);
}

/* The main function */
int main(int argc, char *argv[])
{
    
    int clientSock;		    /* socket descriptor */
    struct sockaddr_in serv_addr;   /* The server address */
    
    char sndBuf[SNDBUFSIZE];	    /* Send Buffer */
    unsigned char rcvBuf[RCVBUFSIZE];	    /* Receive Buffer */
    unsigned short servPort = 6079;
    
    //int i;			    /* Counter Value */
    
    // Clear the buffers
    memset(sndBuf, 0, sizeof(sndBuf));
    memset(rcvBuf, 0, sizeof(rcvBuf));

    
    /* Create a new TCP socket*/
    /*	    FILL IN	*/
    clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSock < 0)
        DieWithErr("socket() failed");
    
    /* Construct the server address structure */
    /*	    FILL IN	 */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("130.207.114.22");
    serv_addr.sin_port = htons(servPort);
    
    /* Establish connecction to the server */
    /*	    FILL IN	 */
    if (connect(clientSock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        DieWithErr("connect() failed");
    
    
    /* Send the string to the server */
    /*	    FILL IN	 */

	char* listCmd = "LIST";
    size_t cmdLen = strlen(listCmd);
    
    ssize_t numBytes = send(clientSock, listCmd, cmdLen + 1, 0);
    if (numBytes < 0)
        DieWithErr("send() failed");
    else if (numBytes != cmdLen + 1)
        DieWithErr("send() sent unexpected number of bytes");
    
    
    /* Receive and print response from the server */
    /*	    FILL IN	 */
    unsigned int bytesRec = 0;
	int32_t *numItems = NULL;
	
	numBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
	if (numBytes < 0)
            DieWithErr("recv() failed");
        else if (numBytes == 0)
            DieWithErr("recv() connection closed prematurely");
	numItems = (int32_t *)rcvBuf;
	printf("Received %u items\n", *numItems);
	bytesRec += numBytes;
	
    while (bytesRec < sizeof(int32_t) + (*numItems)*sizeof(list_item))
	{
		numBytes = recv(clientSock, rcvBuf + bytesRec, RCVBUFSIZE - bytesRec, 0);
        if (numBytes < 0)
            DieWithErr("recv() failed");
        else if (numBytes == 0)
            DieWithErr("recv() connection closed prematurely");
        
        bytesRec += numBytes;
        
    }

	list_item_array *mostRecentList;
	if(init_list_item_array(&mostRecentList) < 0)
		return -1;
	int k;
	for(k=0; k<*numItems; k++)
	{
		list_item *currentPtr = (list_item *)(rcvBuf+sizeof(int32_t)+(k*sizeof(list_item)));
		if(incr_size_list_item_array(&mostRecentList) < 0)
			return -1;
		memcpy(mostRecentList->items[mostRecentList->count-1]->hash, currentPtr->hash, MD5_DIGEST_LENGTH);
		memcpy(mostRecentList->items[mostRecentList->count-1]->filename, currentPtr->filename, strlen(currentPtr->filename)+1);
	}

	k=0;
	while (k < mostRecentList->count)	//for each list_item
	{
		printf("%s\n", mostRecentList->items[k]->filename);	//print the filename
		int j;
		for (j=0; j < MD5_DIGEST_LENGTH; j++)				//and print the hash
			printf("%02x", mostRecentList->items[k]->hash[j]);
		printf("\n");
		k++;
	}    
    
    
    /*
    for(i = 0; i < RCVBUFSIZE; i++)
        printf("%c", rcvBuf[i]);
    printf("\n");
    */
    
    return 0; 
}

