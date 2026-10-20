#include "main.h"

static int net_init(void) {
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
}


bit8 datademo[MAX_MEM] = {'p','i','n','g'};


int main(int argc, char * argv[]){



    if(argc != 4){
        printf("plz input desIP l n\n");
        exit(1);
    }


    if (!net_init()) 
    {
        printf("Winsock failed to initialize\n");    
        exit(1);
    }


    SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == INVALID_SOCKET) {
        printf("socket invalid: %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }

    
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port   = 0;                    // 原始 ICMP 套接字端口无实际意义
    InetPton(AF_INET, argv[1], &dest.sin_addr);  // 这里填本机 IP

    // if (bind(sock, (struct sockaddr*)&dest, sizeof(dest)) == SOCKET_ERROR) {
    //     printf("bind failed: %d\n", WSAGetLastError());
    //     closesocket(sock);
    //     WSACleanup();
    //     exit(1);
    // }


    int timeout_ms = 100;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(const char *)&timeout_ms, sizeof(timeout_ms));


    bit32 l = atoi(argv[2]);
    bit32 n = atoi(argv[3]);
    bit8 counter_send = 0;
    bit8 counter_receive = 0;
    


    for(bit16 i = 0; i < n; i++)
    {
        icmp * echoicmp = (icmp *)malloc(sizeof(icmp) + l + 4);
        if(!echoicmp) {printf("malloc failed!!!\n");    exit(1);};


        bit16 id_send = (bit16)GetCurrentProcessId();
        bit16 seq_send = i;
        if(Init_icmp(echoicmp, echo, 0, 0, id_send, seq_send, datademo,l) == true)
        {

            if(Send_icmp(echoicmp, sock, l, dest) == true)
            {
            bit32 clock_send = (bit32)GetTickCount64();
            counter_send += 1;

            if(Recv_icmp(echoicmp, sock) == false)
            {
                printf("seq: %u ping unsucessfully\n", seq_send);
                continue;
            }

            bit32 time_diff = (bit32)GetTickCount64() - clock_send;
            printf("seq: %u ping sucessfully , used %u ms\n", seq_send, time_diff);
            counter_receive++;
            }
        }

        

    }

    printf("sent: %u\nreceived: %u\n", counter_send, counter_receive);
    if(counter_receive == n){
        printf("loss rate: 0%%\n");
    }else{
        float a = (float)n;
        float b = (float)counter_receive;
        float loss_rate = (a - b) / a * 100;
        printf("loss rate: %d%%\n", (int)loss_rate);
    }

    return 0;
}



bit8 Init_icmp(icmp * icmp, bit8 type, bit8 code, bit16 checksum, bit16 id, bit16 seq, bit8 data[], bit32 num_of_bit8)
{
    icmp->type = type;
    icmp->code = code;
    icmp->checksum = checksum;
    icmp->id = htons(id);
    icmp->seq = htons(seq);

    for(bit32 i = 0; i < num_of_bit8; i++){
        icmp->data[i] = data[i];
    }

    icmp->checksum = htons(Check_icmp(icmp, num_of_bit8));
    

    return true;
}


bit16 Check_icmp(icmp * icmp, bit32 num_of_bit8)
{
    bit32 i;
    bit32 sum = 0;
    bit32 length = sizeof(icmp) + num_of_bit8;
    bit8 * p = (bit8*)icmp;

    for(i = 0; i + 1 < length; i+=2)
    {
        sum += ((bit16)p[i] << 8) | p[i+1];
    }

    if(i < length)
    {
        sum += (bit16)p[i] << 8;
    }

    while(sum > 0xFFFF)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (bit16)(~sum);
}

bit8 Send_icmp(icmp * icmp, SOCKET sock, bit32 l, struct sockaddr_in dest)
{
    bit32 sent = sendto(sock, (char*)icmp, sizeof(icmp) + l, 0, (struct sockaddr *)&dest, sizeof(dest));//这一串看不懂，AI写的
    if(sent == SOCKET_ERROR)
    {
        printf("send failed\t");
        return false;
    }else{
        printf("have sent %d bytes\t",sent);        
        return true;
    }

}

bit8 Recv_icmp(icmp *echo, SOCKET sock)
{
    bit32 time_start = GetTickCount64();
    while(GetTickCount64() - time_start < MAX_TIME)
    {
        char recvbuf[MAX_MEM];
        bit32 num_of_reply_bytes = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
        if(num_of_reply_bytes == SOCKET_ERROR)
        {   
            continue;
        }

        bit32 ip_len = (recvbuf[0] & 0x0F) * 4; //取ip头长度
        icmp * echo_reply = (icmp *)(recvbuf + ip_len);  //跳过ip头

        if (echo_reply->type == reply && echo_reply->id  == echo->id && echo_reply->seq == echo->seq) {
            printf("received successfully\t");
            return true;
        } else {
            printf("skipped one\t");
            continue;
        }
    }
    printf("timeout\t");
    return false;

}