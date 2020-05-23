#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "message_struct.h"

void error_handling(char *err_string);
int main(int argc,char *argv[]){
    


}

void error_handling(char *err_string){
    fputs(err_string,stderr);
    fputc('\n',stderr);
    return;
}