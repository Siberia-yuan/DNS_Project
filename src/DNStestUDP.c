#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include"message_struct.h"

void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host);
char* ChangetoURL(char* buf, char* dest);
int sendUDPQuery(char *destIP,char *destPORT,char *domainName,int queryType);

#define BUF_SIZE 65535
char send_buff[BUF_SIZE];
char recv_buff[BUF_SIZE];



int main(int argc, char *argv[]){
    int sock;
    char *message1;
    int str_len;
    socklen_t adr_sz, clnt_adr_sz;
    struct sockaddr_in serv_adr, recv_adr, clnt_adr;
    if(argc!=5){
        printf("Usage: %s <ip> <port> <domainName> <type>\n", argv[0]);
        exit(1);
    }
    sock=socket(PF_INET,SOCK_DGRAM,0);

    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_adr.sin_port=htons(atoi(argv[2]));

    int type = atoi(argv[4]);//查询类型 A：1，CNAME：5，MX：15


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
    // printf("qname:%s\n",qname);
    qinfo = (struct QUESTION*) &send_buff[sizeof(struct DNS_UDP_Header)
    + (strlen((const char*) qname) + 1)]; //qinfo指向问题查询区域的查询类型字段

    qinfo->qtype = htons(type);
    qinfo->qclass = htons(1); //查询类为1
    unsigned short len=sizeof(struct DNS_UDP_Header)+strlen((const char*) qname)+1+sizeof(struct QUESTION);

    sendto(sock,send_buff,len,0,(struct sockaddr*)&serv_adr,sizeof(serv_adr));

    // 接受服务器
    unsigned int recv_size = sizeof(serv_adr);
    int recvMsgSize;
    recvMsgSize = recvfrom(sock,recv_buff,BUF_SIZE,0,(struct sockaddr*)&serv_adr,&recv_size);
    if (recvMsgSize < 0) {
        printf("recvfrom() failed.\n");
        exit(1);
    }
    struct DNS_UDP_Header *recv_header=(struct DNS_UDP_Header *)&recv_buff;

    // 没找到RR
    if (recvMsgSize == len && recv_header->rcode == 3) {
        printf("cannot find answer\n");
        return 0;
    } else printf("find resource data!\n");

    int cur = 0;
    cur += sizeof(struct DNS_UDP_Header);

    char *recv_domain=(char *)&recv_buff[cur];
    cur += strlen((const char*) qname)+1;
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

        // if (buf[i] != 0)    //如果没结束就在dest里加个'.'
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

int sendUDPQuery(char *destIP,char *destPORT,char *domainName,int queryType){
        int sock;
    char *message1;
    int str_len;
    socklen_t adr_sz, clnt_adr_sz;
    struct sockaddr_in serv_adr, recv_adr, clnt_adr;

    sock=socket(PF_INET,SOCK_DGRAM,0);

    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=inet_addr(destIP);
    serv_adr.sin_port=htons(atoi(destPORT));

    int type = queryType;//查询类型 A：1，CNAME：5，MX：15


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
    ChangetoDnsNameFormat(qname,(unsigned char*)domainName);
    // printf("qname:%s\n",qname);
    qinfo = (struct QUESTION*) &send_buff[sizeof(struct DNS_UDP_Header)
    + (strlen((const char*) qname) + 1)]; //qinfo指向问题查询区域的查询类型字段

    qinfo->qtype = htons(type);
    qinfo->qclass = htons(1); //查询类为1
    unsigned short len=sizeof(struct DNS_UDP_Header)+strlen((const char*) qname)+1+sizeof(struct QUESTION);

    sendto(sock,send_buff,len,0,(struct sockaddr*)&serv_adr,sizeof(serv_adr));

    // 接受服务器
    unsigned int recv_size = sizeof(serv_adr);
    int recvMsgSize;
    recvMsgSize = recvfrom(sock,recv_buff,BUF_SIZE,0,(struct sockaddr*)&serv_adr,&recv_size);
    if (recvMsgSize < 0) {
        printf("recvfrom() failed.\n");
        exit(1);
    }
    struct DNS_UDP_Header *recv_header=(struct DNS_UDP_Header *)&recv_buff;

    // 没找到RR
    if (recvMsgSize == len && recv_header->rcode == 3) {
        printf("cannot find answer\n");
        return -1;
    } else printf("find resource data!\n");

    int cur = 0;
    cur += sizeof(struct DNS_UDP_Header);

    char *recv_domain=(char *)&recv_buff[cur];
    cur += strlen((const char*) qname)+1;
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
    close(sock);
    return 0;
}