#include<stdio.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>


#define SENDPORT 4001
void error_handling(char *err_string);

struct DNS_RR{
    unsigned char *name;
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
    unsigned char *rdata;
};

struct DNS_Header{
    unsigned short id;
    unsigned short tag;     // 包含QR到Rcode的定义
    unsigned short queryNum;
    unsigned short answerNum;
    unsigned short authorNum;
    unsigned short addNum;
};

struct DNS_Query{
    unsigned char *name;
    unsigned short qtype;
    unsigned short qclass;
};


int main(int argc, char *argv[]){

    
    char receive_buff[100];

    if(argc!=3){
        printf("Usage: %s <query type> <domain name>\n",argv[0]);
        exit(1);
    }

    struct DNS_Header header;
    struct DNS_Query query;
    //initialization of header
    
    header.id=htons(0x0001);
    header.tag=htons(0x0000);
    header.queryNum=htons(0x0001);
    header.answerNum=htons(0x0000);
    header.authorNum=htons(0x0000);
    header.addNum=htons(0x0000);
    //initialization of query
    /*
    query.name=(unsigned char*)argv[2];
    query.qtype=(unsigned short)argv[1];
    query.qclass=(unsigned short)"IN";
    */
    char send_buff[200];
    
    memcpy(send_buff,&header,sizeof(header));
    //memcpy(send_buff+sizeof(header)-1,&query,sizeof(query));
    
    int sock;
    struct sockaddr_in dns_local;
    memset(&dns_local,0,sizeof(dns_local));
    dns_local.sin_family=AF_INET;
    dns_local.sin_addr.s_addr=inet_addr("127.0.0.1");
    dns_local.sin_port=htons(SENDPORT);

    sock=socket(PF_INET,SOCK_STREAM,0);
    if(sock==-1){
        error_handling("socket() error");
    }

    if(connect(sock,(struct sockaddr *)&dns_local,sizeof(dns_local))==-1){
        error_handling("connect() error");
    }

    if(write(sock,send_buff,sizeof(send_buff))==-1){
        error_handling("write() error");
    }

    printf("here");
    int idx=0;
    if(read(sock,receive_buff,sizeof(receive_buff)-1)==-1)
        error_handling("read() error");
    printf("message form dns server:\n");
    printf("%s\n",receive_buff);
    close(sock);
    return 0;
}

void error_handling(char *err_string){
    fputs(err_string,stderr);
    fputc('\n',stderr);
    return;
}