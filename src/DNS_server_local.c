#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include "message_struct.h"

void error_handling(char* message);
int check_cache(int clnt_sock,char* hostName,int queryMethod);
//void sendSubDNS(){}
char send_buff[]="hello world\n";

int main(int argc,char *argv[]){
    int serv_sock;
    int clnt_sock;
    socklen_t clnt_addr_size;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;

    char send_buff[]="hello world\n";
    char read_buff[65535];

    if(argc!=2){
        printf("Usage: %s <port>\n",argv[0]);
        exit(1);
    }

    serv_sock=socket(PF_INET,SOCK_STREAM,0);
    if(!serv_sock){
        error_handling("socket() error");
    }

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serv_addr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)
        error_handling("bind() error");

    if(listen(serv_sock,5)==-1)
        error_handling("listen() error");

    clnt_addr_size=sizeof(clnt_addr);
    clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_addr,&clnt_addr_size);
    if(clnt_sock==-1){
        error_handling("accept error\n");
    }

    write(clnt_sock,send_buff,sizeof(send_buff));
    
    //readpart
    int idx=0;
    if(read(clnt_sock,read_buff,sizeof(read_buff)-1)==-1)
        error_handling("read() error");

    struct DNS_Header *queryHeader;
    struct DNS_Query *dnsquery;
    queryHeader=(struct DNS_Header *)&read_buff;
    dnsquery=(struct DNS_Query*)&read_buff[sizeof(struct DNS_Header)];
    printf("received:%d\n",ntohs(queryHeader->id));
    printf("received:%d\n",ntohs(queryHeader->queryNum));
    printf("received:%s\n",dnsquery->name);
    /*
    //收到数据之后开始处理
    if(check_cache()){
        close(clnt_sock);
        close(serv_sock);
        return 0;
    }else{
        //发送数据请求给toplevel
    }
    */
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

/*
int check_cache(int clnt_sock,char* hostName,int queryMethod){

}*/

//void sendSubDNS(){}