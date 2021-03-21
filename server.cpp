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
int serverSocketAccept(int sockfd);

//MAIN FUNCTION
int main(){

  int timeAns; //what the user decides for timeout interval question

  //INTRO MESSAGE
  cout<<"\nRUNNING SERVER ("<<IP<<")\n\n";

  //SOCKET CONNECTION
  int serverSocket = setupServerSocket();
  serverSocketAccept(serverSocket);
  close(serverSocket);



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



int serverSocketAccept(int sockfd){
    char buffer[MAXLINE];
    const char *hello = "Hello from server";
    int n, connfd;

    len = sizeof(cliaddr);  //len is value/resuslt

    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);

    connfd = sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);

    if (connfd < 0) {
        printf("server acccept failed...\n");
        exit(0);
    }else{
        cout<<"Client Connected: ("<<inet_ntoa(cliaddr.sin_addr)<<")\n\n";
    }

   buffer[n] = '\0';
    printf("Client : %s\n\n", buffer);

    printf("Hello message sent.\n\n");

    return connfd;

}//END OF METHOD
