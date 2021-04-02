#include "includes.h"
#include <sys/time.h>
#define SA struct sockaddr 

typedef struct packet{
    char data[1024];
}Packet;

typedef struct frame{
    int frame_kind; //ACK:0, SEQ:1 FIN:2
    int sq_no;
    int ack;
    Packet packet;
}Frame;

struct timeval tv;
struct sockaddr_in servaddr;
int n, len;

// Define functions
int clientFunction();
int callserver();
void goBackN();
void selectiveRepeat();
void stopAndWait();

/**
 * This function connects us to our server so we can execute the protocol!
 */
int callserver(){
    int sockfd;
    char buffer[MAXLINE];
    const char *ConnConfirm = "Client Connected...";
   
    //SOCKET CREATION AND VERIFICATION  
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed..."); 
        exit(EXIT_FAILURE);
    } else {
        cout<<"Connecting to the client...\n";
    } 
  
    memset(&servaddr, 0, sizeof(servaddr));
    
    //ASSIGN IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(9101);
    servaddr.sin_addr.s_addr = inet_addr(IP);
    
    // connect the client socket to server socket 
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    }

    // Sends our confirm connection to server
    sendto(sockfd, (const char *)ConnConfirm, strlen(ConnConfirm), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr,(socklen_t*)&len);
    buffer[n] = '\0';
    cout<<buffer;
    memset(buffer, 0, sizeof(buffer));
    
    return sockfd;
}//END OF FUNCTION

/**
 * This function gets our information from the client and then calls our protocol/sends information to server
 */
int clientFunction(){
    int protocolChoice; //GBN OR SR or SaW?
    int timeoutChoice; //what the user decides for timeout interval question
    int packetSize; 
    int slidingWindowSize; 
    int errorsChoice;
    int fileSize;  
    FILE *infile;
    char fileName[MAXLINE];
  
    //connect to server
    int sockfd = callserver();
      
    // Gets our protocol choice
    cout << "Type of protocol:\n1)GBN\n2)SR\n3)SaW\n-> ";
    cin >> protocolChoice; 

    cout << "\nPacket size: ";
    cin >> packetSize;
    
    //packetSize *= 1000; //conver to KB                                                    //USE LATER PROBABLY
    
    // Gets our timeout choice
    cout<<"\nDo you wish to set a timeout, or use a ping-calculated timeout?\n";
    cout<<"1)Set a timeout\n2)Ping-calculated timeout\n-> ";
    cin >> timeoutChoice;
    
    // Gets our SWS
    cout<<"\nEnter Sliding Window Size: ";
    cin >> slidingWindowSize;
    
    // Gets our Situation error choice
    cout << "\nSituational errors:\n1)drop packets\n2)lose acks\n-> ";
    cin >> errorsChoice;

    // Gets our file name from user
    cout<<"\nEnter a file name: ";
    cin >> fileName;
    cout<<"\n";

    //HERE WE PROMPT THE USER FOR A FILE NAME AND WE GET THE FILE SIZE
    infile = get_file(fileName);//get filename and open file
    fileSize = get_file_Size(infile);
    
    // send protocol use over to server
    sendto(sockfd, &protocolChoice, sizeof(protocolChoice),MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    
    //send total file size to server
    uint32_t totalSizeTemp = htonl(fileSize);//
    sendto(sockfd, &totalSizeTemp, sizeof(totalSizeTemp),MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
     
	  //send packet size to server
    uint32_t tempPacketSize = htonl(packetSize);// 
    sendto(sockfd, &tempPacketSize, sizeof(tempPacketSize),MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    
    //send packet size to server
    uint32_t tempWindowSize = htonl(slidingWindowSize);// 
    sendto(sockfd, &tempWindowSize, sizeof(tempWindowSize),MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    
    //SaW Protocol
    if(protocolChoice == 3) {
        int frame_id = 0;
	      Frame frame_send;
	      Frame frame_recv;
	      int ack_recv = 1;
             
        FILE *fp = fopen("test.txt","rb");
        if(fp==NULL) {
            printf("File open error\n");
            return 1;   
        }
        
        int run = 1;
        int packetNum = 0;
        int reTranPackets = 0;
        auto start = high_resolution_clock::now();
        while(run) {
            char buff[packetSize];
            int nread = fread(buff,1,packetSize,fp);
            if(nread > 0) {
                //cout << nread;
                if(ack_recv == 1){
                    frame_send.sq_no = frame_id;
    			          frame_send.frame_kind = 1;
    			          frame_send.ack = 0;
    			          strcpy(frame_send.packet.data, buff);
    			          sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    			          printf("Packet %d sent\n", packetNum);
    		        }
                Frame tempFrame = frame_send;
                //cout << tempFrame.packet.data << "\n";
                sendto(sockfd, &tempFrame, sizeof(Frame), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
                tv.tv_sec = 2;
                tv.tv_usec = 0;
                setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
                int f_recv_size = recvfrom(sockfd, &frame_recv, sizeof(frame_recv), 0 ,(struct sockaddr*)&servaddr,(socklen_t*)&len);
    		        if( f_recv_size > 0 && frame_recv.sq_no == 0 && frame_recv.ack == frame_id+1){
    			          printf("Ack %d received\n", frame_id);
    			          ack_recv = 1;
    		        }else{
    			          cout << "Packet " << packetNum << " *****Timed Out*****";
                    cout << "Packet " << packetNum << " Re-transmitted.";
                    reTranPackets++;
    			          ack_recv = 0;
    		        }	
    		        frame_id++;
                packetNum++;
                bzero(buff, packetSize);
            } else {
                run = 0; 
                int done = -1;
                sendto(sockfd, &done, sizeof(done), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
                cout << "\n";
            }                        
        }  
        packetNum--; 
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<seconds>(stop - start);
        // End of file stuff
        cout << "Number of original packets sent: " << packetNum << "\n";
        cout << "Number of retransmitted packets: " << reTranPackets << "\n";  
        cout << "Total elapsed time: " << duration.count() << " seconds\n"; 
        cout << "Total throughput (Mbps): \n" /*<< (((double)((fileSize + reTranPackets * packetSize)) * 8) / duration) / 1000000 << "\n"*/; 
        cout << "Effective throughput: \n" /*<< ((double)(fileSize * 8) / duration) / 1000000*/;
    }	
    
    fclose(infile);
    //printMD5(fileName); 
    return sockfd;
}

// Our main method. Sets up server connection then calls protocol
int main(){
    //INTRO MESSAGE
    cout<<"\nRUNNING CLIENT\n";
    //SOCKET CONNECTION
    int clientSocket = clientFunction();
    close(clientSocket);//CLOSE CONNECTION
}//END OF MAIN
