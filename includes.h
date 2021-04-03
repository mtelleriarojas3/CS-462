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
//
#include <arpa/inet.h>


#define PORT 9101
#define IP "10.34.40.33" //phoenix1 ip address


#define MAX_DATA_SIZE 1024
#define MAX_FRAME_SIZE 1034
#define ACK_SIZE 6

#define current_time chrono::high_resolution_clock::now
#define time_stamp chrono::high_resolution_clock::time_point
#define elapsed_time(end, start) chrono::duration_cast<chrono::milliseconds>(end - start).count()
#define sleep_for(x) this_thread::sleep_for(chrono::milliseconds(x));

typedef unsigned char byte;

void printMD5(char *fileName);
char checksum(char *frame, int count);
int create_frame(int seq_num, char *frame, char *data, int data_size, bool eot);
void create_ack(int seq_num, char *ack, bool error);
bool read_frame(int *seq_num, char *data, int *data_size, bool *eot, char *frame);
bool read_ack(int *seq_num, bool *error, char *ack);

#endif