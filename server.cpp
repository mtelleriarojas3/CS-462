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
#define PORT 9090
#define MAXLINE 1024
#define SA struct sockaddr
struct sockaddr_in servaddr, cliaddr;

//DECLARING FUNCTIONS
int setupServerSocket();
int serverSocketAccept(int sockfd);

//MAIN FUNCTION
int main(){

  //SOCKET CONNECTION
  int serverSocket = setupServerSocket();
  serverSocketAccept(serverSocket);
  close(serverSocket);


}

int setupServerSocket(){

   int sockfd;

    // CREATING SOCKET FILE DESCRIPTOR
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }else{
        printf("Socket successfully created..\n");
        }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // FILLING SERVER INFORMATION
    servaddr.sin_family    = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("10.34.40.33");//PHOENIX1 FOR SERVER
    servaddr.sin_port = htons(PORT);

     // BIND THE SOCKET WITH THE SERVER ADDRESS
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }else{
        printf("Socket successfully binded..\n");
    }

return sockfd;


}//END OF METHOD



int serverSocketAccept(int sockfd){
    char buffer[MAXLINE];
    const char *hello = "Hello from server";
    int len, n, connfd;

    len = sizeof(cliaddr);  //len is value/resuslt

    n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                (socklen_t*)&len);
    buffer[n] = '\0';
    printf("Client : %s\n", buffer);
    sendto(sockfd, (const char *)hello, strlen(hello),
    connfd = MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
    if (connfd < 0) {
        printf("server acccept failed...\n");
        exit(0);
    }else{
        printf("server acccepted the client... server is listenning\n");
    }
    printf("Hello message sent.\n");

    return connfd;
    
}//END OF METHOD

