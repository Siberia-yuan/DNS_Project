#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include"message_struct.h"

void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host);

#define BUF_SIZE 65535
char send_buff[BUF_SIZE];

int main(int argc, char *argv[]){
    int sock;
    char *message1;
    int str_len;
    socklen_t adr_sz;
    struct sockaddr_in serv_adr, from_adr;
    if(argc!=4){
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    sock=socket(PF_INET,SOCK_DGRAM,0);

    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_adr.sin_port=htons(atoi(argv[2]));

    struct DNS_UDP_Header *header;
    struct DNS_Query *query;
    //initialization of header
    header=(struct DNS_UDP_Header*)&send_buff;
    header->id = (unsigned short) htons(getpid());//id设为进程标识符
    header->qr = 0; //查询
    header->opcode = 0; //标准查询
    header->aa = 0; //不授权回答
    header->tc = 0; //不可截断
    header->rd = 1; //期望递归
    header->ra = 0; //不可用递归
    header->z = 0; //必须为0
    header->ad = 0;
    header->cd = 0;
    header->rcode = 0;//没有差错
    header->q_count = htons(1); //1个问题
    header->ans_count = 0; 
    header->auth_count = 0;
    header->add_count = 0;
    //initialization of query
    
    unsigned char *qname;
    struct QUESTION *qinfo = NULL;
    qname = (unsigned char*) &send_buff[sizeof(struct DNS_UDP_Header)];
    //strcpy(qname, domainName);//修改域名格式 
    ChangetoDnsNameFormat(qname,(unsigned char*)argv[3]);
    printf("qname:%s\n",qname);
    qinfo = (struct QUESTION*) &send_buff[sizeof(struct DNS_UDP_Header)
    + (strlen((const char*) qname) + 1)]; //qinfo指向问题查询区域的查询类型字段

    qinfo->qtype = htons(1); //查询类型为A
    qinfo->qclass = htons(1); //查询类为1
    unsigned short len=sizeof(struct DNS_UDP_Header)+strlen((const char*) qname)+1+sizeof(struct QUESTION);

    sendto(sock,send_buff,len,0,(struct sockaddr*)&serv_adr,sizeof(serv_adr));
    /*
    for(int i=3;i<argc;i++){
        message1=argv[i];
        sendto(sock,message1,strlen(message1),0,(struct sockaddr*)&serv_adr,sizeof(serv_adr));
        adr_sz=sizeof(from_adr);
        str_len=recvfrom(sock,message,BUF_SIZE,0,(struct sockaddr*)&from_adr,&adr_sz);
        message[str_len]=0;
        printf("Message from server: %s\n", message);
        
    }*/
    close(sock);
    return 0;
}

void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host) {
    int lock = 0, i;
    strcat((char*) host, ".");

    for (i = 0; i < strlen((char*) host); i++) {
        if (host[i] == '.') {
            *dns++ = i - lock;
            for (; lock < i; lock++) {
                *dns++ = host[lock];
            }
            lock++; 
        }
    }
    *dns++ = '\0';
}