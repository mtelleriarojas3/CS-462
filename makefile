#Makefile

CC = g++
CFLAGS = -std=c++0x -g -Wall -pthread

all: client server

client: client.o includes.o 
	$(CC) $(CFLAGS) -o client client.o includes.o 

server: server.o includes.o 
	$(CC) $(CFLAGS) -o server server.o includes.o 

client.o: client.cpp
	$(CC) $(CFLAGS) -c client.cpp

server.o: server.cpp
	$(CC) $(CFLAGS) -c server.cpp

includes.o: includes.cpp includes.h
	$(CC) $(CFLAGS) -c includes.cpp

clean:
	rm ./server
	rm ./client
