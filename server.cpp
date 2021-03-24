#include <netdb.h> 
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

//DECLARING GLOBAL VARIABLES
#define FRAMESIZE 1500    //FRAME SIZE
#define MAXTIMEOUT 1000    //MAX TIMEOUT
#define PORT 9090
#define IP "10.34.40.33" //phoenix1 ip address
#define MAXLINE 1024 
#define SA struct sockaddr
int n, len;
struct sockaddr_in servaddr, cliaddr;
using namespace std;

int setupServerSocket();
int receivePackets();
int fix(int sockfd, unsigned char *file, int packetSize);
void printMD5(const char *fileName);
///////////////////////////////////////////////////////////////////////////////////////////////

int setupServerSocket(){

    int sockfd;
    char buffer[MAXLINE];
    const char *ConnConfirm = "Connected to server successfully!\n";
    int opt = 1;
    
    // CREATING SOCKET FILE DESCRIPTOR 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }else{
         //cout<<"Socket successfully created..\n"; 
    }
 
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
    
    cout<<"No clients connected at the moment...\n";
    
    len = sizeof(cliaddr);  //len is value/resuslt 

        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                (socklen_t*)&len);
    buffer[n] = '\0';
    printf("%s\n", buffer);
    
    
    sendto(sockfd, (const char *)ConnConfirm, strlen(ConnConfirm), 
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
            len);

    memset(buffer, 0, sizeof(buffer));
    
    return sockfd;


}//END OF METHOD



int receivePackets(){


int packetSize;
int totalFileSize;
uint32_t totalSizeTemp;
uint32_t tempPacketSize;
const char *outputFileName = "sample.out";
int packet = 0; 
printf("HERE WE ARE\n");
  int sockfd = setupServerSocket();

  printf("HERE WE ARE AGAIN\n");
  
	//grab the total size of the file
  recvfrom(sockfd, &totalSizeTemp, sizeof(totalSizeTemp), 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                (socklen_t*)&len);
  totalFileSize = ntohl(totalSizeTemp);
  cout<<"TOTAL SIZE: "<<totalFileSize<<"\n"; 
  
	//grab size of the packet that will be sent here
 
  recvfrom(sockfd, &tempPacketSize, sizeof(tempPacketSize), 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
               (socklen_t*)&len);
  packetSize = ntohl(tempPacketSize);
	cout<<"PACKET SIZE: "<<packetSize<<"\n"; 


  unsigned char *file;
	file = (unsigned char*)calloc(packetSize, sizeof(unsigned char));
	//bzero(file, packetSize);

	ofstream fileReceived; 
	fileReceived.open(outputFileName, ios::out | ios::trunc);
	

		while((fix(sockfd, file, packetSize)) > 0){

		cout<<"Rec packet# " <<dec << packet<<"\n";


    if(totalFileSize - packetSize < 0){
		
		packetSize = totalFileSize;
		fileReceived.write((char*)(file), packetSize);

		}else{
  
			fileReceived.write((char*)(file), packetSize);

		}

		totalFileSize -= packetSize;

	
		packet++;
		bzero(file, packetSize);
		

    }//end of while

  cout<<"\n\nReceive Success!\n";
	
	fileReceived.close();

  printMD5(outputFileName); 

return sockfd;

}//END OF METHOD

int fix(int sockfd, unsigned char *file, int packetSize){

int last_size_read = recvfrom(sockfd, file, sizeof(char), 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                (socklen_t*)&len);

int sum_read = last_size_read;

if(sum_read > 0){
        while(sum_read < packetSize && last_size_read != 0){
                (last_size_read = recvfrom(sockfd, (file+sum_read), (sizeof(char)*packetSize)-(sizeof(char)*sum_read), MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len));
                sum_read += last_size_read;
        }
}
  
return sum_read;
}




void printMD5(const char *fileName) {
	string filetbs = fileName;
	string md5file = "md5sum "+filetbs;
	const char *actualmd5 = md5file.c_str();
	
	cout<<"\nMD5:\n";

	system(actualmd5);
	cout<<"\n";
}


//MAIN FUNCTION
int main(){

  //INTRO MESSAGE	
  cout<<"\nRUNNING SERVER ("<<IP<<")\n\n";

  //SOCKET CONNECTION	
  int serverSocket = receivePackets();
  
close(serverSocket);//CLOSE CONNECTION

}//END OF MAIN


