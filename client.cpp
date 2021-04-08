#include "includes.h"
#define TIMEOUT 10

using namespace std;
using namespace std::chrono;

int sockfd;
struct sockaddr_in servaddr;
int window_len;
bool *window_ack_mask;
time_stamp *window_sent_time;
int lar, lfs, len;
time_stamp TMIN = current_time();
mutex window_info_mutex;
struct timeval tv;

typedef struct packet{
    char data[1024];
}Packet;

typedef struct frame{
    int frame_kind; //ACK:0, SEQ:1 FIN:2
    int sq_no;
    int ack;
    Packet packet;
}Frame;

void run_SR(int packetSize, int slidingWindowSize, char *fileName, int timeout);
void run_SaW(int packetSize, int slidingWindowSize, char *fileName, int timeout);
void run_GBN(int packetSize, int slidingWindowSize, char *outputFile);


void listen_ack() {
    //Variables
    int ackCount = 0;
    char ack[ACK_SIZE];
    int ack_size;
    int ack_seq_num;
    bool ack_error;
    bool ack_neg;

    //Listen for ack from reciever
    while (true) {
        socklen_t server_addr_size;
        ack_size = recvfrom(sockfd, (char *)ack, ACK_SIZE, 
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

void callserver(){
    int n;
    char buffer[MAXLINE];
    const char *ConnConfirm = "Client Connected...\n";
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
    char fileName[MAXLINE]; //File name
    int timeout; //Timeout

    // Gets our protocol choice
    cout << "Type of protocol:\n1)SaW\n2)SR\n-> ";
    cin >> protocolChoice; 

    cout << "\nPacket size: ";
    cin >> packetSize;
    
    // Gets our timeout choice
    cout<<"\nDo you wish to set a timeout, or use a ping-calculated timeout?\n";
    cout<<"1)Set a timeout\n2)Ping-calculated timeout\n-> ";
    cin >> timeoutChoice;
    
    if(timeoutChoice == 1){
        cout<<"Enter a timeout: ";
        cin >> timeout;
        
    }else{
        timeout = TIMEOUT;
    }
    
    
    if(protocolChoice != 1){
        // Gets our SWS
        cout<<"\nEnter Sliding Window Size: ";
        cin >> slidingWindowSize;
    }else{
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
    
    if (protocolChoice == 1){
        run_SaW(packetSize, slidingWindowSize, fileName, timeout);
    }else if(protocolChoice == 2){
        run_SR(packetSize, slidingWindowSize, fileName, timeout);
    }else if(protocolChoice == 3){
        run_GBN(packetSize, slidingWindowSize, fileName);
    }
}//end of menu 

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

    char frame[MAX_FRAME_SIZE];
    char data[MAX_DATA_SIZE];
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
        int seq_count = bufferSize / MAX_DATA_SIZE + ((bufferSize % MAX_DATA_SIZE == 0) ? 0 : 1);
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
        
        //Send current buffer with sliding window
        bool send_done = false;
        
        while (!send_done) {
            window_info_mutex.lock();

            //Check window ack mask, shift window if possible
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
                lar += shift;
                lfs = lar + slidingWindowSize;
            }
            window_info_mutex.unlock();

            //Send frames that have not been sent or have timed out
            for (int i = 0; i < slidingWindowSize; i ++) {
                seq_num = lar + i + 1;
                if (seq_num < seq_count) {
                    window_info_mutex.lock();
                    if (!window_sent_mask[i] || (!window_ack_mask[i] && (elapsed_time(current_time(), window_sent_time[i]) > timeout))) {
                        int buffer_shift = seq_num * MAX_DATA_SIZE;
                        data_size = (bufferSize - buffer_shift < MAX_DATA_SIZE) ? (bufferSize - buffer_shift) : MAX_DATA_SIZE;
                        memcpy(data, buffer + buffer_shift, data_size);
                        bool eot = (seq_num == seq_count - 1) && (read_done);
                        if(eot == true){
                            cout<<"\nPacket "<<packet<<" *****Timed Out*****\n";
                            cout<<"\nPacket "<<packet<<" Re-transmitted.\n";
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

void run_SaW(int packetSize, int slidingWindowSize, char *fileName, int timeout){
    int frame_id = 0;
    Frame frame_send;
    Frame frame_recv;
    int ack_recv = 1;
    int totalFileSize;
             
    FILE *fp = fopen(fileName,"rb");
    if(fp==NULL) {
        printf("File open error\n");
        exit(0);   
    }
        
    int run = 1;
    int packetNum = 0;
    int reTranPackets = 0;
    auto start = high_resolution_clock::now();
    while(run) {
        char buff[packetSize];
        int nread = fread(buff,1,packetSize,fp);
        if(nread > 0) {
            if(ack_recv == 1){
                frame_send.sq_no = frame_id;
		            frame_send.frame_kind = 1;
		            frame_send.ack = 0;
 			          strcpy(frame_send.packet.data, buff);
 			          sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
 			          printf("Packet %d sent\n", packetNum);
            }
            Frame tempFrame = frame_send;
            sendto(sockfd, &tempFrame, sizeof(Frame), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
            int f_recv_size = recvfrom(sockfd, &frame_recv, sizeof(frame_recv), 0 ,(struct sockaddr*)&servaddr,(socklen_t*)&len);
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
    
    // End of transmission stuff
    cout << "Number of original packets sent: " << packetNum << "\n";
    cout << "Number of retransmitted packets: " << reTranPackets << "\n";  
    cout << "Total elapsed time: " << duration.count() << " seconds\n"; 
    cout << "Total throughput (Mbps): \n" /*<< (((double)((fileSize + reTranPackets * packetSize)) * 8) / duration) / 1000000 << "\n"*/; 
    cout << "Effective throughput: \n" /*<< ((double)(fileSize * 8) / duration) / 1000000*/;
    printMD5(fileName);
}//end of run SaW


void run_GBN(int packetSize, int slidingWindowSize, char *outputFile){


}//end of GBN






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