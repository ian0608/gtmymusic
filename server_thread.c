/*///////////////////////////////////////////////////////////
*
* FILE:		server.c
* AUTHOR:	Warren Shenk
* PROJECT:	CS 3251 Project 1 - Professor Traynor
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
#include <fcntl.h>

#define RCVBUFSIZE 512		/* The receive buffer size */
#define SNDBUFSIZE 512		/* The send buffer size */
#define BUFSIZE 40		/* Your name can be as many as 40 chars*/
#define MDLEN 32
#define MAXBUFLEN 10000


int *file1;
char fileBuffer[MAXBUFLEN + 1];

void DieWithErr(char *errorMessage){
    printf("%s\n", errorMessage);
    exit(EXIT_FAILURE);
}

struct ThreadArgs {
    int clntSock;
};

void *ThreadMain(void *args);

/* The main function */
int main(int argc, char *argv[])
{

    int serverSock;				/* Server Socket */
    int clientSock;				/* Client Socket */
    struct sockaddr_in changeServAddr;		/* Local address */
    struct sockaddr_in changeClntAddr;		/* Client address */
    unsigned short changeServPort;		/* Server port */
    unsigned int clntLen;			/* Length of address data struct */

    char nameBuf[BUFSIZE];			/* Buff to store name from client */
    unsigned char md_value[EVP_MAX_MD_SIZE];	/* Buff to store change result */
    EVP_MD_CTX *mdctx;				/* Digest data structure declaration */
    const EVP_MD *md;				/* Digest data structure declaration */
    int md_len;					/* Digest data structure size tracking */
    
    

    static const int MAXPENDING = 5;
    
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
    
    int serverSock;				/* Server Socket */
    int clientSock;				/* Client Socket */
    struct sockaddr_in changeServAddr;		/* Local address */
    struct sockaddr_in changeClntAddr;		/* Client address */
    unsigned short changeServPort;		/* Server port */
    unsigned int clntLen;			/* Length of address data struct */
    
    char nameBuf[BUFSIZE];			/* Buff to store name from client */
    unsigned char md_value[EVP_MAX_MD_SIZE];	/* Buff to store change result */
    EVP_MD_CTX *mdctx;				/* Digest data structure declaration */
    const EVP_MD *md;				/* Digest data structure declaration */
    int md_len;					/* Digest data structure size tracking */
    int i;
  
    
    // Clear the buffers
    memset(nameBuf,0,sizeof(nameBuf));
    memset(md_value, 0, sizeof(md_value));
    
    // Setup Pthread & get the args
    pthread_detach(pthread_self());
    clientSock = ((struct ThreadArgs *) threadArgs)->clntSock;
    free(threadArgs);
    
    
    /* Extract Your Name from the packet, store in nameBuf */
    /*	FILL IN	    */
    ssize_t numBytesRecvd = recv(clientSock, nameBuf, BUFSIZE, 0);
    if (numBytesRecvd < 0)
        DieWithErr("recv() failed");
    else
        printf("Original Name: %s\n", nameBuf);
    
    
    /* OPEN FILE */
    if ((file1 = fopen("test.txt", "r")) == -1){
        fprintf(stderr, "open() failed");
        exit(EXIT_FAILURE);
    }
    
    /* READ FILE TO BUFFER*/
    size_t newLen = fread(fileBuffer, sizeof(char), MAXBUFLEN, file1);
    if (newLen == 0) {
        fputs("Error reading file", stderr);
    } else {
        fileBuffer[++newLen] = '\0'; // Just to be safe.

    }
    

    /* Send file to client */
	/*	FILL IN	    */
    
    while (numBytesRecvd > 0) {
        ssize_t numBytesSent = send(clientSock, fileBuffer, sizeof(fileBuffer), 0);
        if (numBytesSent < 0)
            DieWithErr("send() failed");
        else if (numBytesSent != sizeof(fileBuffer))
            DieWithErr("send() sent unexpected number of bytes");
        
        
        numBytesRecvd = recv(clientSock, nameBuf, MAXBUFLEN, 0);
        if (numBytesRecvd < 0)
            DieWithErr("recv() failed");
    }

    
    close(clientSock);
    
    return (NULL);
}

