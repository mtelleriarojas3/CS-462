#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>//added this
#include <unistd.h>//and this
#include <netinet/in.h>
using namespace std;
#define SA struct sockaddr
struct sockaddr_in servaddr, cli;
#define PORT     9090
#define MAXLINE 1024


int callserver();

int main(){

    int sockfd = callserver();

}

int callserver(){
        int sockfd;
        char buffer[MAXLINE];
        const char *hello = "Hello from client";


    // socket create and varification
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        printf("socket creation failed...\n");
        exit(0);
    }

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("10.34.40.33");//running phoenix2 for client
    servaddr.sin_port = htons(PORT);//PORT

     int n, len;

    sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &servaddr,
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
