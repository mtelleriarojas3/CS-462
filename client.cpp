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
#define SA struct sockaddr 
struct sockaddr_in servaddr;
using namespace std;
#define PORT 9090 
#define IP "10.34.40.33"//PHOENIX1 IP ADDRESS
#define MAXLINE 1024


int callserver();
int clientFunction();

int main(){

    //INTRO MESSAGE
    cout<<"\nRUNNING CLIENT\n";

    //SOCKET CONNECTION
   int clientSocket =  clientFunction();
       	

   close(clientSocket);//CLOSE CONNECTION

}//END OF MAIN

int callserver(){

	int sockfd;

    //SOCKET CREATION AND VERIFICATION  
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        cout<<"socket creation failed...\n"; 
        exit(0); 
    }else{
	cout<<"Connected to server successfully!\n\n";
    } 
  
    //ASSIGN IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(PORT);
  
    // int n, len;

    //sendto(sockfd, (const char *)hello, strlen(hello),
    //MSG_CONFIRM, (const struct sockaddr *) &servaddr,sizeof(servaddr));

    //cout<<"Hello message sent.\n\n";

    //n = recvfrom(sockfd, (char *)buffer, MAXLINE,
    //MSG_WAITALL, (struct sockaddr *) &servaddr,(socklen_t*)&len);
    //buffer[n] = '\0';
   // cout<<"Server : "<<buffer <<"\n\n";
    
    return sockfd;

}//END OF FUNCTION


//I STILL DON'T KNOW WHAT I WILL DO WITH THIS
int clientFunction(){

	int sockfd = callserver();

	char buff[MAXLINE];
        int n;
	int len;

        for (;;) {
        
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
       
       //SEND DATA TO THE SERVER	
       sendto(sockfd, (const char *)buff, strlen(buff),
       MSG_CONFIRM, (const struct sockaddr *) &servaddr,sizeof(servaddr));
       
       memset(buff, 0, sizeof(buff));
        
       //DATA RECEIVED FROM THE SERVER	
       n = recvfrom(sockfd, (char *)buff, MAXLINE,
       MSG_WAITALL, (struct sockaddr *) &servaddr,(socklen_t*)&len);
       buff[n] = '\0';
       cout<<"Server : "<<buff<<"\n\n";


        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }


return sockfd;
}
