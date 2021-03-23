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
#define FRAMESIZE 1500
#define MAXTIMEOUT 1000    //MAX TIMEOUT
#define PORT 9090
#define IP "10.34.40.33" //phoenix1 ip address
#define MAXLINE 1024 
#define SA struct sockaddr
int sockfd, timeOut, len; 
struct sockaddr_in servaddr, cliaddr;
using namespace std;

//DECLARING FUNCTIONS
int setupServerSocket();
void serverSocketAccept(int sockfd);


int serverFunction(int sockfd);////


//MAIN FUNCTION
int main(){

  int timeAns; //what the user decides for timeout interval question

  //INTRO MESSAGE	
  cout<<"\nRUNNING SERVER ("<<IP<<")\n\n";

  //SOCKET CONNECTION	
  int serverSocket = setupServerSocket();
  serverFunction(serverSocket);
  //close(serverSocket); 
  


  //PROMPTING USER FOR TIMEOUT INTERVAL
    cout<<"\nDo you wish to set a timeout, or use a ping-calculated timeout?\n\n";
    cout<<"1)Set a timeout\n2)Ping-calculated timeout\n";
    
    while (scanf("%i", &timeAns) != 1 && ((timeAns != 1) && (timeAns != 2))) {
        getchar();
    }
  
    if(timeAns == 1){//IF THE USER WANTS TO SET A TIMEOUT
	   
	    cout<<"\nInput a timeout value (In Microseconds): ";
	    while (scanf("%i", &timeAns) && ((timeAns <= 0) || (timeAns > MAXTIMEOUT))) {
            getchar(); 

	    }

    } else if(timeAns == 2){
	   //timeAns = calculateCustomTimeout(td);
        printf("\nCalculated timeout using given packet size of %d bytes is: %d microseconds\n", FRAMESIZE, timeAns);
    }


    //ASSIGN TIMEOUT
    timeOut = timeAns;

close(serverSocket);//CLOSE CONNECTION

}//END OF MAIN

int setupServerSocket(){
   
  

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
    servaddr.sin_family    = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(PORT);

     // BIND THE SOCKET WITH THE SERVER ADDRESS
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }//else{
     //   printf("Socket successfully binded..\n");
    //}
    cout<<"No clients connected at the moment...\n";

return sockfd;


}//END OF METHOD


int serverFunction(int sockfd){
	


	char buff[MAXLINE]; 
        int n;
       	int len = sizeof(cliaddr);

		
        
	// infinite loop for chat 
        for (;;) { 
 	 
        // read the message from client and copy it in buffer 
        //read(sockfd, buff, sizeof(buff));
	n = recvfrom(sockfd, (char *)buff, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);

	
        // print buffer which contains the client contents 
        printf("From client: %s\t To client : ", buff); 
        
	memset(buff, 0, sizeof(buff)); 
        n = 0; 
        // copy server message in the buffer 
        while ((buff[n++] = getchar()) != '\n') 
            ; 
  
        // and send that buffer to client 
        
  	sendto(sockfd, (const char *)buff, strlen(buff), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
	 
	buff[n] = '\0';
        printf("Client : %s\n\n", buff);

        // if msg contains "Exit" then server exit and chat ended. 
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    }
	return sockfd;
}





//ideas: gotta make two different methods for server socket, one to send and another to receive, that way the client won't disconnect, look at ref.

