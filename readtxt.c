#include <stdio.h>
#include<string.h>


int match(unsigned char *dest,unsigned char *ref);

int main() {
    char* filename = "../RR/testRR";
    FILE *fp=fopen(filename,"r");
    if(fp==NULL) {
        printf("file open error\n");
        return -1;
    }
    printf("open success\n");
    char str[11];
    int count=0;
    unsigned char* domainName=(unsigned char*)"www.baidu.com";

    while(!feof(fp)) {
        if (count==0) {     // 每行第一个是待匹配域名
            // 匹配
            fscanf(fp,"%[^,]%*c",str);
            printf("domain name: %s\t",str);
            printf("match result: %d\n",match(domainName,str));
        }
        else {
            fscanf(fp,"%[^,]%*c",str);
            printf("other: %s\n",str);
        }

        count++;
        if (count>4){
            count = 0;  // 到新的一行了
            fgetc(fp);  // 去除换行符
        }
    }
    fclose(fp);
    return 0;
}


int match(unsigned char *dest,unsigned char *ref) {  // ref是文件里的，短的
    int length=(int)strlen((char *)ref);
    int length1=(int)strlen((char *)dest)-length;

    for(int i=length-1;i>=0;i--){
        if(*(dest+i+length1)=='.' && *(ref+i)=='.') {
            continue;
        }else{
            if(*(dest+i+length1)!=*(ref+i)){
                // printf("%c %c\n",*(dest+i),*(ref+i));
                return 0;
            }
        }
    }
    return 1;
}