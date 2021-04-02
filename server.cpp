#include "includes.h"
#define SA struct sockaddr
int n, len;

typedef struct packet{
    char data[1024];
}Packet;

typedef struct frame{
    int frame_kind; //ACK:0, SEQ:1 FIN:2
    int sq_no;
    int ack;
    Packet packet;
}Frame;

struct sockaddr_in servaddr, cliaddr;

// Define functions
int setupServerSocket();
int receivePackets();
void goBackN();
void selectiveRepeat();
void stopAndWait();
char checksum(char *frame, int count);

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
    servaddr.sin_port = htons(9101);
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
    int windowSize;
    uint32_t totalSizeTemp;
    uint32_t tempPacketSize;
    uint32_t tempWindowSize;
    //uint32_t tempProtocolType;
    //const char *outputFileName = "sample.txt"; 
    int sockfd = setupServerSocket();

    printf("\n");
  
    //grab protocol type from client
    recvfrom(sockfd, &protocolType, sizeof(protocolType), MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
  
	  //grab the total size of the file
    recvfrom(sockfd, &totalSizeTemp, sizeof(totalSizeTemp), MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
    totalFileSize = ntohl(totalSizeTemp);
    cout<<"TOTAL SIZE: "<<totalFileSize<<"\n"; 
  
	  //grab size of the packet that will be sent here
    recvfrom(sockfd, &tempPacketSize, sizeof(tempPacketSize), MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
    packetSize = ntohl(tempPacketSize);
	  cout<<"PACKET SIZE: "<<packetSize<<"\n"; 
     
    //grab window size
    recvfrom(sockfd, &tempWindowSize, sizeof(tempWindowSize), MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
    windowSize = ntohl(tempWindowSize);
	  cout<<"SLIDING WINDOW SIZE: "<<windowSize<<"\n";
    
    //Selective Repeat
    if(protocolType == 2) {
        
    }
    
    //Selective Repeat
    if(protocolType == 2) {
        
    }

    //SaW Protocol
    if(protocolType == 3) {
        //Variables
        char recvBuff[packetSize];
        int len = strlen(recvBuff);
        memset(recvBuff, '0', sizeof(recvBuff));
        int frame_id=0;
	      Frame frame_recv;
	      Frame frame_send;
        Frame frame_checksum;
        int packetNum = 0;
        int ackNum = 0;
       	ofstream fileReceived;
        int run = 1;
        int reTranPackets = 0;
        
        FILE *fp;
        fp = fopen("Sample.txt", "ab");
        if(NULL == fp) {
            printf("Error opening file");
            return 1;
        }
        
        //Loop indefinitely
        while(run) {
            int f_recv_size = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0, (struct sockaddr*)&cliaddr,(socklen_t*)&len);
		        if (f_recv_size > 0 && frame_recv.frame_kind == 1 && frame_recv.sq_no == frame_id){
    			      printf("Packet %d received\n", packetNum);
                std::string test;
                test += frame_recv.packet.data;                  
                fwrite(test.c_str(), 1, test.size(), fp); 
                int f_checksum_size = recvfrom(sockfd, &frame_checksum, sizeof(Frame), 0, (struct sockaddr*)&cliaddr,(socklen_t*)&len);
                if(f_checksum_size > 0 && frame_checksum.frame_kind == 1 && frame_checksum.sq_no == frame_id) {
                      char test1 = checksum(frame_checksum.packet.data, f_checksum_size);
                      char test2 = checksum(frame_recv.packet.data, f_recv_size);
                      if(test1 == test2) {
                          cout << "Checksum OK \n";
                      } else {
                          cout << "Checksum failed \n";
                      }
                }
    			      frame_send.sq_no = 0;
    			      frame_send.frame_kind = 0;
    			      frame_send.ack = frame_recv.sq_no + 1;
                sendto(sockfd, &frame_send, sizeof(frame_send), 0, (struct sockaddr*)&cliaddr, sizeof(servaddr));
    			      printf("Ack %d sent\n", ackNum);
		        } else {
			          printf("Packet Not Received\n");
                reTranPackets++;
                run = 0;
		        }
            ackNum++;
            packetNum++;
 		        frame_id++;
            bzero(frame_recv.packet.data, packetSize);
        }
        packetNum-=2;
        reTranPackets--;
        cout << "Last packet seq# received: " << packetNum << "\n";
        cout << "Number of original packets received: " << packetNum << "\n";
        cout << "Number of retransmitted packets received: " << reTranPackets << "\n";
    }
    return sockfd;
}//END OF METHOD

char checksum(char *frame, int count) {
    u_long sum = 0;
    while (count--) {
        sum += *frame++;
        if (sum & 0xFFFF0000) {
            sum &= 0xFFFF;
            sum++; 
        }
    }
    return (sum & 0xFFFF);
}

//MAIN FUNCTION
int main(){
  //INTRO MESSAGE	
  cout<<"\nRUNNING SERVER ("<<IP<<")\n\n";
  //SOCKET CONNECTION	
  int serverSocket = receivePackets();
  close(serverSocket);//CLOSE CONNECTION
}//END OF MAIN
