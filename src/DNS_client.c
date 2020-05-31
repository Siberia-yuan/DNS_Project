#include<stdio.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include "message_struct.h"

#define SENDPORT 4001

void error_handling(char *err_string);
void sendTcpQuery(char *domainName,int queryType);
char receive_buff[65535];
char send_buff[65535];

int main(int argc, char *argv[]){
    if(argc!=3){
        printf("Usage: %s <query type> <domain name>\n",argv[0]);
        exit(1);
    }
    if(strcmp(argv[1],"A")==0){
        sendTcpQuery(argv[2],1);
    }else if(strcmp(argv[1],"MX")==0){
        sendTcpQuery(argv[2],2);
    }else if(strcmp(argv[1],"EX")==0){
        sendTcpQuery(argv[2],3);
    }else{
        printf("<Query type>: A,MX,TX\n");
        exit(1);
    }
    return 0;
}

void error_handling(char *err_string){
    fputs(err_string,stderr);
    fputc('\n',stderr);
    return;
}

void sendTcpQuery(char *domainName,int queryType){
    struct DNS_Header *header;
    struct DNS_Query *query;
    //initialization of header
    header=(struct DNS_Header*)&send_buff;
    header->id=htons(0x0001);
    header->tag=htons(0x0000);
    header->queryNum=htons(0x0001);
    header->answerNum=htons(0x0000);
    header->authorNum=htons(0x0000);
    header->addNum=htons(0x0000);
    //initialization of query
    /*
    int len=sizeof(domainName)+1;
    query->name=(unsigned char *)malloc(len*sizeof(char));
    memcpy(domainName,query->name,sizeof(len));
    query->qtype=htons((unsigned short)queryType);
    query->qclass=htons(0);
    query=(struct DNS_Query*)&send_buff[sizeof(struct DNS_Header)];
    */
    //int len=sizeof(domainName);
    //query->name=(unsigned char *)malloc(sizeof(char)*len);
    //memcpy(query->name,domainName,len);
    query=(struct DNS_Query*)&send_buff[sizeof(struct DNS_Header)];
    strcpy(query->name,domainName);
    query->qtype=htons((unsigned short)queryType);
    query->qclass=htons(0);

    

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
    printf("Initiating request...\n");

    int idx=0;
    if(read(sock,receive_buff,sizeof(receive_buff)-1)==-1)
        error_handling("read() error");
    
    printf("message form dns server:\n");
    printf("%s\n",receive_buff);
    close(sock);
    return;
}