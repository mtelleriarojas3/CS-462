#Makefile

CC = g++
CFLAGS = -std=c++0x -g -Wall -pthread

make: server.cpp client.cpp
        @$(CC) $(CFLAGS) -c server.cpp
        @$(CC) $(CFLAGS) -c client.cpp
        @$(CC) server.o -o server
        @$(CC) client.o -o client

clean:
        rm ./server
        rm ./client
