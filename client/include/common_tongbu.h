#pragma once
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include<QUuid>
#include <map>
#include <cstdlib>
#include <winsock2.h>
#include <time.h>
#include <windows.h>
#include <list>
#include <functional>

#include "common_order.h"

using namespace std;

#define NONBLOCK_NONE 0
#define NONBLOCK_AFTER_SOCKET 1
#define SEPARATE_LENGTH 40

struct file_info{
    char* file_name;
    char* file_md5;
    int fileLen;
    int recvLen;
};


void Myexit(const char* str);
void printerror(const char* str);
void client_init_nocon(int& client_fd);
int my_connect(int& client_fd,struct sockaddr_in& mysock);
int file_size(const char* filename);

bool str2int(const char* str,int & ans);
int gettimeofday(struct timeval *tp, void *tzp);

bool IsDirectory(const char *pDir);
bool DeleteDirectory(const char * DirName);
bool DeleteFile_(const char * DirName);
time_t gettime(const char* filename);

void findDirectory(const char * DirName,Socket_RIO* rio);
void findFile(const char * DirName,Socket_RIO* rio);

string getDeviceFingerPrint();

