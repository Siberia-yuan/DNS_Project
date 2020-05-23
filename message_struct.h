#ifndef _MESSAGE_STRUCT
#define _MESSAGE_STRUCT

struct DNS_RR{
    unsigned char *name;
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
    unsigned char *rdata;
};

struct DNS_Header{
    unsigned short id;
    unsigned short tag;     // 包含QR到Rcode的定义
    unsigned short queryNum;
    unsigned short answerNum;
    unsigned short authorNum;
    unsigned short addNum;
};

struct DNS_Query{
    unsigned char *name;
    unsigned short qtype;
    unsigned short qclass;
};



#endif