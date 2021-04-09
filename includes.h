#ifndef INCLUDES_H
#define INCLUDES_H

#include <chrono>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <thread>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <mutex>
#include <unistd.h>
#include <sys/time.h>
#include <chrono>
#include <arpa/inet.h>


#define PORT 9295
#define IP "10.34.40.33" //phoenix1 ip address

#define ACKSIZE 6
#define MAXLINE 1024
#define MAXFRAME 1034

#define current_time chrono::high_resolution_clock::now
#define time_stamp chrono::high_resolution_clock::time_point
#define elapsed_time(end, start) chrono::duration_cast<chrono::milliseconds>(end - start).count()
#define sleep_for(x) this_thread::sleep_for(chrono::milliseconds(x));

typedef unsigned char byte;

void printMD5(char *fileName);
bool read_frame(int *seq_num, char *data, int *data_size, bool *eot, char *frame);
int create_frame(int seq_num, char *frame, char *data, int data_size, bool eot);
bool read_ack(int *seq_num, bool *error, char *ack);
void create_ack(int seq_num, char *ack, bool error);
char checksum(char *frame, int count);


#endif