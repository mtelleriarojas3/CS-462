#include "includes.h"
#define TIMEOUT 10
#define MAXLINE 1024
using namespace std;

int sockfd;
struct sockaddr_in servaddr;
int window_len;
bool *window_ack_mask;
time_stamp *window_sent_time;
int lar, lfs, len;
time_stamp TMIN = current_time();
mutex window_info_mutex;

void run_SR(int packetSize, int slidingWindowSize, char *fileName, int timeout);


void listen_ack() {
    int ackCount = 0;
    char ack[ACK_SIZE];
    int ack_size;
    int ack_seq_num;
    bool ack_error;
    bool ack_neg;

    /* Listen for ack from reciever */
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
  const char *ConnConfirm = "Client Connected...";


    memset(&servaddr, 0, sizeof(servaddr)); 

    /* Fill server address data structure */
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP);
    

    /* Create socket file descriptor */ 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "socket creation failed" << endl;
        
    }

        // Sends our confirm connection to server
    sendto(sockfd, (const char *)ConnConfirm, strlen(ConnConfirm), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr,(socklen_t*)&len);
    buffer[n] = '\0';
    cout<<buffer;
    memset(buffer, 0, sizeof(buffer));
  
}//end of callserver


void menu(){
    int protocolChoice; //GBN OR SR or SaW?
    int timeoutChoice; //what the user decides for timeout interval question
    int packetSize; 
    int slidingWindowSize; 
    int errorsChoice;
    char fileName[MAXLINE];
    int timeout;

    // Gets our protocol choice
    cout << "Type of protocol:\n1)GBN\n2)SR\n3)SaW\n-> ";
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
    
    
    //SEND PROTOCOL BEING USED
        sendto(sockfd, &protocolChoice, sizeof(protocolChoice), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        
    //SEND PACKETSIZE
        sendto(sockfd, &packetSize, sizeof(packetSize), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    
    //SEND SLIDING WINDOW SIZE
        sendto(sockfd, &slidingWindowSize, sizeof(slidingWindowSize), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    

    if (protocolChoice == 1){
    // do something
    }else if(protocolChoice == 2){
      run_SR(packetSize, slidingWindowSize, fileName, timeout);
    }else if(protocolChoice == 3){
      //RUN SaW
    }


}//end of menu 


void run_SR(int packetSize, int slidingWindowSize, char *fileName, int timeout){

    int max_buffer_size;
    window_len = slidingWindowSize;
    max_buffer_size = packetSize;//MAX_DATA_SIZE * packetSize

    if (access(fileName, F_OK) == -1) {
        cerr << "file doesn't exist: " << fileName << endl;
    }

    //OPEN FILE TO SEND
    FILE *file = fopen(fileName, "rb");//fname
    char buffer[max_buffer_size];
    int buffer_size;

   //THE THREAD WILL START LISTENING FOR ACK
    thread recv_thread(listen_ack);

    char frame[MAX_FRAME_SIZE];
    char data[MAX_DATA_SIZE];
    int frame_size;
    int data_size;

   //SEND FILE
    bool read_done = false;
    int buffer_num = 0;
    while (!read_done) {

        //READ PART OF FILE TO BUFFER
        buffer_size = fread(buffer, 1, max_buffer_size, file);
        if (buffer_size == max_buffer_size) {
            char temp[1];
            int next_buffer_size = fread(temp, 1, 1, file);
            if (next_buffer_size == 0) read_done = true;
            int error = fseek(file, -1, SEEK_CUR);
        } else if (buffer_size < max_buffer_size) {
            read_done = true;
        }
        
        window_info_mutex.lock();

        //INITIALIZE THE VARIABLES FOR SLIDING WINDOW
        int seq_count = buffer_size / MAX_DATA_SIZE + ((buffer_size % MAX_DATA_SIZE == 0) ? 0 : 1);
        int seq_num;
        window_sent_time = new time_stamp[window_len];
        window_ack_mask = new bool[window_len];
        bool window_sent_mask[window_len];
        for (int i = 0; i < window_len; i++) {
            window_ack_mask[i] = false;
            window_sent_mask[i] = false;
        }
        lar = -1;
        lfs = lar + window_len;

        window_info_mutex.unlock();
        
        //Send current buffer with sliding window
        bool send_done = false;
        int packet = 0;
        while (!send_done) {

            window_info_mutex.lock();

            //Check window ack mask, shift window if possible
            if (window_ack_mask[0]) {
                int shift = 1;
                for (int i = 1; i < window_len; i++) {
                    if (!window_ack_mask[i]) break;
                    shift += 1;
                }
                for (int i = 0; i < window_len - shift; i++) {
                    window_sent_mask[i] = window_sent_mask[i + shift];
                    window_ack_mask[i] = window_ack_mask[i + shift];
                    window_sent_time[i] = window_sent_time[i + shift];
                }
                for (int i = window_len - shift; i < window_len; i++) {
                    window_sent_mask[i] = false;
                    window_ack_mask[i] = false;
                }
                lar += shift;
                lfs = lar + window_len;
            }
            
            
            
            window_info_mutex.unlock();

            //Send frames that has not been sent or has timed out
            for (int i = 0; i < window_len; i ++) {
                seq_num = lar + i + 1;
                
                if (seq_num < seq_count) {
                ///
               
                    window_info_mutex.lock();
                  ///
                    if (!window_sent_mask[i] || (!window_ack_mask[i] && (elapsed_time(current_time(), window_sent_time[i]) > timeout))) {
                        int buffer_shift = seq_num * MAX_DATA_SIZE;
                        data_size = (buffer_size - buffer_shift < MAX_DATA_SIZE) ? (buffer_size - buffer_shift) : MAX_DATA_SIZE;
                        memcpy(data, buffer + buffer_shift, data_size);
                        
                        bool eot = (seq_num == seq_count - 1) && (read_done);
                        frame_size = create_frame(seq_num, frame, data, data_size, eot);

                        sendto(sockfd, frame, frame_size, 0, 
                                (const struct sockaddr *) &servaddr, sizeof(servaddr));
                                
                        window_sent_mask[i] = true;
                        window_sent_time[i] = current_time();
                    }
                    
                    window_info_mutex.unlock();
                }
            }

            //Move to next buffer if all frames in current buffer has been acked
            if (lar >= seq_count - 1) send_done = true;
        }
         packet++;
         cout<<"Packet " <<packet<< " sent\n";                    

        buffer_num += 1;
        if (read_done) break;
    }
    
    fclose(file);
    delete [] window_ack_mask;
    delete [] window_sent_time;
    recv_thread.detach();
    
    printMD5(fileName);



}//end of run_SR


void run_SaW(){



}//end of run SaW




int main() {

    //INTRO MESSAGE
    cout<<"\nRUNNING CLIENT\n";

    //CONNECT TO THE SERVER
    callserver();
    
    //DISPLAY MENU FOR THE USER
    menu();
    

    return 0;
}