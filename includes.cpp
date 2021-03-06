#include "includes.h"

using namespace std;
using namespace std::chrono;

//PRINT MD5
void printMD5(char *fileName){
	string file = fileName;
	string md5file = "md5sum "+file;
	const char *actualmd5 = md5file.c_str();
	
	cout<<"\nMD5:\n";

	system(actualmd5);
	cout<<"\n";
}

bool read_frame(int *seq_num, char *data, int *data_size, bool *eot, char *frame) {
    *eot = frame[0] == 0x0 ? true : false;

    uint32_t net_seq_num;
    memcpy(&net_seq_num, frame + 1, 4);
    *seq_num = ntohl(net_seq_num);

    uint32_t net_data_size;
    memcpy(&net_data_size, frame + 5, 4);
    *data_size = ntohl(net_data_size);

    memcpy(data, frame + 9, *data_size);

    return frame[*data_size + 9] != checksum(frame, *data_size + (int) 9);
}


bool read_ack(int *seq_num, bool *neg, char *ack) {
    *neg = ack[0] == 0x0 ? true : false;

    uint32_t net_seq_num;
    memcpy(&net_seq_num, ack + 1, 4);
    *seq_num = ntohl(net_seq_num);

    return ack[5] != checksum(ack, ACKSIZE - (int) 1);
}


int create_frame(int seq_num, char *frame, char *data, int data_size, bool eot) {
    frame[0] = eot ? 0x0 : 0x1;
    uint32_t net_seq_num = htonl(seq_num);
    uint32_t net_data_size = htonl(data_size);
    memcpy(frame + 1, &net_seq_num, 4);
    memcpy(frame + 5, &net_data_size, 4);
    memcpy(frame + 9, data, data_size);
    frame[data_size + 9] = checksum(frame, data_size + (int) 9);

    return data_size + (int)10;
}

void create_ack(int seq_num, char *ack, bool error) {
    ack[0] = error ? 0x0 : 0x1;
    uint32_t net_seq_num = htonl(seq_num);
    memcpy(ack + 1, &net_seq_num, 4);
    ack[5] = checksum(ack, ACKSIZE - (int) 1);
}


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

