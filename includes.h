#ifndef ___INCLUDES_H___
#define ___INCLUDES_H___

//HERE ARE THE INCLUDES
#include <netdb.h> 
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <chrono>
using namespace std;
using namespace std::chrono;

//PACKET INFO
#define FMESSAGE_LENGTH 1492       // Frame message length
#define CHECK_SUM 4          // Checksum length
#define FRAME_SIZE 1500       // Total frame size
#define HEADER_LENGTH 4          // Frame header length

//CONNECTION STUFF
#define PORT 9101
#define IP "10.34.40.33" //phoenix1 ip address
#define MAXLINE 1024 
#define MAXTIMEOUT 1000    //MAX TIMEOUT
//OTHER STUFF

//FLAGS FOR FRAME
#define FLAG_END_DATA 0x4
#define FLAG_CLIENT_JOIN 0x8
#define FLAG_SERVER_RTT 0x10
#define FLAG_CLIENT_EXIT 0x20
#define FLAG_ACK_VALID 0x1
#define FLAG_HAS_DATA 0x2
#define FLAG_CLIENT_RTT 0x40
#define FLAG_RT_DATA 0x80

typedef unsigned int uint32;

//WINDOW DESIGN
const uint32 SERVERWINDOWSIZE = 8;   // Server window size
const uint32 RECIEVINGWINDOWSIZE = 8;   // Recieving window size
const uint32 SEQUENCENUMBER = 32;   // Sequence Number

void printMD5(char *fileName);
FILE *get_file(char *fileName);
int get_file_Size (FILE *fp);

#endif
