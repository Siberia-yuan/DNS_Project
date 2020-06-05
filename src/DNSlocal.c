#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include "message_struct.h"

int match(unsigned char *dest,unsigned char *ref);
void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host);
void readurl(char* buf, char* dest);
void error_handling(char* message);
int check_cache(int clnt_sock,unsigned char* hostName,int queryMethod);
void sendUDPQuery(unsigned char *domainName,int queryType,char *ip,char *port);

#define BUF_SIZE 65535
char send_buff[BUF_SIZE];
char recv_buff[BUF_SIZE];
char write_buff[BUF_SIZE];
char read_buff[BUF_SIZE];

int main(int argc,char *argv[]){
    int serv_sock;
    int clnt_sock;
    socklen_t clnt_addr_size;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;

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

    //write(clnt_sock,write_buff,sizeof(write_buff));
    
    //readpart
    int idx=0;
    if(read(clnt_sock,read_buff,sizeof(read_buff)-1)==-1)
        error_handling("read() error");

    struct DNS_Header *queryHeader;
    unsigned char *dnsquery;
    struct QUESTION *que;
    queryHeader=(struct DNS_Header *)&read_buff;
    dnsquery=(unsigned char*)&read_buff[sizeof(struct DNS_Header)];
    que=(struct QUESTION *)&read_buff[sizeof(struct DNS_Header)
    + (strlen((const char*)dnsquery) + 1)];
    printf("received:%s\n",dnsquery);
    printf("received:%d\n",ntohs(que->qclass));
    printf("received:%d\n",ntohs(que->qtype));

    /*
    char *dnsbuff;
    memcpy(dnsbuff,dnsquery,strlen((const char*)dnsquery)+1);
    char *convDest;
    readurl(dnsbuff,convDest);*/
    /*
    char *test2;
    readurl((char *)dnsquery,test2);
    char *test1=".com";
    printf("matchresult:%d",match((unsigned char*)test2,(unsigned char*)test1));*/
    
    //收到数据之后开始处理
    if(check_cache(clnt_sock,dnsquery,que->qtype)){
        close(clnt_sock);
        close(serv_sock);
        return 0;
    }else{
        //发送数据请求给toplevel
        char* ip="127.0.0.1";
        char* port="4002";
        //sendUDPQuery
        sendUDPQuery(dnsquery,1,ip,port);
        struct DNS_UDP_Header *header=(struct DNS_UDP_Header *)&recv_buff;
        dnsquery=(unsigned char*)&recv_buff[sizeof(struct DNS_UDP_Header)];
        que=(struct QUESTION *)&recv_buff[sizeof(struct DNS_UDP_Header)+(strlen((const char*)dnsquery)+1)];
        printf("received udp:%s\n",dnsquery);
        printf("received udp:%d\n",ntohs(que->qclass));
        printf("received udp:%d\n",ntohs(que->qtype));
    }
    
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

int check_cache(int clnt_sock,unsigned char* hostName,int queryMethod){
    char* filename = "../cache/local_cache";
    FILE *fp=fopen(filename,"r");
    if(fp==NULL) {
        printf("file open error\n");
        return -1;
    }
    printf("open success\n");
    char str[11];
    int count=0;
    while(!feof(fp)) {
        if(count==0){
            fscanf(fp,"%[^,]%*c",str);
            count++;
            if(match(hostName,(unsigned char*)str)){
                struct DNS_RR *rrResponse;
                memset(&write_buff,0,sizeof(write_buff));
                unsigned char *dnsName;
                dnsName=(unsigned char*)&write_buff;
                ChangetoDnsNameFormat(dnsName,hostName);
                rrResponse=(struct DNS_RR*)&write_buff[strlen((const char*)dnsName)+1];
                rrResponse->data_len=0;
                fscanf(fp,"%[^,]%*c",str);//ttl
                count++;
                rrResponse->ttl=htons(atoi(str));
                fscanf(fp,"%[^,]%*c",str);//IN
                count++;
                fscanf(fp,"%[^,]%*c",str);//A
                count++;
                rrResponse->type=1;
                rrResponse->_class=1;
                fscanf(fp,"%[^,]%*c",str);//responding addr
                count++;
                unsigned char* rdata=(unsigned char*)&write_buff[strlen((const char*)dnsName)+1+sizeof(struct DNS_RR)];
                ChangetoDnsNameFormat(rdata,(unsigned char*)str);
                rrResponse->data_len=strlen((const char*)dnsName)+1+sizeof(struct DNS_RR)+strlen((const char*)rdata);
                write(clnt_sock,write_buff,sizeof(write_buff));
                return 1;//finding result
            }
        }
        count++;
        if (count>4){
            count = 0;  // 到新的一行了
            fgetc(fp);  // 去除换行符
        }
    }
    //No result
    fclose(fp);
    return 0;
}

void sendUDPQuery(unsigned char *domainName,int queryType,char *ip,char *port){
    int sock;
    char *message1;
    int str_len;
    socklen_t adr_sz;
    struct sockaddr_in serv_adr, from_adr;
    sock=socket(PF_INET,SOCK_DGRAM,0);

    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=inet_addr(ip);
    serv_adr.sin_port=htons(atoi(port));

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
    strcpy((char *)qname, (char *)domainName);//修改域名格式 
    //ChangetoDnsNameFormat(qname,(unsigned char*)argv[3]);
    printf("qname:%s\n",qname);
    qinfo = (struct QUESTION*) &send_buff[sizeof(struct DNS_UDP_Header)
    + (strlen((const char*) qname) + 1)]; //qinfo指向问题查询区域的查询类型字段

    qinfo->qtype = htons(1); //查询类型为A
    qinfo->qclass = htons(1); //查询类为1
    unsigned short len=sizeof(struct DNS_UDP_Header)+strlen((const char*) qname)+1+sizeof(struct QUESTION);
    sendto(sock,send_buff,len,0,(struct sockaddr*)&serv_adr,sizeof(serv_adr));
    memset(&recv_buff,sizeof(recv_buff),0);
    str_len=recvfrom(sock,recv_buff,BUF_SIZE,0,(struct sockaddr*)&from_adr,&adr_sz);
    /*
    header=(struct DNS_UDP_Header *)&recv_buff;
    qname=(unsigned char*)&recv_buff[sizeof(struct DNS_UDP_Header)];
    qinfo=(struct QUESTION *)&recv_buff[sizeof(struct DNS_UDP_Header)+(strlen((const char*)qname)+1)];
    printf("received udp:%s\n",qname);
    printf("received udp:%d\n",ntohs(qinfo->qclass));
    printf("received udp:%d\n",ntohs(qinfo->qtype));*/
    close(sock);
    return;
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

void readurl(char* buf, char* dest)
{
    int len = strlen(buf);
    int i = 0, j = 0, k = 0;
    while (i < len)
    {
        if (buf[i] > 0 && buf[i] <= 63) //����Ǹ�����
        {
            for (j = buf[i], i++; j > 0; j--, i++, k++) //j�Ǽ����Ǽ���k��Ŀ��λ���±꣬i�Ǳ�������±�
                dest[k] = buf[i];
        }

        if (buf[i] != 0)    //���û��������dest��Ӹ�'.'
        {
            dest[k] = '.';
            k++;
        }
    }
    dest[k] = '\0';
}

int match(unsigned char *dest,unsigned char *ref){
    int length=(int)strlen((char *)ref);
    int length1=(int)strlen((char *)dest)-length;

    for(int i=length-1;i>=0;i--){
        if(*(dest+i+length1)=='.'&&*(ref+i)=='.'){

            //(isdigit(*(dest+i))||*(dest+i)=='.')&&(isdigit(*(ref+i))||*(ref+i)=='.')
        }else{
            if(*(dest+i+length1)!=*(ref+i)){
                printf("%c %c\n",*(dest+i),*(ref+i));
                return 0;
            }   
        }
    }
    return 1;
}