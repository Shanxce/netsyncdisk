
#include "../include/common_RIO.h"


using namespace std;
RIO::RIO()
{
    this->buf_head = this->buf_tail = this->buf;
    this->cnt = 0;
}
ssize_t RIO::read_buf(const void * __buf,size_t __nbytes)
{   
    if(this->cnt==RIO_BUFSIZE)
        return 0;
    const char *_buf = (const char *)__buf;
    ssize_t uread = __nbytes;
    if (buf_tail >= buf_head) //可能要分成两次读入
    {
        int readbytes = (size_t)(RIO_BUFSIZE - (buf_tail - buf)) < __nbytes ? RIO_BUFSIZE - (buf_tail - buf) : __nbytes;
        memcpy(buf_tail, _buf, readbytes);
        buf_tail = ((buf_tail - buf) + readbytes) % RIO_BUFSIZE + buf;
        uread -= readbytes;
        this->cnt += readbytes;
        _buf += readbytes;
    }
    if(uread!=0&&this->cnt!=RIO_BUFSIZE)
    {
        int readbytes = buf_head - buf_tail < uread ? buf_head - buf_tail : uread;
        memcpy(buf_tail, _buf, readbytes);
        buf_tail += readbytes;
        uread -= readbytes;
        this->cnt += readbytes;
    }
    return __nbytes - uread;
}
ssize_t RIO::read_fd(int __fd,size_t __nbytes)
{    
    if(this->cnt==RIO_BUFSIZE)
        return 0;
    ssize_t uread = __nbytes;
    // cout << "uread:" << uread <<' '<<RIO_BUFSIZE<< endl;

    if (buf_tail >= buf_head) //可能要分成两次读入
    {
        int readbytes = (size_t)RIO_BUFSIZE - (buf_tail - buf)<__nbytes?
                        RIO_BUFSIZE - (buf_tail - buf):
                        __nbytes;
        while(readbytes>0)
        {
            int ret=recv(__fd, buf_tail, readbytes,0);
            if(ret<0)
            {
                if(errno==EINTR)
                    ret = 0;
                else
                    return -1;
            }
            else if(ret==0)
                break;
            readbytes -= ret;
            this->cnt += ret;
            this->buf_tail += ret;
            uread -= ret;
        }
        buf_tail = (buf_tail - buf) % RIO_BUFSIZE + buf;
    }
    if(uread!=0&&this->cnt!=RIO_BUFSIZE)
    {
        int readbytes = buf_head - buf_tail < uread ? buf_head - buf_tail : uread;
        while(readbytes>0)
        {
            int ret=recv(__fd, buf_tail, readbytes,0);
            if(ret<0)
            {
                if(errno==EINTR)
                    ret = 0;
                else
                    return -1;
            }
            else if(ret==0)
                break;
            readbytes -= ret;
            this->cnt += ret;
            this->buf_tail += ret;
            uread -= ret;
        }
    }
    return __nbytes - uread;
}
ssize_t RIO::write_buf(void * __buf,size_t __nbytes)
{
    if(this->cnt==0)
        return 0;
    char *_buf = (char *)__buf;
    ssize_t uwrite = __nbytes;
    if (buf_tail <= buf_head) //可能要分成两次读入
    {
        int writebytes = (size_t)RIO_BUFSIZE - (buf_head - buf)<__nbytes?
                        RIO_BUFSIZE - (buf_head - buf):
                        __nbytes;
        memcpy((void *)_buf, buf_head, writebytes);
        buf_head = ((buf_head - buf) + writebytes) % RIO_BUFSIZE + buf;
        uwrite -= writebytes;
        this->cnt -= writebytes;
        _buf += writebytes;
    }
    if(uwrite!=0&&this->cnt!=0)
    {
        int writebytes = buf_tail - buf_head < uwrite ? buf_tail - buf_head : uwrite;
        memcpy((void*)_buf, buf_head, writebytes);
        uwrite -= writebytes;
        this->cnt -= writebytes;
        buf_head+= writebytes;
    }
    return __nbytes - uwrite;
}

