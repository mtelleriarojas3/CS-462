#include "includes.h"
#define SA struct sockaddr
int n, len;
struct sockaddr_in servaddr, cliaddr;

// Define functions
int setupServerSocket();
int receivePackets();
void goBackN();
void selectiveRepeat();
void stopAndWait();

/**
 * This function sets up our server sockets to listen from the client
 */
int setupServerSocket(){
    int sockfd;
    char buffer[MAXLINE];
    const char *ConnConfirm = "Connected to server successfully!\n";
    int opt = 1;
    
    // CREATING SOCKET FILE DESCRIPTOR 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }/*else{
         //cout<<"Socket successfully created..\n"; 
    }*/
 
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
    
    //We have no clients yet
    cout<<"No clients connected at the moment...\n";
    
    len = sizeof(cliaddr);  //len is value/result

    // Receive connection confirmation from client
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
    buffer[n] = '\0';
    printf("%s\n", buffer);

    sendto(sockfd, (const char *)ConnConfirm, strlen(ConnConfirm), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
    memset(buffer, 0, sizeof(buffer));
    return sockfd;
}//END OF METHOD

/**
 * This is where we start to receive information from client. It calls the correct protocol
 */
int receivePackets(){
    int packetSize;
    int totalFileSize;
    int protocolType;
    uint32_t totalSizeTemp;
    uint32_t tempPacketSize;
    uint32_t tempProtocolType;
    const char *outputFileName = "sample.txt"; 
    int sockfd = setupServerSocket();

    printf("\n");
  
    //grab protocol type from client
    recvfrom(sockfd, &protocolType, sizeof(protocolType), MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
    
 
    if(protocolType == 1) {
    
        goBackN();
        
    } else if(protocolType == 2) {
    
        selectiveRepeat();
        
    } else if(protocolType == 3) {
    
        stopAndWait();
        
    }
  
	  //grab the total size of the file
    recvfrom(sockfd, &totalSizeTemp, sizeof(totalSizeTemp), MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
    totalFileSize = ntohl(totalSizeTemp);
    cout<<"TOTAL SIZE: "<<totalFileSize<<"\n"; 
  
	  //grab size of the packet that will be sent here
    recvfrom(sockfd, &tempPacketSize, sizeof(tempPacketSize), MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
    packetSize = ntohl(tempPacketSize);
	  cout<<"PACKET SIZE: "<<packetSize<<"\n"; 

    return sockfd;
}//END OF METHOD

void goBackN() {
    cout<<"we are in go back n\n";
}

void selectiveRepeat() {
    cout<<"we are in selective repeat\n";
}

void stopAndWait() {
    cout<<"we are in stop and wait\n";
}



//MAIN FUNCTION
int main(){
  //INTRO MESSAGE	
  cout<<"\nRUNNING SERVER ("<<IP<<")\n\n";
  //SOCKET CONNECTION	
  int serverSocket = receivePackets();
  close(serverSocket);//CLOSE CONNECTION
}//END OF MAIN