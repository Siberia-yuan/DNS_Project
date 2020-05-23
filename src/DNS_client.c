#include<stdio.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define SENDPORT 4001
void error_handling(char *err_string);

int main(int argc, char *argv[]){

    char send_buff[]="Client send\n";
    char receive_buff[100];


    int str_len=0, read_len=0;
    if(argc!=3){
        printf("Usage: %s <query type> <domain name>\n",argv[0]);
        exit(1);
    }
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