#################################################################
##
## FILE:	Makefile
## PROJECT:	CS 3251 Project 1 - Professor Traynor
## DESCRIPTION: Compile Project 1
##
#################################################################

CC=gcc

OS := $(shell uname -s)

# Extra LDFLAGS if Solaris
ifeq ($(OS), SunOS)
	LDFLAGS=-lsocket -lnsl
    endif

all: client server server2

client: client.c
	$(CC) client.c -o nameChanger

server: server.c
	$(CC) -lcrypto server.c -o changeServer

server2: server_thread.c
	$(CC) -lcrypto server_thread.c -o threadServer
    
clean:
	    rm -f client server *.o

