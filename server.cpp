/**
 * Mason Waters, Marcelo Telleria
 * Sliding Window Project
 * This is our server side of the project
 * Due: April 10th 2021
 */
#include "includes.h"
#define STDBY_TIME 3000
#define MAXLINE 1024
using namespace std;

//Variables
int sockfd;
struct sockaddr_in servaddr, cliaddr;
int len;

//Our packet
typedef struct packet{
    char data[1024];
}Packet;

//Our frame
typedef struct frame{
    int frame_kind; //ACK:0, SEQ:1 FIN:2
    int sq_no;
    int ack;
    Packet packet;
}Frame;

//Declare our functions
void run_SR(int slidingWindowSize, int packetSize, char *outputFile);
void run_SaW(int PacketSize, int slidingWindowSize, char *outputFile);
void run_GBN(int PacketSize, int slidingWindowSize, char *outputFile);

/**
 * This method is responsible for sending our acknowledgement for the SR protocol
 */
void send_ack() {
    //Declare variables
    char frame[MAX_FRAME_SIZE];
    char data[MAX_DATA_SIZE];
    char ack[ACK_SIZE];
    int frame_size;
    int data_size;
    socklen_t clientSize;
    int recv_seq_num;
    bool frame_error;
    bool eot;

    /* Listen for frames and send ack */
    while (true) {
        frame_size = recvfrom(sockfd, (char *)frame, MAX_FRAME_SIZE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &clientSize);
        frame_error = read_frame(&recv_seq_num, data, &data_size, &eot, frame);
        create_ack(recv_seq_num, ack, frame_error);
        sendto(sockfd, ack, ACK_SIZE, 0, (const struct sockaddr *) &cliaddr, clientSize);
    }
}

/**
 * This method is called by main to set uo our sockets for transferring the file
 */
void setupServerSocket(){
    //Vairables
    char buffer[MAXLINE];
    const char *ConnConfirm = "Connected to server successfully!\n";
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    /* Fill server address data structure */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT);

    /* Create socket file descriptor */ 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cout<< "socket creation failed...";
    }

    /* Bind socket to server address */
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) { 
        cout<<"binding failed...";
    }

    int n;  
    cout<<"No clients connected at the moment...\n";//Waiting for client
    len = sizeof(cliaddr);  //len is value/result

    // Receive connection confirmation from client
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
    buffer[n] = '\0';
    printf("%s\n", buffer);

    sendto(sockfd, (const char *)ConnConfirm, strlen(ConnConfirm), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
    memset(buffer, 0, sizeof(buffer));
}//end of setupserversocket


/**
 * This function sets up our server and prompts the user. It then calls the respective protocol to transfer the file.
 */
void serverFunction(){
    //Variables
    int protocolChoice;
    int packetSize;
    int slidingWindowSize;
    
    //RECEIVE PROTOCOL CHOICE
    recvfrom(sockfd, &protocolChoice, sizeof(protocolChoice), MSG_WAITALL, (struct sockaddr *) &servaddr,(socklen_t*)&len);
    
    //RECEIVE PACKETSIZE
    recvfrom(sockfd, &packetSize, sizeof(packetSize), MSG_WAITALL, (struct sockaddr *) &servaddr,(socklen_t*)&len);
    
    //RECEIVE SLIDING WINDOW SIZE
    recvfrom(sockfd, &slidingWindowSize, sizeof(slidingWindowSize), MSG_WAITALL, (struct sockaddr *) &servaddr,(socklen_t*)&len);

    //Output file name
    char outputFile[] = "received.txt";
    
    //Calls the correct protocol
    if(protocolChoice == 1){
        //Call Go Back N Protocol
        run_SaW(packetSize, slidingWindowSize, outputFile);
    }else if(protocolChoice == 2){
        //Call Selective Repeat Protocol
        run_SR(slidingWindowSize, packetSize, outputFile);
    }else if(protocolChoice == 3) {
        //Call Stop and Wait Protocol
        run_GBN(packetSize, slidingWindowSize, outputFile);
    }else{
      //print error message
    }
   
    printMD5(outputFile);
}//end of serverfunction

/**
 * This is our SR method. It is called from the serverFunction method and completes the protocol.
 * It calls our packetSize, slidingWindowSize and outputFile name
 */
