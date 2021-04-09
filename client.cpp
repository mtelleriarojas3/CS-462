/**
 * Mason Waters, Marcelo Telleria
 * Sliding Window Project
 * This is our server side of the project
 * Due: April 10th 2021
 */

#include "includes.h"
#define TIMEOUT 10

using namespace std;
using namespace std::chrono;

//Global Variables
int sockfd;
struct sockaddr_in servaddr;
int window_len;
bool *window_ack_mask;
time_stamp *window_sent_time;
int lar, lfs, len;
time_stamp TMIN = current_time();
mutex window_info_mutex;
struct timeval tv;

//Our packet struct
typedef struct packet{
    char data[1024];
}Packet;

//Our frame struct
typedef struct frame{
    int frame_kind; //ACK:0, SEQ:1 FIN:2
    int sq_no;
    int ack;
    Packet packet;
}Frame;

//Instantiate our functions
void run_SR(int packetSize, int slidingWindowSize, char *fileName, int timeout);
void run_SaW(int packetSize, int slidingWindowSize, char *fileName, int timeout);
void run_GBN(int packetSize, int slidingWindowSize, char *fileName, int timeout);

/**
 * This method is responsible for listening for our acknowledgement for the SR protocol
 */
void listen_ack() {
    //Variables
    int ackCount = 0;
    char ack[ACKSIZE];
    int ack_size;
    int ack_seq_num;
    bool ack_error;
    bool ack_neg;

    //Listen for ack from reciever
    while (true) {
        socklen_t server_addr_size;
        ack_size = recvfrom(sockfd, (char *)ack, ACKSIZE, 
                MSG_WAITALL, (struct sockaddr *) &servaddr, 
                &server_addr_size);
        ack_error = read_ack(&ack_seq_num, &ack_neg, ack);
        
        ackCount++;
        cout<<"Ack " <<ackCount<< " received!\n";
        window_info_mutex.lock();
        if (!ack_error && ack_seq_num > lar && ack_seq_num <= lfs) {
            if (!ack_neg) {
                window_ack_mask[ack_seq_num - (lar + 1)] = true;
            } else {
                window_sent_time[ack_seq_num - (lar + 1)] = TMIN;
            }
        }
        window_info_mutex.unlock();
    }
}

/**
 * This method is called by main to connect to our server
 */
void callserver(){
    //Variables
    int n;
    char buffer[MAXLINE];
    const char *ConnConfirm = "Client Connected...";
    memset(&servaddr, 0, sizeof(servaddr)); 

    /* Fill server address data structure */
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP);
    
    /* Create socket file descriptor */ 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cout<<"socket creation failed...";
        
    }

    // Sends our confirm connection to server
    sendto(sockfd, (const char *)ConnConfirm, strlen(ConnConfirm), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr,(socklen_t*)&len);
    buffer[n] = '\0';
    cout<<buffer;
    memset(buffer, 0, sizeof(buffer));
}//end of callserver

/**
 * This is the menu for our user to put info in. It also calls the respective protocol for file transmission
 */
