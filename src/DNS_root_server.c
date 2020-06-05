#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include"message_struct.h"

#define FILE_PATH #../RR/root_RR.txt
#define BUF_SIZE 65535
void readurl(char* buf, char* dest);
void error_handling(char* message);
char recv_buff[BUF_SIZE];
char send_buff[BUF_SIZE];

int main(int argc, char *argv[]){
    int serv_sock;
    int str_len;
    socklen_t clnt_adr_sz;
    struct sockaddr_in serv_adr, clnt_adr;
    if(argc!=2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(0);
    }
    serv_sock=socket(PF_INET,SOCK_DGRAM,0);
    if(serv_sock==-1){
        error_handling("UDP socket creation error");
    }

    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1){
        error_handling("bind() error");
    }

    while(1){
        clnt_adr_sz=sizeof(clnt_adr);
        str_len=recvfrom(serv_sock,recv_buff,BUF_SIZE,0,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
        struct DNS_UDP_Header *header=(struct DNS_UDP_Header *)&recv_buff;
        char *qname=(char *)&recv_buff[sizeof(struct DNS_UDP_Header)];
        struct QUESTION *ques=(struct QUESTION*)&recv_buff[sizeof(struct DNS_UDP_Header)+strlen(qname)+1];
        //process query
        printf("received:%s\n",qname);
        //echo sendback
        struct DNS_UDP_Header *header1;
        header1=(struct DNS_UDP_Header*)&send_buff;
        memcpy(header1,header,sizeof(struct DNS_UDP_Header));
        char *qname1=(char *)&send_buff[sizeof(struct DNS_UDP_Header)];
        strcpy(qname1,qname);
        struct QUESTION*ques1=(struct QUESTION*)&send_buff[sizeof(struct DNS_UDP_Header)+1+strlen(qname)];
        memcpy(ques1,ques,sizeof(struct QUESTION));
        unsigned short len=sizeof(struct DNS_UDP_Header)+strlen((const char*) qname)+1+sizeof(struct QUESTION);
        sendto(serv_sock,send_buff,len,0,(struct sockaddr*)&clnt_adr,clnt_adr_sz);
    }
    close(serv_sock);
    return 0;


}

void error_handling(char* message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

void readurl(char* buf, char* dest){
    int len = strlen(buf);
    int i = 0, j = 0, k = 0;
    while (i < len)
    {
        if (buf[i] > 0 && buf[i] <= 63) 
        {
            for (j = buf[i], i++; j > 0; j--, i++, k++) 
                dest[k] = buf[i];
        }

        if (buf[i] != 0)  
        {
            dest[k] = '.';
            k++;
        }
    }
    dest[k] = '\0';
}
