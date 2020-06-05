#ifndef _MESSAGE_STRUCT
#define _MESSAGE_STRUCT

struct DNS_RR{
    // unsigned short name;
    unsigned short type;
    unsigned short _class;
    unsigned short ttl1;
    unsigned short ttl2;
    unsigned short data_len;
    // unsigned char *rdata;
};

struct DNS_Header{
    unsigned short length;
    unsigned short id; //会话标识
    unsigned char rd :1; // 表示期望递归
    unsigned char tc :1; // 表示可截断的
    unsigned char aa :1; //  表示授权回答
    unsigned char opcode :4;
    unsigned char qr :1; //  查询/响应标志，0为查询，1为响应
    unsigned char rcode :4; //应答码
    unsigned char cd :1;
    unsigned char ad :1;
    unsigned char z :1; //保留值
    unsigned char ra :1; // 表示可用递归
    unsigned short q_count; // 表示查询问题区域节的数量
    unsigned short ans_count; // 表示回答区域的数量
    unsigned short auth_count; // 表示授权区域的数量
    unsigned short add_count; // 表示附加区域的数量
};

struct DNS_UDP_Header{
    unsigned short id; //会话标识
    unsigned char rd :1; // 表示期望递归
    unsigned char tc :1; // 表示可截断的
    unsigned char aa :1; //  表示授权回答
    unsigned char opcode :4;
    unsigned char qr :1; //  查询/响应标志，0为查询，1为响应
    unsigned char rcode :4; //应答码
    unsigned char cd :1;
    unsigned char ad :1;
    unsigned char z :1; //保留值
    unsigned char ra :1; // 表示可用递归
    unsigned short q_count; // 表示查询问题区域节的数量
    unsigned short ans_count; // 表示回答区域的数量
    unsigned short auth_count; // 表示授权区域的数量
    unsigned short add_count; // 表示附加区域的数量
};



struct QUESTION {
    unsigned short qtype;//查询类型
    unsigned short qclass;//查询类
};
struct DNS_Query{
    unsigned char *name;
    struct QUESTION *ques;
};

#endif