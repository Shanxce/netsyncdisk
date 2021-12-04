#include "../include/common_tongbu_server.h"

void Myexit(const char* str){
	perror(str);
	cout<<"errorno:"<<errno<<endl;
	exit(1);
}

void printerror(const char* str){
	perror(str);
	cout<<"errorno:"<<errno<<endl;
}

/*******************************************
 * 将进程变为守护进程
 * ****************************************/
void init_daemon()
{
	pid_t pc; 
    pc = fork(); /*第一步:创建子进程*/
    if(pc<0){
        exit(1);
    }else if(pc>0){//父进程退出
        exit(0);
    }  
    
    setsid(); /*第二步:在子进程中创建新会话*/
    char szPath[1024];
    if(getcwd(szPath, sizeof(szPath)) == NULL)
    {//获得当前路径
        exit(1);
    }  
    //printf("current working directory : %s\n", szPath);
    //chdir("/"); /*第三步:改变当前目录为根目录*/
    chdir(szPath);
    umask(0); /*第四步:重设文件权限掩码*/
    //for(i=0;i<MAXFILE;i++) /*第五步:关闭文件描述符*/                                                                                               
     //   close(i);
}


/*************************
 * 设置非阻塞模式
 * **********************/
void SETNONBLOCK(int& fd){
	int flags = fcntl(fd, F_GETFL, 0);	  		//获取文件的flags值。
	if(flags==-1){
		Myexit("set nonblocks error");
	}
	int flags_ = fcntl(fd, F_SETFL, flags | O_NONBLOCK); 	//设置成非阻塞模式；
	if(flags_==-1){
		Myexit("set nonblocks error");
	}
	// cout<<"set nonblocks"<<endl;
}

/*************************
 * 服务端初始化，不accept
 * server_fd 文件描述符，connection 客户端描述符
 * link_port 端口，ipaddr ip地址
 * nonblock_time表示设置非阻塞的时间
 * **********************/
void server_init_noacc(int & server_fd, int link_port,const char* ipaddr ,int nonblock_time){
    int flag = 1;
	struct sockaddr_in tcpserveraddr;
    
	//创建套接字
	server_fd= socket(AF_INET, SOCK_STREAM, 0);
	int error = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
	if (error< 0)
	{
		printerror("setsockopt(SO_REUSEADDR) failed");
	}
    // cout<<"server set SO_REUSEADDR success"<<endl;

	if(nonblock_time==NONBLOCK_AFTER_SOCKET){
		SETNONBLOCK(server_fd);
	}
	
	//绑定套接字到一个IP地址和一个端口上
	tcpserveraddr.sin_family = AF_INET;
	tcpserveraddr.sin_port = htons(link_port);
	tcpserveraddr.sin_addr.s_addr = inet_addr(ipaddr);
	int bind_return = bind(server_fd, (struct sockaddr *)&tcpserveraddr, sizeof(tcpserveraddr));
	if (bind_return== -1)
	{
		Myexit("server bind error");
	}

	//将套接字设置为监听模式等待连接请求
	if (listen(server_fd, 20) == -1)
	{
		Myexit("listen error");
	}
	cout<<"listen ready!"<<endl;
}


void my_accept(int server_fd, int& connect_fd, struct sockaddr_in& client_addr, socklen_t& length){
    connect_fd = accept(server_fd, (struct sockaddr *)&client_addr, &length);
    if (connect_fd < 0) {
        Myexit("accept error");
    }
    cout << "client connect, client IP:" << inet_ntoa(client_addr.sin_addr) << ",port:" << ntohs(client_addr.sin_port) << endl;
    SETNONBLOCK(connect_fd);
}

void my_epoll_ctl(int& epfd,int option,int & connect_fd,struct epoll_event* event){
    int ctl_flag = epoll_ctl(epfd, option, connect_fd, event);
    if(ctl_flag==-1){
        Myexit("epoll_ctl error");
    }
}


int file_size(const char* filename)  
{  
    FILE *fp=fopen(filename,"rb");  
    if(!fp){
        cout<<"文件打开失败"<<endl;
        exit(-1);
    } 
    fseek(fp,0L,SEEK_END);  
    int size=ftell(fp);  
    fclose(fp);  
    return size;  
}  

/*******************************************
 * str输入字符串
 * ans答案
 * 返回值为0表示非数字，1表示正确
 * ****************************************/
bool str2int(const char* str,int & ans){
    ans=0;
    int wz=0;
    bool flag=0;//是否负数
    if(str[0]=='-'){
        flag=1;
        wz=1;
    }
    while(str[wz]){
        if(str[wz]<'0' || str[wz]>'9') return 0;//非数字，错误
        ans=ans*10+str[wz]-'0';
        wz++;
    }
    if(flag) ans=-ans;
    return 1;
}

void Getfilepath(const char *path, const char *filename, char *filepath)
{
    strcpy(filepath, path);
    if (filepath[strlen(path) - 1] != '/')
        strcat(filepath, "/");
    strcat(filepath, filename);
    // printf("path is = %s\n", filepath);
}

bool IsDirectory(const char *path){
    struct stat statbuf;
    lstat(path, &statbuf);

    if (S_ISREG(statbuf.st_mode)) //判断是否是常规文件
    {
        return false;
    }
    return true;
}

bool DeleteFile_(const char *path)
{
    DIR *dir;
    struct dirent *dirinfo;
    struct stat statbuf;
    char filepath[256] = {0};
    lstat(path, &statbuf);

    if (S_ISREG(statbuf.st_mode)) //判断是否是常规文件
    {
        remove(path);
    }
    else if (S_ISDIR(statbuf.st_mode)) //判断是否是目录
    {
        if ((dir = opendir(path)) == NULL)
            return 1;
        while ((dirinfo = readdir(dir)) != NULL)
        {
            Getfilepath(path, dirinfo->d_name, filepath);
            if (strcmp(dirinfo->d_name, ".") == 0 || strcmp(dirinfo->d_name, "..") == 0) //判断是否是特殊目录
                continue;
            DeleteFile_(filepath);
            rmdir(filepath);
        }
        closedir(dir);
        rmdir(path);
    }
    return 0;
}

long gettime(const char * filename){
    struct stat buf;
    lstat(filename, &buf);
    return buf.st_mtime;
}


void addr_change_normal(char* addr){
    int wz=0;
    while(addr[wz]){
        if(addr[wz]=='\\') addr[wz]='/';
        wz++;
    }
}


void printTime(time_t now,ofstream& fout){
    int unixTime = (int)time(&now);
    time_t tick = (time_t)unixTime;  
    struct tm tm;   
    char s[100];  
    tm = *localtime(&tick);  
    strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);  
    fout<<"["<<s<<"]  ";   
}
