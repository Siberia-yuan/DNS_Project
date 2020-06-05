#include<stdio.h>
#include<stdlib.h>
int defineLocal(char*target);

int main(){
    char* test="";
    strcat(test,3);

    printf("%s\n",test);
    return 0;
}

int defineLocal(char*target){
    int length=strlen(target);
    if(*target=='1'&&*(target+1)=='2'&&*(target+2)=='7'){
        return 1;
    }else{
        return 0;
    }
}