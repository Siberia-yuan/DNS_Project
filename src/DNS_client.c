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
    qname = (unsigned char*) &send_buff[sizeof(struct DNS_Header)];

    strcpy(qname, domainName);//修改域名格式 
    qinfo = (struct QUESTION*) &send_buff[sizeof(struct DNS_Header)
    + (strlen((const char*) qname) + 1)]; //qinfo指向问题查询区域的查询类型字段

    qinfo->qtype = htons(queryType); //查询类型为A
    qinfo->qclass = htons(1); //查询类为1


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