void menu(){
    int protocolChoice; //GBN or SR or SaW?
    int timeoutChoice; //what the user decides for timeout interval question
    int packetSize; //Our packet size
    int slidingWindowSize; //Sliding window size
    int errorsChoice; //Which error the user wants to implements
    char fileName[MAXLINE]; //File name to parse
    int timeout; //Timeout

    // Gets our protocol choice
    cout << "Type of protocol:\n1)SaW\n2)SR\n-> ";
    cin >> protocolChoice; 

    //Gets our packet size
    cout << "\nPacket size: ";
    cin >> packetSize;
    
    // Gets our timeout choice
    cout<<"\nDo you wish to set a timeout, or use a ping-calculated timeout?\n";
    cout<<"1)Set a timeout\n2)Ping-calculated timeout\n-> ";
    cin >> timeoutChoice;
    
    //If we want a timeout or nah
    if(timeoutChoice == 1){
        cout<<"Enter a timeout: ";
        cin >> timeout; 
    }else{
        timeout = TIMEOUT;
    }
    
    //If our protocol choice is not SaW we need to enter a window size, otherwise it is automatically set to 1
    if(protocolChoice != 1) {
        // Gets our SWS
        cout<<"\nEnter Sliding Window Size: ";
        cin >> slidingWindowSize;
    }else {
        slidingWindowSize = 1;
    }
    
    // Gets our Situation error choice
    cout << "\nDo you want to create errors?\n1)Yes\n2)No\n-> ";
    cin >> errorsChoice;

    // Gets our file name from user
    cout<<"\nEnter a file name: ";
    cin >> fileName;
    cout<<"\n";
    
    //SEND PROTOCOL BEING USED
    sendto(sockfd, &protocolChoice, sizeof(protocolChoice), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        
    //SEND PACKETSIZE
    sendto(sockfd, &packetSize, sizeof(packetSize), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    
    //SEND SLIDING WINDOW SIZE
    sendto(sockfd, &slidingWindowSize, sizeof(slidingWindowSize), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    
    //Sends us to the respective protocol to be done
    if (protocolChoice == 1){
    
        run_SaW(packetSize, slidingWindowSize, fileName, timeout);
        
    }else if(protocolChoice == 2){
    
        run_SR(packetSize, slidingWindowSize, fileName, timeout);
        
    }else if(protocolChoice == 3){
    
        run_GBN(packetSize, slidingWindowSize, fileName, timeout);
        
    }
}//end of menu 

/**
 * This is our SR method. It is called from the menu method and completes the protocol.
 * It calls our packetSize, slidingWindowSize and outputFile name and timeout
 */
void run_SR(int packetSize, int slidingWindowSize, char *fileName, int timeout){
    auto start = high_resolution_clock::now();
    int packet = 0;

    if (access(fileName, F_OK) == -1) {
        cerr << "file doesn't exist: " << fileName << endl;
    }

    //OPEN FILE TO SEND
    FILE *file = fopen(fileName, "rb");//fname
    char buffer[packetSize];
    int bufferSize;

   //THE THREAD WILL START LISTENING FOR ACK
    thread recv_thread(listen_ack);

    char frame[MAXFRAME];
    char data[MAXLINE];
    int frame_size;
    int data_size;

    //SEND FILE
    bool read_done = false;
    int reTranPackets = 0;
    
    while (!read_done) {
        //READ PART OF FILE TO BUFFER
        bufferSize = fread(buffer, 1, packetSize, file);
        if (bufferSize == packetSize) {
            char temp[1];
            int bufferSize = fread(temp, 1, 1, file);
            if (bufferSize == 0) read_done = true;
            int error = fseek(file, -1, SEEK_CUR);
        } else if (bufferSize < packetSize) {
            read_done = true;
        }
        window_info_mutex.lock();

        //INITIALIZE THE VARIABLES FOR SLIDING WINDOW
        int seq_count = bufferSize / MAXLINE + ((bufferSize % MAXLINE == 0) ? 0 : 1);
        int seq_num;
        window_sent_time = new time_stamp[slidingWindowSize];
        window_ack_mask = new bool[slidingWindowSize];
        bool window_sent_mask[slidingWindowSize];
        for (int i = 0; i < slidingWindowSize; i++) {
            window_ack_mask[i] = false;
            window_sent_mask[i] = false;
        }
        lar = -1;
        lfs = lar + slidingWindowSize;

        window_info_mutex.unlock();
        //SEND THE BUFFER WITH SLIDING WINDOW
        bool send_done = false;
        
        while (!send_done) {
            window_info_mutex.lock();

            
            //CHECK WINDOW ACK MASK AND SHIFT IF NECESSARY
            if (window_ack_mask[0]) {
                int shift = 1;
                for (int i = 1; i < slidingWindowSize; i++) {
                    if (!window_ack_mask[i]) break;
                    shift += 1;
                }
                for (int i = 0; i < slidingWindowSize - shift; i++) {
                    window_sent_mask[i] = window_sent_mask[i + shift];
                    window_ack_mask[i] = window_ack_mask[i + shift];
                    window_sent_time[i] = window_sent_time[i + shift];
                }
                for (int i = slidingWindowSize - shift; i < slidingWindowSize; i++) {
                    window_sent_mask[i] = false;
                    window_ack_mask[i] = false;
                }
                lar += shift; //last ack received
                lfs = lar + slidingWindowSize; //last frame sent
            }
            window_info_mutex.unlock();

           
            //SEND ALL FRAMES THAT HAVE TIMED OUT OR HAVEN'T BEEN SENT
            for (int i = 0; i < slidingWindowSize; i ++) {
                seq_num = lar + i + 1;
                if (seq_num < seq_count) {
                    window_info_mutex.lock();
                    if (!window_sent_mask[i] || (!window_ack_mask[i] && (elapsed_time(current_time(), window_sent_time[i]) > timeout))) {
                        int buffer_shift = seq_num * MAXLINE;
                        data_size = (bufferSize - buffer_shift < MAXLINE) ? (bufferSize - buffer_shift) : MAXLINE;
                        memcpy(data, buffer + buffer_shift, data_size);
                        bool eot = (seq_num == seq_count - 1) && (read_done);
                        if(eot == true){
                            cout<<"\nPacket "<<packet<<" *****Timed Out*****\n";
                            cout<<"Packet "<<packet<<" Re-transmitted.\n";
                            reTranPackets++; 
                        }
                        frame_size = create_frame(seq_num, frame, data, data_size, eot);
                        sendto(sockfd, frame, frame_size, 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));     
                        window_sent_mask[i] = true;
                        window_sent_time[i] = current_time();
                    }
                    window_info_mutex.unlock();
                }
            }
            //Move to next buffer if all frames in current buffer has been acked
            if (lar >= seq_count - 1) 
            send_done = true;
        }                   
        cout<<"\nPacket " <<packet<< " sent\n"; 
        cout<<"Current Window: [";
        for(int i = packet; i < ((packet + slidingWindowSize)-1); i++) {
            cout << i << ", ";
        } 
        cout << (packet + slidingWindowSize) << "]\n";
        packet++;
        if (read_done) break;
    }
    fclose(file);
    delete [] window_ack_mask;
    delete [] window_sent_time;
    recv_thread.detach();
    
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    // End of transmission stuff
    cout << "\nNumber of original packets sent: " << packet << "\n";
    cout << "Number of retransmitted packets: " << reTranPackets << "\n";  
    cout << "Total elapsed time: " << duration.count() << " seconds\n"; 
    cout << "Total throughput (Mbps): \n" /*<< (((double)((fileSize + reTranPackets * packetSize)) * 8) / duration) / 1000000 << "\n"*/; 
    cout << "Effective throughput: \n" /*<< ((double)(fileSize * 8) / duration) / 1000000*/;
    printMD5(fileName);
}//end of run_SR

/**
 * This is our SaW method. It is called from the menu method and completes the protocol.
 * It calls our packetSize, slidingWindowSize and outputFile name and timeout
 */
void run_SaW(int packetSize, int slidingWindowSize, char *fileName, int timeout){
    //Variable creation
    int frame_id = 0;
    Frame frame_send;
    Frame frame_recv;
    int ack_recv = 1;
    int totalFileSize;
    int run = 1;
    int packetNum = 0;
    int reTranPackets = 0;
    
    //Open our file to be transferred
    FILE *fp = fopen(fileName,"rb");
    if(fp==NULL) {
        printf("File open error\n");
        exit(0);   
    }
        
    //Start our timer
    auto start = high_resolution_clock::now();
    //Loop indefinitely till done
    while(run) {
        //Start parsing file
        char buff[packetSize];
        int nread = fread(buff,1,packetSize,fp);
        //If our parsing is not complete, transfer shit
        if(nread > 0) {
            //We have received an ack
            if(ack_recv == 1){
                //Reset frame shit
                frame_send.sq_no = frame_id;
		            frame_send.frame_kind = 1;
		            frame_send.ack = 0;
 			          strcpy(frame_send.packet.data, buff);
                //Send our packet
 			          sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
 			          printf("Packet %d sent\n", packetNum);
            }
            //Send our stuff for checksum
            Frame tempFrame = frame_send;
            sendto(sockfd, &tempFrame, sizeof(Frame), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
            //Wait for ack
            int f_recv_size = recvfrom(sockfd, &frame_recv, sizeof(frame_recv), 0 ,(struct sockaddr*)&servaddr,(socklen_t*)&len);
            //Ack is received brother otherwise time out
            if( f_recv_size > 0 && frame_recv.sq_no == 0 && frame_recv.ack == frame_id+1){
                printf("Ack %d received\n", frame_id);
		            ack_recv = 1;
                cout << "Current window = [" << frame_send.sq_no<<"]\n\n";
            }else{
		            cout << "Packet " << packetNum << " *****Timed Out*****";
                cout << "Packet " << packetNum << " Re-transmitted.";
                reTranPackets++;
		            ack_recv = 0;
            }	
            //Increment stuff
            frame_id++;
            packetNum++;
            bzero(buff, packetSize);
        } else {
            //Run is complete and we send out last timeout packet because we DONE
            run = 0; 
            int done = -1;
            sendto(sockfd, &done, sizeof(done), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
            cout << "\n";
        }                        
    }  
    //Decrement stuff and stop timer then compute our duration
    packetNum--; 
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    
    // End of transmission stuff
    cout << "Session successfully terminated\n\n";
    cout << "Number of original packets sent: " << packetNum << "\n";
    cout << "Number of retransmitted packets: " << reTranPackets << "\n";  
    cout << "Total elapsed time: " << duration.count() << " seconds\n"; 
    cout << "Total throughput (Mbps): \n" /*<< (((double)((fileSize + reTranPackets * packetSize)) * 8) / duration) / 1000000 << "\n"*/; 
    cout << "Effective throughput: \n" /*<< ((double)(fileSize * 8) / duration) / 1000000*/;
    printMD5(fileName);
}//end of run SaW

void run_GBN(int packetSize, int slidingWindowSize, char *fileName, int timeout) {
    cout << "We are in GBN!\n";
    /*Frame frame_send;
    Frame frame_temp;
    int sequence_first = 0;
    int sequence_N = 0;
    int run = 1;
    
    while(run) {
        
    }*/
}

/**
 * This is our main method that runs the client
 */
int main() {
    //INTRO MESSAGE
    cout<<"\nRUNNING CLIENT\n";
    //CONNECT TO THE SERVER
    callserver();
    //DISPLAY MENU FOR THE USER
    menu();
    return 0;
}