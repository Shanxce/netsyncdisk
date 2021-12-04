#pragma once
#include <fcntl.h>
#include <fstream>
#include <map>
#include <cstdlib>
#include <time.h>
#include "../include/common_RIO.h"
#include "../include/common_tongbu.h"
#include "../include/common_md5.h"

#define delFile 1
#define addFile 2
#define sendFile 3
#define modifyFileName 4
#define responseSend 5
#define responseBase 6
#define responseLogin 9
#define sendRegister 7
#define sendLogin 8
#define sendUnbundling 10
#define sendBundling 11
#define need_init_synchronous 12

#define YES 1
#define NO 0
#define countWrong -2
#define passwdWrong 0
#define FILE_EXIST 2
#define SENDING 3
#define INIT_END 5

#define SEND_FILE_SIZE 8192


struct REQUEST_BASE{
    int length;
    int id;
    int type;
};
struct DELFILE{
    REQUEST_BASE base;
    char fileMd5[32];
    int fileTime;
    int addrLen;
    char fileAddr[256];
};
struct ADDFILE{
    REQUEST_BASE base;
    char fileMd5[32];
    int fileTime;
    int fileSize;
    int addrLen;
    char fileAddr[256];
};
struct SENDFILE{
    REQUEST_BASE base;
    char fileMd5[32];
    int startPos;
    int endPos;
    int addrLen;
    char fileAddr[256];
};
struct MODIFYNAME{
    REQUEST_BASE base;
    int fileTime;
    int addrLen;
    char fileAddr[256];
    int newaddrLen;
    char newfileAddr[256];
};
struct REGISTER{
    REQUEST_BASE base;
    int passwdLen;
    int usenameLen;
    char passwd[20];
    char username[20];
};
struct LOGIN{
    REQUEST_BASE base;
    int passwdLen;
    int usenameLen;
    int cp_ipLen;
    char passwd[20];
    char username[20];
    char cp_ip[40];
};

struct UNBUNDLING{
    REQUEST_BASE base;
    int addrLen;
    char addr[256];
};
struct BUNDLING{
    REQUEST_BASE base;
    int addrLocalLen;
    char addrLocal[256];
    int addrServerLen;
    char addrServer[256];
};


struct RESPONSE_BASE{
    REQUEST_BASE base;
    int status;
};
struct RESPONSE_SEND{
    REQUEST_BASE base;
    int status;
    int startPos;
    int endPos;
    char content[SEND_FILE_SIZE];
};
struct RESPONSE_LOGIN{
    REQUEST_BASE base;
    int status;
    int shareAddrLen;
    char shareAddr[256];
    int shareAddrLen_server;
    char shareAddr_server[256];
};

union REQUEST{
    REQUEST_BASE base;

    DELFILE delfile;//请求删除文件
    ADDFILE addfile;//请求增加文件
    SENDFILE sendfile;//请求文件内容
    MODIFYNAME modifyname;//请求修改文件名
    REGISTER register_;//请求注册
    LOGIN login;//请求登录

    BUNDLING bundling;//绑定
    UNBUNDLING unbundling;//解除绑定

    RESPONSE_BASE response_base;//普通回复
    RESPONSE_SEND response_send;//请求文件内容回复
    RESPONSE_LOGIN response_login;//登录返回包
};

int real_addfile(const char* filename,Socket_RIO* rio);
void real_delfile(const char* filename,Socket_RIO* rio);
void real_modifyname(const char* filename,char* newfilename,Socket_RIO* rio);
void real_sendfile(int id,Socket_RIO* rio);
int real_register(const char* username, const char* passwd,Socket_RIO* rio);
int real_login(const char* username, const char* passwd,Socket_RIO* rio);
void real_bundling(const char* localAddr,const char* serverAddr,Socket_RIO* rio);
void real_unbundling(const char* addr,Socket_RIO* rio);
void real_need_init(Socket_RIO* rio);


void delfile(REQUEST& response,int fd_now,Socket_RIO* rio);
void addfile(REQUEST& response,int fd_now,Socket_RIO* rio);
int sendfile(REQUEST& response,int client_fd,Socket_RIO* rio);
void modifyfilename(REQUEST& response,int fd_now,Socket_RIO* rio);

void responsesend(REQUEST& response,int fd_now,Socket_RIO* rio);
void responsebase(REQUEST& response,int fd_now);
void responselogin(REQUEST& response,int fd_now);

void init_synchronous(const char* foldername,Socket_RIO* rio);

