#include<stdio.h>
#include<stdlib.h>
#include<winsock2.h>
#include<ws2tcpip.h>

typedef unsigned char bit8;
typedef unsigned short bit16;
typedef unsigned int bit32;

#define MAX_MEM 1024
#define MAX_TIME 5000

#pragma pack(push,1)

typedef struct
{
    bit8 type;
    bit8 code;
    bit16 checksum;
    bit16 id;
    bit16 seq;
    bit8 data[];
} icmp;

#pragma pack(pop)

enum type
{
    reply = 0,
    echo = 8
};

enum bool
{
    true = 0,
    false = 1
};

int main(int argc, char* argv[]);
//此处type依据请求还是接受，code恒为0，num_of_bit8为数据字节数，注意需要转换大小端
bit8 Init_icmp(icmp * icmp, bit8 type, bit8 code, bit16 checksum, bit16 id, bit16 seq, bit8 data[], bit32 num_of_bit8);
bit16 Check_icmp(icmp * icmp, bit32 num_of_bit8);
bit8 Send_icmp(icmp * icmp, SOCKET sock, bit32 l, struct sockaddr_in dest);
bit8 Recv_icmp(icmp * echo, SOCKET sock);
void Show_icmp(icmp * icmp);//debug


