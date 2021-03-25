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
#include <iomanip>
#include <bits/stdc++.h>

// DECLARING GLOBAL VARIABLES
#define SA struct sockaddr 
struct sockaddr_in servaddr;
using namespace std;
#define PORT 9090 //Port NUM
#define IP "10.34.40.33"//PHOENIX1 IP ADDRESS
#define MAXLINE 1024
int n, len;

// Define functions
int clientFunction();
int callserver();
FILE *get_file(char *fileName);
int get_file_Size (FILE *fp);
void sendPackets(int sockfd, FILE *infile, int packetSize, int size);
void printMD5(char *fileName);
void goBackN();
void selectiveRepeat();
void stopAndWait();

/**
 * This function connects us to our server so we can execute the protocol!
 */
int callserver(){
    int sockfd;
    char buffer[MAXLINE];
    const char *ConnConfirm = "Client Connected...";
   
    //SOCKET CREATION AND VERIFICATION  
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed..."); 
        exit(EXIT_FAILURE);
    } else {
        cout<<"Connecting to the client...\n";
    } 
  
    memset(&servaddr, 0, sizeof(servaddr));
    
    //ASSIGN IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP);
    
    // connect the client socket to server socket 
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    }

    // Sends our confirm connection to server
    sendto(sockfd, (const char *)ConnConfirm, strlen(ConnConfirm), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr,(socklen_t*)&len);
    buffer[n] = '\0';
    cout<<buffer;
    memset(buffer, 0, sizeof(buffer));
    
    return sockfd;
}//END OF FUNCTION

/**
 * This function gets our information from the client and then calls our protocol/sends information to server
 */
int clientFunction(){
    int protocolChoice; //GBN OR SR or SaW?
    int timeoutChoice; //what the user decides for timeout interval question
    int packetSize; 
    int slidingWindowSize; 
    int errorsChoice;
    int fileSize;  
    FILE *infile;
    char fileName[MAXLINE];
  
    //connect to server
    int sockfd = callserver();
      
    // Gets our protocol choice
    cout << "Type of protocol:\n1)GBN\n2)SR\n3)SaW\n-> ";
    cin >> protocolChoice; 

    cout << "\nPacket size: ";
    cin >> packetSize;
    
    //packetSize *= 1000; //conver to KB                                                    //USE LATER PROBABLY
    
    // Gets our timeout choice
    cout<<"\nDo you wish to set a timeout, or use a ping-calculated timeout?\n";
    cout<<"1)Set a timeout\n2)Ping-calculated timeout\n-> ";
    cin >> timeoutChoice;
    
    // Gets our SWS
    cout<<"\nEnter Sliding Window Size: ";
    cin >> slidingWindowSize;
    
    // Gets our Situation error choice
    cout << "\nSituational errors:\n1)drop packets\n2)lose acks\n-> ";
    cin >> errorsChoice;

    // Gets our file name from user
    cout<<"\nEnter a file name: ";
    cin >> fileName;
    cout<<"\n";

    //HERE WE PROMPT THE USER FOR A FILE NAME AND WE GET THE FILE SIZE
    infile = get_file(fileName);//get filename and open file
    fileSize = get_file_Size(infile);
    
    // Call correct protocol and send it to the server so server knows which protocol to use
    if(protocolChoice == 1) {
        //send protocol type to server
        uint32_t protocolTypeToSend = htonl(protocolChoice);
        sendto(sockfd, &protocolTypeToSend, sizeof(protocolTypeToSend),MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        goBackN();
    } else if(protocolChoice == 2) {
        //send protocol type to server
        uint32_t protocolTypeToSend = htonl(protocolChoice);
        sendto(sockfd, &protocolTypeToSend, sizeof(protocolTypeToSend),MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        selectiveRepeat();
    } else if(protocolChoice == 3) {
        //send protocol type to server
        uint32_t protocolTypeToSend = htonl(protocolChoice);
        sendto(sockfd, &protocolTypeToSend, sizeof(protocolTypeToSend),MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        stopAndWait();
    } else {
        cout<<"The protocol you chose was shit, ending program";
    }
    
    //send total file size to server
    uint32_t totalSizeTemp = htonl(fileSize);//
    sendto(sockfd, &totalSizeTemp, sizeof(totalSizeTemp),MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
     
	  //send packet size to server
    uint32_t tempPacketSize = htonl(packetSize);// 
    sendto(sockfd, &tempPacketSize, sizeof(tempPacketSize),MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    	
    // Close our file and print the MD5 of said file
    fclose(infile);
    printMD5(fileName); 
    return sockfd;
}

// GBN protocol
void goBackN() {
    cout<<"we are in goBackN";
}

// SR protocol
void selectiveRepeat() {
    cout<<"we are in SR";
}

// SaW protocol
void stopAndWait() {
    cout<<"we are in stop and wait";
}

// Gets our file name and then opens our file
FILE *get_file(char *fileName) {
    const char *method = "rb";
    FILE *fp = NULL;
    while (!strlen(fileName) || !fp) {        
        fp = fopen(fileName, method);
    }
    return fp;
}

// Gets the total size of our file
int get_file_Size (FILE *fp) {
    if (fseek(fp, 0, SEEK_END)) {
        printf("Error: Unable to find end of file\n");
        exit(1);
    }
    int size = ftell(fp);
    // Back to beginning of file;
    fseek(fp, 0, SEEK_SET);
    return size;
}

// This prints the MD5 of our file with a systen call
void printMD5(char *fileName){
	  string filetbs = fileName;
	  string md5file = "md5sum "+filetbs;
	  const char *actualmd5 = md5file.c_str();
	  cout<<"\nMD5:\n";
	  system(actualmd5);
	  cout<<"\n";
}

// Our main method. Sets up server connection then calls protocol
int main(){
    //INTRO MESSAGE
    cout<<"\nRUNNING CLIENT\n";
    //SOCKET CONNECTION
    int clientSocket = clientFunction();
    close(clientSocket);//CLOSE CONNECTION
}//END OF MAIN