ssize_t RIO::write_fd(int __fd,size_t __nbytes)
{
    if(this->cnt==0)
        return 0;
    ssize_t uwrite = __nbytes;
    if (buf_tail <= buf_head) //可能要分成两次读入
    {
        int writebytes = (size_t)RIO_BUFSIZE - (buf_head - buf)<__nbytes?
                        RIO_BUFSIZE - (buf_head - buf):
                        __nbytes;
        while(writebytes>0)
        {
            int ret=send(__fd, buf_head, writebytes,0);
            if(ret<=0)
            {
                if(errno==EINTR)
                    ret = 0;
                else
                    return -1;
            }
            writebytes -= ret;
            this->cnt -= ret;
            this->buf_head += ret;
            uwrite -= ret;
        }
        buf_head = (buf_head - buf) % RIO_BUFSIZE + buf;
    }
    if(uwrite!=0&&this->cnt!=0)
    {
        int writebytes = buf_tail - buf_head < uwrite ? buf_tail - buf_head : uwrite;
        while(writebytes>0)
        {
            int ret=send(__fd, buf_head, writebytes,0);
            if(ret<=0)
            {
                if(errno==EINTR)
                    ret = 0;
                else
                    return -1;
            }
            writebytes -= ret;
            this->cnt -= ret;
            this->buf_head += ret;
            uwrite -= ret;
        }
    }
    return __nbytes - uwrite;
}
int RIO::rlen()
{
    return this->cnt;
}
int RIO::wlen()
{
    return RIO_BUFSIZE - this->cnt;
}
bool RIO::peek(int * _i)
{
    if(cnt<4)
        return false;
    char  *_buf = (char *)(_i);
    if(this->buf_head-buf+4<=RIO_BUFSIZE)
        *_i = *((int*)this->buf_head);
    else 
    {
        int prelen = RIO_BUFSIZE - (this->buf_head - buf);
        memcpy(_buf, buf_head, prelen);
        memcpy(_buf + prelen, buf, 4 - prelen);
    }
    return true;
}

Socket_RIO::Socket_RIO(int fd)
{
    this->fd = fd;
}
ssize_t Socket_RIO::read(void *__buf, size_t __nbytes)
{
    //先将所有内容从socket中读出来
    ssize_t ret=this->read_buf.write_buf(__buf, __nbytes);
    //this->read_buf.read_fd(fd, read_buf.wlen());
    return ret;
}
ssize_t Socket_RIO::write(const void *__buf, size_t __nbytes)
{
    ssize_t ret=this->write_buf.read_buf(__buf, __nbytes);
    this->write_buf.write_fd(fd, write_buf.rlen());
    return ret;
}
int Socket_RIO::rlen()
{
    this->read_buf.read_fd(fd, read_buf.wlen());
    return this->read_buf.rlen();
}
int Socket_RIO::wlen()
{
    this->write_buf.write_fd(fd, write_buf.rlen());
    return this->write_buf.wlen();
}
bool Socket_RIO::peek(int * _i)
{
    //this->read_buf.read_fd(fd, read_buf.wlen());
    return this->read_buf.peek(_i);
}

/*
int main()
{
    int readfd = open("test", O_RDONLY);
    int writefd = open("out", O_WRONLY|O_CREAT,0644);
    cout << writefd << endl;
    getchar();
    RIO rio;

    int leftcnt = 50;
    while(leftcnt>0)
    {
        int ret=rio.read_fd(readfd,min(rand()%20,leftcnt));
        cout << "read ret:" << ret << endl;
        leftcnt -= ret;
        int tt;
        rio.peek(&tt);
        const char * t=(const char *)(&tt);
        cout << "peek";
        for (int i = 0; i < 4; ++i)
            cout << t[i] << ' ';
        cout << endl;
        cout << rio.buf_tail - rio.buf << " " << rio.buf_head - rio.buf << ' ' << rio.cnt << endl;
        ret = rio.write_fd(writefd, random());
        cout << "write ret:" << ret << endl;            
        cout << endl;
        cout << rio.buf_tail - rio.buf << " " << rio.buf_head - rio.buf << ' ' << rio.cnt << endl;
        getchar();
    }
    return 0;
}*/
