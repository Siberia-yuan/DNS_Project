#include<stdio.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include "message_struct.h"


#define SENDPORT 53

void error_handling(char *err_string);
void sendTcpQuery(char *domainName,int queryType);
void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host);
char* ChangetoURL(char* buf, char* dest);

char recv_buff[65535];
char send_buff[65535];
char send_buff1[65535];

int main(int argc, char *argv[]){
    if(argc!=3){
        printf("Usage: %s <query type> <domain name>\n",argv[0]);
        exit(1);
    }
    int type;
    if(strcmp(argv[1],"A")==0){
        type=1;
        sendTcpQuery(argv[2],1);
    }else if(strcmp(argv[1],"MX")==0){
        type=15;
        sendTcpQuery(argv[2],15);
    }else if(strcmp(argv[1],"CNAME")==0){
        type=5;
        sendTcpQuery(argv[2],5);
    }else{
        printf("<Query type>: A,MX,CNAME\n");
        exit(1);
    }
    //processing response...
    int cur = 2;
    struct DNS_UDP_Header *recv_header=(struct DNS_UDP_Header *)&recv_buff[cur];
    if (recv_header->rcode == 3) {
        printf("cannot find answer\n");
        exit(1);
    } else printf("find resource data!\n");

    cur += sizeof(struct DNS_UDP_Header);

    char *recv_domain=(char *)&recv_buff[cur];
    cur += strlen((const char*) recv_domain)+1;
    cur += sizeof(struct QUESTION);

    unsigned short* name = (unsigned short *)&recv_buff[cur];
    cur += sizeof(unsigned short); // 跳过name

    struct DNS_RR *rr = (struct DNS_RR *)&recv_buff[cur];
    cur += sizeof(struct DNS_RR);

    // 要从3www5baidu变成www.baidu.com
    char *pdata;
    pdata = recv_buff + cur;
    char url[65];
    char recv_url[65];
    if (type == 1) {
        printf("received: %u.%u.%u.%u\n", (unsigned char)*pdata, (unsigned char)*(pdata + 1), (unsigned char)*(pdata + 2), (unsigned char)*(pdata + 3));
    }
    else if (type==5) {
        memcpy(recv_url, &(recv_buff[cur]), ntohs(rr->data_len));
        ChangetoURL(recv_url, url);
        printf("received: %s\n",url);
    }
    else if (type==15) {
        memcpy(recv_url, &(recv_buff[cur+sizeof(unsigned short)]), ntohs(rr->data_len)-sizeof(unsigned short)); // 跳过perference
        ChangetoURL(recv_url, url);
        printf("received: %s\n",url);
        if (recv_header->add_count != 0) {  // 有additional answer
            cur = cur + ntohs(rr->data_len);
            cur += sizeof(unsigned short); // 跳过name
            struct DNS_RR *add_rr = (struct DNS_RR *)&recv_buff[cur];
            cur += sizeof(struct DNS_RR);
            pdata = recv_buff + cur;
            printf("received mx ip: %u.%u.%u.%u\n", (unsigned char)*pdata, (unsigned char)*(pdata + 1), (unsigned char)*(pdata + 2), (unsigned char)*(pdata + 3));
        }
        else {
            printf("no MX ip address\n");
        }
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
    // struct DNS_Query *query;
    //initialization of header
    header=(struct DNS_Header*)&send_buff;
    header->length=htons(0);
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

    //strcpy(qname, domainName);//修改域名格式
    ChangetoDnsNameFormat(qname,(unsigned char*)domainName);
    // printf("qname:%s\n",qname);
    qinfo = (struct QUESTION*) &send_buff[sizeof(struct DNS_Header)
    + (strlen((const char*) qname) + 1)]; //qinfo指向问题查询区域的查询类型字段

    qinfo->qtype = htons(queryType); //查询类型为A
    qinfo->qclass = htons(1); //查询类为1
    unsigned short len=sizeof(struct DNS_Header)+strlen((const char*) qname)+1+sizeof(struct QUESTION);
    header->length=htons(len);

    // unsigned short tcp_len = htons(len);
    // memcpy(send_buff1, &tcp_len, sizeof(unsigned short));
    // memcpy(send_buff1+2, &send_buff, sizeof(recv_buff)-2);

    int sock;
    struct sockaddr_in dns_local;
    memset(&dns_local,0,sizeof(dns_local));
    dns_local.sin_family=AF_INET;
    dns_local.sin_addr.s_addr=inet_addr("127.0.0.2");
    dns_local.sin_port=htons(SENDPORT);

    sock=socket(PF_INET,SOCK_STREAM,0);
    if(sock==-1){
        error_handling("socket() error");
    }

    if(connect(sock,(struct sockaddr *)&dns_local,sizeof(dns_local))==-1){
        error_handling("connect() error");
    }

    if(write(sock,send_buff,len+2)==-1){
        error_handling("write() error");
    }
    printf("Initiating request...\n");

    int idx=0;
    if(read(sock,recv_buff,sizeof(recv_buff))==-1)
        error_handling("read() error");
    // printf("received message from server:\n");

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

//从报文buf里读取url到dest里。格式类似3www5baidu3com0
char* ChangetoURL(char* buf, char* dest) {
    int len = strlen((const char*)buf);
    int i = 0, j = 0, k = 0;
    while (i < len)
    {

        if (buf[i] > 0 && buf[i] <= 63) //如果是个计数
        {
            for (j = buf[i], i++; j > 0; j--, i++, k++) //j是计数是几，k是目标位置下标，i是报文里的下标
                dest[k] = buf[i];
            i--;
        }

        //如果没结束就在dest里加个'.'
        if (i<len-1)
        {
            dest[k] = '.';      // segmentation fault
            k++;
        }
        i++;
    }
    dest[k] = '\0';
    return dest;
}