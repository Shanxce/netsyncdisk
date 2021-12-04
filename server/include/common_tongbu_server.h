#pragma once
#include <errno.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h> 
#include <sys/epoll.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <map>
#include <signal.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <cstdlib>
using namespace std;


#define NONBLOCK_NONE 0
#define NONBLOCK_AFTER_SOCKET 1

struct file_info{
    char* file_name;
    char* file_md5_name;
    char* file_md5;
    int fileLen;
    int recvLen;
};



void Myexit(const char* str);
void printerror(const char* str);
void init_daemon();
void SETNONBLOCK(int& fd);
void server_init_noacc(int & server_fd, int link_port,const char* ipaddr ,int nonblock_time);
void my_accept(int server_fd, int& connect_fd, struct sockaddr_in& client_addr, socklen_t& length);
void my_epoll_ctl(int& epfd,int option,int & connect_fd,struct epoll_event* event);
int file_size(const char* filename);
bool str2int(const char* str,int & ans);

void Getfilepath(const char *path, const char *filename, char *filepath);
bool DeleteFile_(const char *path);
long gettime(const char * filename);
bool IsDirectory(const char *path);

void addr_change_normal(char* addr);
void printTime(time_t now,ofstream& fout);
