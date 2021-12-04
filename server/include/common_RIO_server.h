#pragma once
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define RIO_BUFSIZE 81920

class RIO
{
public:
    char *buf_head,*buf_tail;
    int cnt;//buf中已经读入的字节数
    char buf[RIO_BUFSIZE];
    RIO();
    ssize_t read_buf(const void * __buf,size_t __nbytes);
    ssize_t write_buf(void * __buf,size_t __nbytes);
    ssize_t read_fd(int __fd,size_t __nbytes);
    ssize_t write_fd(int __fd,size_t __nbytes);
    int rlen();
    int wlen();
    bool peek(int * _i);
};

class Socket_RIO
{
public:
    int fd;
    
    char user_id[50]; 
    char cp_ip[50];

    RIO read_buf, write_buf;//理解为socket缓冲区的一份拷贝
    Socket_RIO(int fd);
    ssize_t read(void *__buf, size_t __nbytes);
    ssize_t write(const void *__buf, size_t __nbytes);
    int rlen();
    int wlen();
    bool peek(int * _i);
};