/**
 * 2nd level domain DNS
 * Author: Qincheng Gao
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include"message_struct.h"

#define BUF_SIZE 65535
void error_handling(char* message);
char* searchIP(char* domainName, int type, char* ip);
int match(unsigned char *dest,unsigned char *ref);
int ChangeTypetoInt(char *str);
void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host);

char recv_buff[BUF_SIZE];
char send_buff[BUF_SIZE];

// char* filename = "../RR/testRR";
char* filename = "../RR/gov_RR";
char* serverIP = "127.0.1.4";
int PORT=53;
int en_iter = 0;    // 查询RR文件时type必须匹配

int main(int argc, char *argv[]){
    int serv_sock, clnt_sock;
    int str_len;
    socklen_t clnt_adr_sz;
    struct sockaddr_in serv_adr, clnt_adr;
    // if(argc!=2){
    //     printf("Usage: %s <port>\n", argv[0]);
    //     exit(1);
    // }
    serv_sock=socket(PF_INET,SOCK_DGRAM,0);
    if(serv_sock==-1){
        error_handling("UDP socket creation error");
    }

    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=inet_addr(serverIP);
    // serv_adr.sin_port=htons(atoi(argv[1]));
    serv_adr.sin_port=htons(PORT);

    if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1){
        error_handling("bind() error");
    }
    printf("2LD server start\n");
    printf("===============\n");

    char* ip;
    ip = (char *)malloc(60);
    char* mxip;
    mxip = (char *)malloc(16);

    while(1){
        clnt_adr_sz=sizeof(clnt_adr);
        str_len=recvfrom(serv_sock,recv_buff,BUF_SIZE,0,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
        memcpy(send_buff, recv_buff, str_len);

        struct DNS_UDP_Header *header=(struct DNS_UDP_Header *)&send_buff;
        char *qname=(char *)&send_buff[sizeof(struct DNS_UDP_Header)];
        struct QUESTION *ques=(struct QUESTION*)&send_buff[sizeof(struct DNS_UDP_Header)+strlen(qname)+1];
        printf("received:%s\n",qname);
        unsigned short len=sizeof(struct DNS_UDP_Header)+strlen((const char*) qname)+1+sizeof(struct QUESTION);

        // 域名转换成www.baidu.com
        char ori_url[65];
        memcpy(ori_url, &(send_buff[sizeof(struct DNS_UDP_Header)]), str_len);	//获取请求报文中的域名表示

        unsigned short type1 = ntohs(ques->qtype);
        int type = (int)type1;

        // search RR
        ip = searchIP(ori_url,type,ip);
        printf("resource data: %s\n",ip);

        int found;
        found = strcmp(ip,"");
        if (found == 0) {  // RR里没找到
            printf("cannot find resource data\n");
            header->qr = 1; // 回答
            header->rcode = 3;
            // printf("rcode:%d\n", header->rcode );
            printf("starting sending: cannot find resource data message.\n");
            int send_len = sendto(serv_sock,send_buff,len,0,(struct sockaddr*)&clnt_adr,sizeof(clnt_adr));
            if (send_len<0) {
                printf("send fail\n");
            }
            continue;
        }

        // header
        header->qr = 1; // 回答
        header->ans_count = htons(1);

        // 构造RR
        unsigned short name = htons(0xc00c);//域名指针（偏移量）
        memcpy(send_buff+len, &name, sizeof(unsigned short));
        len += sizeof(unsigned short);

        struct DNS_RR *rrResponse = NULL;
        rrResponse = (struct DNS_RR*) &send_buff[len];
        rrResponse->type = htons(type);
        rrResponse->_class = htons(0x0001);
        rrResponse->ttl1 = htons(0x0000);
        rrResponse->ttl2 = htons(0x012c);

        // resource data: ip地址
        len += sizeof(struct DNS_RR);
        // type=A
        if (type==1) {
            rrResponse->data_len = htons(0x0004);
            unsigned long resource_data = (unsigned long)inet_addr(ip);
            memcpy(send_buff + len, &resource_data, sizeof(unsigned long));
            len += sizeof(unsigned long);
        }
        // type=CNAME
        else if (type==5) {
            unsigned char *cname;
            cname = (unsigned char*) &send_buff[len];
            ChangetoDnsNameFormat(cname,(ip));
            unsigned short cname_len = strlen((const char*)cname)+1;
            rrResponse->data_len = htons(cname_len);
            len += cname_len;
        }
        // type=MX
        else if (type==15) {
            // perference
            unsigned short perference = htons(5);
            memcpy(send_buff+len, &perference, sizeof(unsigned short));
            len += sizeof(unsigned short);
            // mx地址=mail+name指针
            unsigned char *mxname;
            mxname = (unsigned char*) &send_buff[len];
            ChangetoDnsNameFormat(mxname,(ip));
            unsigned short mxname_len = strlen((const char*)mxname)+1;
            rrResponse->data_len = htons(mxname_len + sizeof( unsigned short)); // data length要包含perference的长度
            len += mxname_len;


            mxip = searchIP(mxname,1,mxip);
            int found1;
            found1 = strcmp(mxip,"");
            if (found1 == 0) {  // RR里没找到
                printf("cannot find mx ip\n");

            }
            else {
                header->add_count = htons(1);
                unsigned short name1 = htons(0xc00c); // 域名偏移量 TODO: 改成mx域名的偏移量
                memcpy(send_buff+len, &name1, sizeof(unsigned short));
                len += sizeof(unsigned short);

                struct DNS_RR *rrResponse = NULL;
                rrResponse = (struct DNS_RR*) &send_buff[len];
                rrResponse->type = htons(1);
                rrResponse->_class = htons(0x0001);
                rrResponse->ttl1 = htons(0x0000);
                rrResponse->ttl2 = htons(0x0012c);
                rrResponse->data_len = htons(0x0004);
                len += sizeof(struct DNS_RR);

                unsigned long mxip_data = (unsigned long)inet_addr(mxip);
                memcpy(send_buff + len, &mxip_data, sizeof(unsigned long));
                // printf("resource data-mx ip: %s", mxip);
                len += sizeof(unsigned long);

                // len-=4;
            }
        }

        printf("starting sending\n");
        int send_len = sendto(serv_sock,send_buff,len,0,(struct sockaddr*)&clnt_adr,sizeof(clnt_adr));
        if (send_len<0 ) {
            printf("send fail\n");
        }
        printf("===============\n");

    }

    free(mxip);
    mxip==NULL;
    free(ip);
    ip==NULL;
    close(serv_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
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

char* searchIP(char* domainName, int type, char* resultIP) {
    FILE *fp=fopen(filename,"r");
    if (fp==NULL) {
        printf("file open error\n");
        return "";
    }
    char domain[65],class[65],typeF[6];
    char resource[65] = "";
    int ttl;
    int count=0;
    char c;

    while (!feof(fp)) {
        fscanf(fp,"%s %d %s %s %s\n",&domain,&ttl,&class,&typeF,&resource);
        // printf("str: %s\n",domain);
        // printf("other:%d,%s,%s,%s\n",&ttl,&class,&typeF,&resource);
        if (match(domainName,domain) == 1) {
            int type1 = ChangeTypetoInt(typeF);
            // printf("type:%d,%d\n",type,type1);
            if (type1 == type || en_iter==1) { // 找到对应RR条目 (type也相同)
                strcpy(resultIP,resource);
                break;
            }

        }

    }
    fclose(fp);
    return resultIP;
}

int match(unsigned char *dest,unsigned char *ref) {  // ref是文件里的，短的
    int length=(int)strlen((char *)ref);
    int length1=(int)strlen((char *)dest)-length;
    // printf("match: %s, %s\n",dest,ref);

    for(int i=length-1;i>=0;i--){
        if(*(dest+i+length1)<64 && *(dest+i+length1)>0 && *(ref+i)=='.') {
            continue;
        }else{
            if(*(dest+i+length1)!=*(ref+i)){
                return 0;
            }
        }
    }
    return 1;
}

int ChangeTypetoInt(char *str) {
    if (strcmp(str,"A")==0) {
        return 1;
    }
    else if (strcmp(str,"MX")==0) {
        return 15;
    }
    else if (strcmp(str,"CNAME")==0) {
        return 5;
    }
    else {
        return -1;
    }
}