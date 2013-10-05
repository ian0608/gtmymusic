/*///////////////////////////////////////////////////////////
 *
 * FILE:		client.c
 * AUTHOR:	Warren Shenk
 * PROJECT:	CS 3251 Project 1 - Professor Traynor
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
    
    char *studentName;		    /* Your Name */
    
    char sndBuf[SNDBUFSIZE];	    /* Send Buffer */
    unsigned char rcvBuf[RCVBUFSIZE];	    /* Receive Buffer */
    unsigned short servPort = 6079;
    
    int i;			    /* Counter Value */
    
    
    // Clear the buffers
    memset(sndBuf, 0, sizeof(sndBuf));
    memset(rcvBuf, 0, sizeof(rcvBuf));
    
    
    /* Get the Student Name from the command line */
    if (argc != 2)
    {
        printf("Incorrect input format. The correct format is:\n\tnameChanger your_name\n");
        exit(1);
    }
    studentName = argv[1];
    //servPort = (unsigned short *) atoi(argv[2]);
    memset(&sndBuf, 0, RCVBUFSIZE);
    memset(&rcvBuf, 0, RCVBUFSIZE);
    
    /* Create a new TCP socket*/
    /*	    FILL IN	*/
    clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSock < 0)
        DieWithErr("socket() failed");
    
    /* Construct the server address structure */
    /*	    FILL IN	 */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(servPort);
    
    /* Establish connecction to the server */
    /*	    FILL IN	 */
    if (connect(clientSock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        DieWithErr("connect() failed");
    
    
    /* Send the string to the server */
    /*	    FILL IN	 */
    size_t nameLen = strlen(studentName);
    
    ssize_t numBytes = send(clientSock, studentName, nameLen, 0);
    if (numBytes < 0)
        DieWithErr("send() failed");
    else if (numBytes != nameLen)
        DieWithErr("send() sent unexpected number of bytes");
    
    
    /* Receive and print response from the server */
    /*	    FILL IN	 */
    unsigned int totalBytesRecieved = 0;
    fputs("Recieved:\n", stdout);
    while (totalBytesRecieved < nameLen) {
        
        numBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
        if (numBytes < 0)
            DieWithErr("recv() failed");
        else if (numBytes == 0)
            DieWithErr("recv() connection closed prematurely");
        
        totalBytesRecieved += numBytes;
        rcvBuf[numBytes] = '\0';
        
    }
    
    if ((f1 = creat("recv.txt",
                    S_IRUSR | S_IWUSR)) == -1) {
        DieWithErr("Can't create file");
    }
    if (write(f1, rcvBuf, strlen(rcvBuf)) == -1) {
		DieWithErr("Can't write file");
	}
    
    
    
    
    for(i = 0; i < RCVBUFSIZE; i++)
        printf("%c", rcvBuf[i]);
    printf("\n");
    
    
    
    
    
    return 0; 
}

