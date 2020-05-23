#include<stdio.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>


void error_handling(char *err_string);

int main(int argc, char *argv[]){

    char message[100];
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
    dns_local.sin_port=htons(53);

    sock=socket(PF_INET,SOCK_STREAM,NULL);
    if(sock==-1){
        error_handling("socket() error");
    }

    if(connect(sock,(struct sockaddr *)&dns_local,sizeof(dns_local))==-1){
        error_handling("connect() error");
    }

    while(read_len=read(sock,&message[idx++],1)){
        if(read_len==-1)
            error_handling("rea() error");
        str_len+=read_len;
    }
    printf("message form dns server:\n");
    printf("%s\n",message);
    close(sock);
    return 0;
}

void error_handling(char *err_string){
    fputs(err_string,stderr);
    fputc('\n',stderr);
    return;
}