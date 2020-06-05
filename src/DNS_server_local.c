#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include "message_struct.h"

int match(unsigned char *dest,unsigned char *ref);
char* ChangetoURL(char* buf, char* dest);
void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host);
void error_handling(char* message);
//int check_cache(int clnt_sock,unsigned char* hostName,int queryMethod);
int sendUDPQuery(char *destIP,char *destPORT,char *domainName,int queryType);
int defineLocal(char*target);

#define BUF_SIZE 65535
char send_buff[BUF_SIZE];
char recv_buff[BUF_SIZE];
char write_buff[BUF_SIZE];
char read_buff[BUF_SIZE];
char *DNSIP="127.0.0.1";
int PORT=4002;
char *rootDNSIP="127.0.0.1";
char *rootPORT="4001"; 

int main(int argc,char *argv[]){
    int serv_sock;
    int clnt_sock;
    socklen_t clnt_addr_size;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;

    serv_sock=socket(PF_INET,SOCK_STREAM,0);
    if(!serv_sock){
        error_handling("socket() error");
    }

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(DNSIP);
    serv_addr.sin_port=htons(PORT);

    if(bind(serv_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)
        error_handling("bind() error");

    while(1){
    if(listen(serv_sock,5)==-1)
        error_handling("listen() error");

    clnt_addr_size=sizeof(clnt_addr);
    clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_addr,&clnt_addr_size);
    if(clnt_sock==-1){
        error_handling("accept error\n");
    }
    //readpart
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
    
    //收到数据之后开始处理
    /*
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
        printf("received udp:%d\n",ntohs(que->qtype));*/
        /*
        while(1){
            
            if(){
                break;
            }

        }*/
        //sendBack requested Data */
    sendUDPQuery(rootDNSIP,rootPORT,(char*)dnsquery,ntohs(que->qtype));
    int flag=1;
    while(1){
        char tempDestDNS[65];
        //memcpy(write_buff,recv_buff,sizeof(recv_buff));
        /*
        if(flag!=1){
            sendUDPQuery(tmpDestDNS,"53",(char*)dnsquery,ntohs(que->qtype));
        }*/
        struct DNS_UDP_Header *recv_header=(struct DNS_UDP_Header *)&recv_buff;
        if (recv_header->rcode == 3) {
            printf("cannot find answer\n");
            break;
        } else printf("find resource data!\n");
    
        int cur = 0;
        cur += sizeof(struct DNS_UDP_Header);

        char *recv_domain=(char *)&recv_buff[cur];
        cur += strlen((const char*) recv_domain)+1;
        cur += sizeof(struct QUESTION);
        unsigned short* name = (unsigned short *)&recv_buff[cur];
        cur += sizeof(unsigned short); // 跳过name
        struct DNS_RR *rr = (struct DNS_RR *)&recv_buff[cur];
        cur += sizeof(struct DNS_RR);

        int type=ntohs(que->qtype);
        // 要从3www5baidu变成www.baidu.com
        char *pdata;
        pdata = recv_buff + cur;
        char url[65];
        char recv_url[65];

        sprintf(tempDestDNS,"%u.%u.%u.%u", (unsigned char)*pdata, (unsigned char)*(pdata + 1), (unsigned char)*(pdata + 2), (unsigned char)*(pdata + 3));
        if(defineLocal(tempDestDNS)==1){
            continue;
        }else{
            break;
        }
        /*
        if((int)*pdata==127){
                tmpDestDNS="";
                strcat(tmpDestDNS,(const char *)itoa((int)*pdata));
                strcat(tmpDestDNS,".");
                strcat(tmpDestDNS,(const char *)itoa((int)*(pdata+1)));
                strcat(tmpDestDNS,".");
                strcat(tmpDestDNS,(const char *)itoa((int)*(pdata+2)));
                strcat(tmpDestDNS,".");
                strcat(tmpDestDNS,(const char *)itoa((int)*(pdata+3)));
        }*/
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
    }
    memcpy(write_buff,recv_buff,sizeof(recv_buff));
    if(write(clnt_sock,(const void*)write_buff,sizeof(write_buff))==-1){
        error_handling("write() error");
    }
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

/*
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
                //ChangetoDnsNameFormat(rdata,(unsigned char*)str);
                strcpy((char *)rdata,str);
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
*/

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

int match(unsigned char *dest,unsigned char *ref) {  // ref是文件里的，短的
    int length=(int)strlen((char *)ref);
    int length1=(int)strlen((char *)dest)-length;
    printf("match: %s, %s\n",dest,ref);

    for(int i=length-1;i>=0;i--){
        if(*(dest+i+length1)<64 && *(dest+i+length1)>0 && *(ref+i)=='.') {
        // if(*(dest+i+length1)=='.' && *(ref+i)=='.') {
            continue;
        }else{
            if(*(dest+i+length1)!=*(ref+i)){
                return 0;
            }
        }
    }
    return 1;
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
    memset(&recv_buff,0,sizeof(recv_buff));
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

int defineLocal(char*target){
    int length=strlen(target);
    if(*target=='1'&&*(target+1)=='2'&&*(target+2)=='7'){
        return 1;
    }else{
        return 0;
    }
}