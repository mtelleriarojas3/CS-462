#include "includes.h"
#define SA struct sockaddr 
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
    servaddr.sin_port = htons(PORT);
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
    	
    // Close our file and print the MD5 of said file
    fclose(infile);
    printMD5(fileName); 
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
