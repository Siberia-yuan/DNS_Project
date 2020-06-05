#include<stdio.h>
#include<stdlib.h>

void ChangetoDnsNameFormat(char* dns, char* host);
void readurl(char* buf, char* dest);
int main(){
    char* OriginName="www.baidu.com";
    char* dest=(char *)malloc(sizeof(OriginName));
    ChangetoDnsNameFormat((unsigned char*)dest,(unsigned char*)OriginName);
    printf("%s\n",dest);
    return 0;
}

void ChangetoDnsNameFormat(char* dns,char* host) {
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