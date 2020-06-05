#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include "message_struct.h"

#define BUF_SIZE 65535
char wirte_buff[BUF_SIZE];
int match(unsigned char *dest,unsigned char *ref);
int main(){
    unsigned char* hostName="www.baidu.com";
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
                //struct DNS_RR *rrResponse;
                //memset(&write_buff,0,sizeof(write_buff));
                //unsigned char *dnsName;
                //dnsName=(unsigned char*)&write_buff;
                //ChangetoDnsNameFormat(dnsName,hostName);
                //rrResponse=(struct DNS_RR*)&write_buff[strlen((const char*)dnsName)+1];
                //rrResponse->data_len=0;
                fscanf(fp,"%[^,]%*c",str);//ttl
                count++;
                //rrResponse->ttl=htons(atoi(str));
                fscanf(fp,"%[^,]%*c",str);//IN
                count++;
                fscanf(fp,"%[^,]%*c",str);//A
                count++;
                //rrResponse->type=1;
                //rrResponse->_class=1;
                fscanf(fp,"%[^,]%*c",str);//responding addr
                count++;
                //unsigned char* rdata=(unsigned char*)&write_buff[strlen((const char*)dnsName)+1+sizeof(struct DNS_RR)];
                //ChangetoDnsNameFormat(rdata,(unsigned char*)str);
                //strcpy((char *)rdata,str);
                //rrResponse->data_len=strlen((const char*)dnsName)+1+sizeof(struct DNS_RR)+strlen((const char*)rdata);
                //return 1;//finding result
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
    printf("no result\n");

    return 0;
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