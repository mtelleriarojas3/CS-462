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
#define IP "10.34.40.33"//phoenix1
#define MAXLINE 1024


int callserver();

int main(){

    //INTRO MESSAGE
    cout<<"\nRUNNING CLIENT ("<< IP <<")\n";

    //SOCKET CONNECTION
    callserver();



}

int callserver(){

        int sockfd;
        char buffer[MAXLINE];
        const char *hello = "Hello from client";


    //SOCKET CREATION AND VERIFICATION
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        printf("socket creation failed...\n");
        exit(0);
    }
    
    //ASSIGN IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(PORT);

     int n, len;

    sendto(sockfd, (const char *)hello, strlen(hello),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr,
            sizeof(servaddr));
    printf("Hello message sent.\n");

    n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                (socklen_t*)&len);
    buffer[n] = '\0';
    printf("Server : %s\n", buffer);

    close(sockfd);
    return 0;

}