void run_SR(int slidingWindowSize, int packetSize, char *outputFile){
    //Open our file to put receive buffers in
    FILE *file = fopen(outputFile, "wb");
        if(NULL == file) {
        cout<< "Error opening file";
        exit(1);
    }
    
    char buffer[packetSize];
    int buffer_size;

    //Initialize sliding window variables 
    char frame[MAX_FRAME_SIZE];
    char data[MAX_DATA_SIZE];
    char ack[ACK_SIZE];
    int frame_size;
    int data_size;
    int lfr, laf;
    int recv_seq_num;
    bool eot;
    bool frame_error;

    //Receive frames until end of transmission
    int reTranPackets = 0;
    int packet = 0;
    int ackCount = 0;
    bool recv_done = false;
    int buffer_num = 0;
    
    //Loop indefinitely until complete
    while (!recv_done) {
        buffer_size = packetSize;
        memset(buffer, 0, buffer_size);
        int recv_seq_count = (int) packetSize / MAX_DATA_SIZE;
        bool window_recv_mask[slidingWindowSize];
        for (int i = 0; i < slidingWindowSize; i++) {
            window_recv_mask[i] = false;
        }
        lfr = -1;
        laf = lfr + slidingWindowSize;
        
        /* Receive current buffer with sliding window */
        while (true) {
            socklen_t clientSize;
            frame_size = recvfrom(sockfd, (char *) frame, MAX_FRAME_SIZE, 
                    MSG_WAITALL, (struct sockaddr *) &cliaddr, 
                    &clientSize);
            frame_error = read_frame(&recv_seq_num, data, &data_size, &eot, frame);
            if(frame_size > 0){ 
                packet++;
                cout<<"\nPacket " << packet << " received\n";
                cout<<"Current Window: [";
                for(int i = packet; i < ((packet + slidingWindowSize)-1); i++) {
                    cout << i << ", ";
                } 
                cout << (packet + slidingWindowSize) << "]\n";
                if(frame_error == 0){
                    cout<<"CheckSum OK\n";
                }else{
                    cout<<"Checksum failed\n";
                    reTranPackets++;
                }
            }
            create_ack(recv_seq_num, ack, frame_error);
            sendto(sockfd, ack, ACK_SIZE, 0, (const struct sockaddr *) &cliaddr, clientSize);
            ackCount++;
            cout<<"Ack " <<ackCount<< " sent\n";
            if (recv_seq_num <= laf) {
                if (!frame_error) {
                    int buffer_shift = recv_seq_num * MAX_DATA_SIZE;
                    if (recv_seq_num == lfr + 1) {
                        memcpy(buffer + buffer_shift, data, data_size);
                        int shift = 1;
                        for (int i = 1; i < slidingWindowSize; i++) {
                            if (!window_recv_mask[i]) break;
                            shift += 1;
                        }
                        for (int i = 0; i < slidingWindowSize - shift; i++) {
                            window_recv_mask[i] = window_recv_mask[i + shift];
                        }
                        for (int i = slidingWindowSize - shift; i < slidingWindowSize; i++) {
                            window_recv_mask[i] = false;
                        }
                        lfr += shift;
                        laf = lfr + slidingWindowSize;
                    } else if (recv_seq_num > lfr + 1) {
                        if (!window_recv_mask[recv_seq_num - (lfr + 1)]) {
                            memcpy(buffer + buffer_shift, data, data_size);
                            window_recv_mask[recv_seq_num - (lfr + 1)] = true;
                        }
                    }
                    /* Set max sequence to sequence of frame with EOT */ 
                    if (eot) {
                        buffer_size = buffer_shift + data_size;
                        recv_seq_count = recv_seq_num + 1;
                        recv_done = true;
                    }
                }
            }
            /* Move to next buffer if all frames in current buffer has been received */
            if (lfr >= recv_seq_count - 1) break;
        }
        //Write to our file
        fwrite(buffer, 1, buffer_size, file);
        buffer_num += 1;
    }
    //Close file when done
    fclose(file);

    cout << "\nLast packet seq# received: Packet #" << packet << "\n";
    cout << "Number of original packets received: " << packet << "\n";
    cout << "Number of retransmitted packets received: " << reTranPackets << "\n";
}

/**
 * This is our SaW method. It is called from the serverFunction method and completes the protocol.
 * It calls our packetSize, slidingWindowSize and outputFile name
 */
