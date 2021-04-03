#include "includes.h"
#define STDBY_TIME 3000
#define MAXLINE 1024
using namespace std;

int sockfd;
struct sockaddr_in servaddr, cliaddr;
int len;

typedef struct packet{
    char data[1024];
}Packet;

typedef struct frame{
    int frame_kind; //ACK:0, SEQ:1 FIN:2
    int sq_no;
    int ack;
    Packet packet;
}Frame;

void run_SR(int PacketSize, int window_len, int max_buffer_size, char *outputFile);
void run_SaW(int PacketSize, int window_len, int max_buffer_size, char *outputFile);


void send_ack() {
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
        frame_size = recvfrom(sockfd, (char *)frame, MAX_FRAME_SIZE, 
                MSG_WAITALL, (struct sockaddr *) &cliaddr, 
                &clientSize);
        frame_error = read_frame(&recv_seq_num, data, &data_size, &eot, frame);
        
        create_ack(recv_seq_num, ack, frame_error);
        sendto(sockfd, ack, ACK_SIZE, 0, 
                (const struct sockaddr *) &cliaddr, clientSize);
    }
}


void setupServerSocket(){
    int port = 9001;
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
        cerr << "socket creation failed" << endl;
    }

    /* Bind socket to server address */
    if (::bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) { 
        cerr << "socket binding failed" << endl;
    }

    int n;  
    //We have no clients yet
    cout<<"No clients connected at the moment...\n";

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
    int protocolChoice;
    int packetSize;
    int slidingWindowSize;
    
    //RECEIVE PROTOCOL CHOICE
    recvfrom(sockfd, &protocolChoice, sizeof(protocolChoice), MSG_WAITALL, (struct sockaddr *) &servaddr,(socklen_t*)&len);
    
    //RECEIVE PACKETSIZE
    recvfrom(sockfd, &packetSize, sizeof(packetSize), MSG_WAITALL, (struct sockaddr *) &servaddr,(socklen_t*)&len);
    
    //RECEIVE SLIDING WINDOW SIZE
    recvfrom(sockfd, &slidingWindowSize, sizeof(slidingWindowSize), MSG_WAITALL, (struct sockaddr *) &servaddr,(socklen_t*)&len);

    //Variables
    int window_len = slidingWindowSize;
    int max_buffer_size = packetSize;
    char outputFile[] = "received.txt";
    
    
    if(protocolChoice == 1){
        //Call Go Back N Protocol
    }else if(protocolChoice == 2){
        //Call Selective Repeat Protocol
        run_SR(packetSize,window_len, max_buffer_size, outputFile);
    }else if(protocolChoice == 3) {
        //Call Stop and Wait Protocol
        run_SaW(packetSize, window_len, max_buffer_size, outputFile);
    }else{
      //print error message
    }
   

    printMD5(outputFile);
}//end of serverfunction

void run_SR(int PacketSize, int window_len, int max_buffer_size, char *outputFile){
    //Open our file to put receive buffers in
    FILE *file = fopen(outputFile, "wb");
    char buffer[max_buffer_size];
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
    int packet = 0;
    int ackCount = 0;
    bool recv_done = false;
    int buffer_num = 0;
    while (!recv_done) {
        buffer_size = max_buffer_size;
        memset(buffer, 0, buffer_size);
    
        int recv_seq_count = (int) max_buffer_size / MAX_DATA_SIZE;
        bool window_recv_mask[window_len];
        for (int i = 0; i < window_len; i++) {
            window_recv_mask[i] = false;
        }
        lfr = -1;
        laf = lfr + window_len;
        
        /* Receive current buffer with sliding window */
        while (true) {
            socklen_t clientSize;
            frame_size = recvfrom(sockfd, (char *) frame, MAX_FRAME_SIZE, 
                    MSG_WAITALL, (struct sockaddr *) &cliaddr, 
                    &clientSize);
            frame_error = read_frame(&recv_seq_num, data, &data_size, &eot, frame);
         
            packet++;
            cout<<"\nPacket " << packet << " received!\n";
            
            create_ack(recv_seq_num, ack, frame_error);
            sendto(sockfd, ack, ACK_SIZE, 0, (const struct sockaddr *) &cliaddr, clientSize);
            ackCount++;
            cout<<"Ack " <<ackCount<< " sent!\n";
            if (recv_seq_num <= laf) {
                if (!frame_error) {
                    int buffer_shift = recv_seq_num * MAX_DATA_SIZE;
                    if (recv_seq_num == lfr + 1) {
                        memcpy(buffer + buffer_shift, data, data_size);
                        int shift = 1;
                        for (int i = 1; i < window_len; i++) {
                            if (!window_recv_mask[i]) break;
                            shift += 1;
                        }
                        for (int i = 0; i < window_len - shift; i++) {
                            window_recv_mask[i] = window_recv_mask[i + shift];
                        }
                        for (int i = window_len - shift; i < window_len; i++) {
                            window_recv_mask[i] = false;
                        }
                        lfr += shift;
                        laf = lfr + window_len;
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

    //Start thread to keep sending requested ack to sender for 3 seconds 
    thread stdby_thread(send_ack);
    time_stamp start_time = current_time();
    while (elapsed_time(current_time(), start_time) < STDBY_TIME) {
        cout << "\r" << "[STANDBY TO SEND ACK FOR 3 SECONDS | ]" << flush;
        sleep_for(100);
        cout << "\r" << "[STANDBY TO SEND ACK FOR 3 SECONDS / ]" << flush;
        sleep_for(100);
        cout << "\r" << "[STANDBY TO SEND ACK FOR 3 SECONDS - ]" << flush;
        sleep_for(100);
        cout << "\r" << "[STANDBY TO SEND ACK FOR 3 SECONDS \\ ]" << flush;
        sleep_for(100);
    }
    stdby_thread.detach();
    cout<<"\n";
}

void run_SaW(int packetSize, int window_len, int max_buffer_size, char *test) {
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
        
    FILE *fp;
    fp = fopen("Sample.txt", "ab");
    if(NULL == fp) {
        printf("Error opening file");
        exit(1);
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

int main() {
    //INTRO MESSAGE	
    cout<<"\nRUNNING SERVER ("<<IP<<")\n\n";
    //SET UP SERVER SOCKET
    setupServerSocket();
    //call server function
    serverFunction();
    return 0;
}