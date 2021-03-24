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

//DECLARING GLOBAL VARIABLES
#define FRAMESIZE 1500    //FRAME SIZE
#define MAXTIMEOUT 1000    //MAX TIMEOUT
#define PORT 9090
#define IP "10.34.40.33" //phoenix1 ip address
#define MAXLINE 1024 
#define SA struct sockaddr

int sockfd, timeOut, len, n;; 
struct sockaddr_in servaddr, cliaddr;
using namespace std;

int setupServerSocket();
void userPrompt(int sockfd);

///////////////////////////////////////////////////////////////////////////////////////////////

int setupServerSocket(){
    char buffer[MAXLINE];
    const char *ConnConfirm = "Connected to server successfully!\n";
    int opt = 1;
    
    // CREATING SOCKET FILE DESCRIPTOR 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }else{
         //cout<<"Socket successfully created..\n"; 
    }
 
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // FILLING SERVER INFORMATION
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP);


     // BIND THE SOCKET WITH THE SERVER ADDRESS
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    cout<<"No clients connected at the moment...\n";
    
    
    
    len = sizeof(cliaddr);  //len is value/resuslt 

    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                (socklen_t*)&len);
    buffer[n] = '\0';
    cout<<buffer;
    
    
    sendto(sockfd, (const char *)ConnConfirm, strlen(ConnConfirm), 
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
            len);
    
    
    
    

    return sockfd;


}//END OF METHOD



//THIS IS THE "MENU" FOR THE USER
void userPrompt(int sockfd) {

  //DECLARING VARIABLES 
 // const char *prompts;
  int protocolChoice; //GBN OR SR?
  int timeAns; //what the user decides for timeout interval question
  int packetSize; 
  int slidingWindowSize; 

  
  while (scanf("%i", &protocolChoice) != 1 && ((protocolChoice != 1) && (protocolChoice != 2))) {
        getchar();
    }
  
   //PROMPTING USER FOR SIZE OF PACKET
   cout<<"\nEnter Packet Size: ";
   scanf("%i", &packetSize);
   
  
  //PROMPTING USER FOR TIMEOUT INTERVAL OR PING CALCULATED
    cout<<"\nDo you wish to set a timeout, or use a ping-calculated timeout?\n\n";
    cout<<"1)Set a timeout\n2)Ping-calculated timeout\n";
    
    while (scanf("%i", &timeAns) != 1 && ((timeAns != 1) && (timeAns != 2))) {
        getchar();
    }
  
    if(timeAns == 1){//if the user wants to set a timeout
	   
	    cout<<"\nInput a timeout value (In Microseconds): ";
	    while (scanf("%i", &timeAns) && ((timeAns <= 0) || (timeAns > MAXTIMEOUT))) {
            getchar(); 

	    }

    } else if(timeAns == 2){
	   //timeAns = calculateCustomTimeout(td);
        printf("\nCalculated timeout using given packet size of %d bytes is: %d microseconds\n", FRAMESIZE, timeAns);
    }
    
    //assing timeout
    timeOut = timeAns;
    
    //PROMPTING USER FOR SIZE OF SLIDING WINDOW
    cout<<"\nEnter Sliding Window Size: ";
    scanf("%i", &slidingWindowSize);
    
    //PROMPTING USER FOR RANGE OF SEQUENCE NUMBERS
    //no idea what to do here yet
    
    //PROMPTING USER FOR SITUATIONAL ERRORS 
    //(none, randomly generated, user specified i.e., drop packets 2 4 5, lose acks 11, etc
    
    

}



//MAIN FUNCTION
int main(){

  //INTRO MESSAGE	
  cout<<"\nRUNNING SERVER ("<<IP<<")\n\n";

  //SOCKET CONNECTION	
  int serverSocket = setupServerSocket();
  userPrompt(serverSocket);
  
  //serverFunction(serverSocket);
  //close(serverSocket); 
  

close(serverSocket);//CLOSE CONNECTION

}//END OF MAIN

//ideas: gotta make two different methods for server socket, one to send and another to receive, that way the client won't disconnect, look at ref.

