#################################################################
##
## FILE:	Makefile
## PROJECT:	CS 3251 Project 2 - Professor Traynor
## DESCRIPTION: Compile Project 2
##
#################################################################

CC=gcc

OS := $(shell uname -s)

# Extra LDFLAGS if Solaris
ifeq ($(OS), SunOS)
	LDFLAGS=-lsocket -lnsl
    endif

all: client server 

client: client.c utils.c
	$(CC) -Wall utils.c client.c -o musicClient -lcrypto

server: server_thread.c
	$(CC) server_thread.c -o threadServer
    
clean:
	    rm -f musicClient *.o