void run_SaW(int packetSize, int slidingWindowSize, char *outputFile) {
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
   	//ofstream fileReceived;
    int run = 1;
    int reTranPackets = 0;
    
    //Open our file to be outputted to    
    FILE *file = fopen(outputFile, "wb");
    if(NULL == file) {
        cout<<"Error opening file";
        exit(1);
    }
    //Loop indefinitely
    while(run) {
        //Receving from client
        int f_recv_size = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0, (struct sockaddr*)&cliaddr,(socklen_t*)&len);
        if (f_recv_size > 0 && frame_recv.frame_kind == 1 && frame_recv.sq_no == frame_id){
            //Packet received, write to file and go to ack
		        printf("Packet %d received\n", packetNum);
            std::string test;
            test += frame_recv.packet.data;                  
            fwrite(test.c_str(), 1, test.size(), file); //Write to file
            //Checksum area
            int f_checksum_size = recvfrom(sockfd, &frame_checksum, sizeof(Frame), 0, (struct sockaddr*)&cliaddr,(socklen_t*)&len);
            if(f_checksum_size > 0 && frame_checksum.frame_kind == 1 && frame_checksum.sq_no == frame_id) {
                cout << "Checksum OK \n";
            }
            //Set sequence numbers for frames
			      frame_send.sq_no = 0;
 			      frame_send.frame_kind = 0;
 			      frame_send.ack = frame_recv.sq_no + 1;
            //Send our ack to client
            sendto(sockfd, &frame_send, sizeof(frame_send), 0, (struct sockaddr*)&cliaddr, sizeof(servaddr));
 			      printf("Ack %d sent\n", ackNum);
        } else {
            //We did not receive a packet, this also causes our checksum to fail
            cout<<"Packet Not Received\n";
            cout<<"Checksum failed \n";
            reTranPackets++;
            //Kill program, end of file
            run = 0;
        }
        //Prints our current window
        cout << "Current window = ["<<frame_recv.sq_no<<"]\n\n";
        //Increment stuff
        ackNum++;
        packetNum++;
        frame_id++;
        //set all content in our frame to null for the next packet
        bzero(frame_recv.packet.data, packetSize);
    }
    
    //Decrement stuff
    packetNum -= 2;
    reTranPackets--;
    //End of file stuff
    cout << "\nLast packet seq# received: " << packetNum << "\n";
    cout << "Number of original packets received: " << packetNum << "\n";
    cout << "Number of retransmitted packets received: " << reTranPackets << "\n";
    //Close our file
    fclose(file);
}

/**
 * This is our GBN method. It is incomplete as we did not have time for it.
 */
void run_GBN(int PacketSize, int slidingWindowSize, char *outputFile) {
    cout << "You are in GBN!\n";
    /*Frame frame_recv;
    Frame frame_send;
    int sequenceNum = 0;
    int run = 1;
    int ackNum = 0;
    int frame_id = 0;
    int reTranPackets = 0;
    
    FILE *file = fopen(outputFile, "wb");
    if(NULL == file) {
        cout<<"Error opening file";
        exit(1);
    }
    while(run) {
        int f_recv_size = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0, (struct sockaddr*)&cliaddr,(socklen_t*)&len);
        if (f_recv_size > 0 && frame_recv.frame_kind == 1 && frame_recv.sq_no == frame_id) {
            if(frame_recv.sq_no <= 0) {
                //Do nothing?
            } else if(frame_recv.sq_no == sequenceNum) {
                std::string test;
                test += frame_recv.packet.data;
                fwrite(test.c_str(), 1, test.size(), file); //Write to our file
                sequenceNum = sequenceNum + 1;
                sendto(sockfd, &frame_send, sizeof(frame_send), 0, (struct sockaddr*)&cliaddr, sizeof(servaddr)); //Send ack
 			          printf("Ack %d sent\n", ackNum);
            }
        } else {
            cout<<"Packet Not Received\n";
            cout<<"Checksum failed \n";
            reTranPackets++;
            run = 0;
        }
    }*/
}

/**
 * This is our main method, it runs the program
 */
int main() {
    //INTRO MESSAGE	
    cout<<"\nRUNNING SERVER ("<<IP<<")\n\n";
    
    //SET UP SERVER SOCKET
    setupServerSocket();
    
    //call server function
    serverFunction();
    
    return 0;
}