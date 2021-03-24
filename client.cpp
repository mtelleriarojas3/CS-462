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

#define SA struct sockaddr 
struct sockaddr_in servaddr;
using namespace std;
#define PORT 9090 
#define IP "10.34.40.33"//PHOENIX1 IP ADDRESS
#define MAXLINE 1024
int n, len;

int clientFunction();
int callserver();
FILE *get_file(char *fileName);
int get_file_Size (FILE *fp);
void sendPackets(int sockfd, FILE *infile, int packetSize, int size);
void printMD5(char *fileName);

int callserver(){

	 int sockfd;
   /////////
     char buffer[MAXLINE];
     const char *ConnConfirm = "Client Connected...";
   //////////
   
    //SOCKET CREATION AND VERIFICATION  
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed..."); 
        exit(EXIT_FAILURE);
    }else{
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
    
    /////////
        sendto(sockfd, (const char *)ConnConfirm, strlen(ConnConfirm),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
    
          
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                (socklen_t*)&len);
    buffer[n] = '\0';
    cout<<buffer;
    memset(buffer, 0, sizeof(buffer));
    //////////////
    
    return sockfd;
    

}//END OF FUNCTION

int clientFunction(){
     
  int protocolChoice; //GBN OR SR?
  int timeoutChoice; //what the user decides for timeout interval question
  int packetSize; 
  int slidingWindowSize; 
  int errorsChoice;
  int fileSize;  
  FILE *infile;
  char fileName[MAXLINE];
  
  
  //connect to server
  int sockfd = callserver();
      
    //THIS IS WHERE WE PROMPT THE USER FOR INPUT IN CASE HE WANTS TO ENTER HIS OWN VALUES.
    cout << "Type of protocol:\n1)GBN\n2)SR\n-> ";
    cin >> protocolChoice; 

    cout << "\nPacket size: ";
    cin >> packetSize;
    
    //packetSize *= 1000; //conver to KB
    
    cout<<"\nDo you wish to set a timeout, or use a ping-calculated timeout?\n";
    cout<<"1)Set a timeout\n2)Ping-calculated timeout\n-> ";
    cin >> timeoutChoice;
    
    cout<<"\nEnter Sliding Window Size: ";
    cin >> slidingWindowSize;
    
    //range of sequence numbers

    cout << "\nSituational errors:\n1)drop packets\n2)lose acks\n-> ";
    cin >> errorsChoice;

    cout<<"\nEnter a file name: ";
    cin >> fileName;

    cout<<"\n";

    //HERE WE PROMPT THE USER FOR A FILE NAME AND WE GET THE FILE SIZE
    infile = get_file(fileName);//get filename and open file
    fileSize = get_file_Size(infile);
    sendPackets(sockfd, infile, packetSize, fileSize);
    	
    fclose(infile);
    printMD5(fileName); 
    

 return sockfd;
}


void sendPackets(int sockfd, FILE *infile, int packetSize, int size){

unsigned char *fileBuff;
    
    //grab sufficient memory for the buffer to hold the text
    fileBuff = (unsigned char*)calloc(packetSize, sizeof(unsigned char));
    //bzero(fileBuff, sizeof(fileBuff));
    
    //send total file size to server
    uint32_t totalSizeTemp = htonl(size);//
    sendto(sockfd, &totalSizeTemp, sizeof(totalSizeTemp),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
     
     
	  //send packet size to server
    uint32_t tempPacketSize = htonl(packetSize);// 
    sendto(sockfd, &tempPacketSize, sizeof(tempPacketSize),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
    //cout<<"\nTemp packet size sent: "<< packetSize <<"\n";

    int packet = 0;
    
 	  //copy all the text into the buffer
	  while(fread(fileBuff, sizeof(char), packetSize, infile)){
    cout<<"Sent packet# " <<dec << packet<<"\n";
    
    packet++;

    //write(sockfd, fileBuff, packetSize);
    sendto(sockfd, (unsigned char *)fileBuff, sizeof(packetSize),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
     bzero(fileBuff, packetSize);        
     
   }//end while
   //fclose(infile);
   


}//END OF METHOD



// Get file name and open file
FILE *get_file(char *fileName) {
    const char *method = "rb";

    FILE *fp = NULL;
    while (!strlen(fileName) || !fp) {        
        fp = fopen(fileName, method);
    }
    return fp;
}

int get_file_Size (FILE *fp) {
    // Need size of file for dynamic memory allocation
    if (fseek(fp, 0, SEEK_END)) {
        printf("Error: Unable to find end of file\n");
        exit(1);
    }
    int size = ftell(fp);
    //printf("File size: %li (bytes)\n", size);
    // Back to beginning of file;
    fseek(fp, 0, SEEK_SET);
    return size;
}

//PRINT MD5
void printMD5(char *fileName){
  
	string filetbs = fileName;
	string md5file = "md5sum "+filetbs;
	const char *actualmd5 = md5file.c_str();
	
	cout<<"\nMD5:\n";

	system(actualmd5);
	cout<<"\n";
}

//MAIN FUNCTION
int main(){

    //INTRO MESSAGE
    cout<<"\nRUNNING CLIENT\n";

    //SOCKET CONNECTION
  int clientSocket = clientFunction();//Set up client Socket
  
  
       	
   close(clientSocket);//CLOSE CONNECTION

}//END OF MAIN



