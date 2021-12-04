#include "../include/common_tongbu.h"
#include<QDir>
void Myexit(const char* str){
	perror(str);
	cout<<"errorno:"<<errno<<endl;
	exit(1);
}

void printerror(const char* str){
	perror(str);
	cout<<"errorno:"<<errno<<endl;
}


/*************************
 * 服务端初始化，不connect
 * client_fd 文件描述符
 * link_port 端口，myport 自己的端口，ipaddr ip地址
 * nonblock_time表示设置非阻塞的时间
 * **********************/
void client_init_nocon(int& client_fd){
    //加载套接字库，创建套接字
	client_fd = socket(AF_INET,SOCK_STREAM, 0);
    if(client_fd == INVALID_SOCKET){
		Myexit("set socket error");
		exit(-1);
	}
	int flag=1;
    int error = setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(int));
	if (error< 0)
	{
		printerror("setsockopt(SO_REUSEADDR) failed");
	}
    else cout<<"client set SO_REUSEADDR success"<<endl;
}

int my_connect(int& client_fd,struct sockaddr_in& mysock){
	client_init_nocon(client_fd);
    if (connect(client_fd, (struct sockaddr *)&mysock, sizeof(mysock)) < 0) {
        perror("connect error");
        return -1;
    }
    unsigned long uflag = 1;
    int ret = ioctlsocket(client_fd, FIONBIO, &uflag);
    
    if(ret==SOCKET_ERROR)
    {
        Myexit("ioctlsocket NOBLOCK fail!");
    }
    cout<<"connect success"<<endl;
    return 1;
}


int file_size(const char* filename)  
{  
    FILE *fp=fopen(filename,"rb");  
    if(!fp){
        return -1;
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

int gettimeofday(struct timeval *tp, void *tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year     = wtm.wYear - 1900;
    tm.tm_mon     = wtm.wMonth - 1;
    tm.tm_mday     = wtm.wDay;
    tm.tm_hour     = wtm.wHour;
    tm.tm_min     = wtm.wMinute;
    tm.tm_sec     = wtm.wSecond;
    tm. tm_isdst    = -1;
    clock = mktime(&tm);
    tp->tv_sec = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;
    return (0);
}



bool IsDirectory(const char *pDir)
{
    QFileInfo fileinfo(pDir);
    return fileinfo.exists()&&fileinfo.isDir();
}
bool DeleteDirectory(const char * DirName)
{
    QDir dir(DirName);
    if (!dir.exists()) {
        return -1;
    }
  //取到所有的文件和文件名，但是去掉.和..的文件夹（这是QT默认有的）
    dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);
    //文件夹优先
    dir.setSorting(QDir::DirsFirst);
    //转化成一个list
    QFileInfoList list = dir.entryInfoList();
    if(list.size()< 1 ) {
        return -1;
    }
    int i=0;

    //递归算法的核心部分
    do{
        QFileInfo fileInfo = list.at(i);
        QString tt = fileInfo.fileName();
        //如果是文件夹，递归
        bool bisDir = fileInfo.isDir();
        cout<<fileInfo.filePath().toStdString()<<' ' ;
        if(bisDir) {
            DeleteDirectory(fileInfo.filePath().toStdString().c_str());
        }
        else{
               DeleteFileA(fileInfo.filePath().toStdString().c_str());
        }//end else
        i++;
    } while(i < list.size());
    return 0;
}

bool DeleteFile_(const char * DirName){
    if (IsDirectory(DirName)) //如果是目录，则递归地调用
        return DeleteDirectory(DirName);
    else
        return DeleteFileA(DirName);
}

time_t gettime(const char* filename){
    WIN32_FILE_ATTRIBUTE_DATA    attr;     //文件属性结构体
    GetFileAttributesExA(filename,GetFileExInfoStandard,&attr);        //获取文件属性
	FILETIME createTime = attr.ftCreationTime;                    //获取文件时间
	FILETIME accessTime = attr.ftLastAccessTime;             
	FILETIME modifyTime = attr.ftLastWriteTime;
	SYSTEMTIME time;                                                     //系统时间结构体
	FileTimeToSystemTime(&modifyTime,&time);             //将文件事件转换为系统时间
    
	tm tm_;
	tm_.tm_year  = time.wYear -1900;
	tm_.tm_mon   = time.wMonth -1;
	tm_.tm_mday  = time.wDay;
	tm_.tm_hour  = time.wHour;
	tm_.tm_min   = time.wMinute;
	tm_.tm_sec   = time.wSecond;
	tm_.tm_isdst = 0;
	time_t t_ = mktime(&tm_);

    return t_;
}
void findDirectory(const char* sComFilePath,Socket_RIO* rio)
{
    QDir dir(sComFilePath);
    if (!dir.exists()) {
        return ;
    }
  //取到所有的文件和文件名，但是去掉.和..的文件夹（这是QT默认有的）
    dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);
    //文件夹优先
    dir.setSorting(QDir::DirsFirst);
    //转化成一个list
    QFileInfoList list = dir.entryInfoList();
    if(list.size()< 1 ) {
        return ;
    }
    int i=0;

    //递归算法的核心部分
    do{
        QFileInfo fileInfo = list.at(i);
        QString tt = fileInfo.fileName();
        //如果是文件夹，递归
        bool bisDir = fileInfo.isDir();
        cout<<fileInfo.filePath().toStdString().c_str()<<endl;

        int outflag=(real_addfile(fileInfo.filePath().toStdString().c_str(),rio)<=0);



        while(outflag==0){
            int lnow=rio->rlen();
            if(lnow==0){
                continue;
            }
            int length;
            rio->peek(&length);
            if (lnow < length){
                continue;
            }
            REQUEST response;
            while(lnow>=length){
                rio->read(&response, length);
                if(response.base.type==sendFile){
                    cout<<sendFile<<endl;

                    if(sendfile(response,0,rio)==1){
                        outflag=1;
                        break;
                    }
                }
                else if(response.base.type==responseBase){
                    cout<<responseBase<<endl;
                    if(response.response_base.status==FILE_EXIST){
                        outflag=1;
                        break;
                    }
                }
                else if(response.base.type==modifyFileName){
                    cout<<modifyFileName<<endl;
                    modifyfilename(response,0,rio);
                    outflag=1;
                    break;
                }
                else{
                    cout<<response.base.type<<endl;
                    cout<<"error"<<endl;
                    exit(-1);
                }
                lnow-=length;
                rio->peek(&length);
            }
        }//end else

        if(bisDir)
            findDirectory(fileInfo.filePath().toStdString().c_str(),rio);
        i++;
    } while(i < list.size());
    return ;
}

void findFile(const char * DirName,Socket_RIO* rio){
    if (IsDirectory(DirName)) //如果是目录，则递归地调用    
        findDirectory(DirName,rio);
}


bool getDevcieInfo(const char* cmd,list<string> &resultList) {
    char buffer[128];
	bool ret = false;
    FILE* pipe = _popen(cmd, "r"); //打开管道，并执行命令 
    if (!pipe)
        return ret;                     
    
	char *name[20] = {"UUID","ProcessorId","SerialNumber"};
	int len0 = strlen(name[0]),len1 = strlen(name[1]),len2 = strlen(name[2]);
	bool isOk = false;
    while(!feof(pipe)) 
	{
		if(fgets(buffer, 128, pipe))
		{
			if( strncmp(name[0],buffer,len0) == 0 
				|| strncmp(name[1],buffer,len1) == 0 
				|| strncmp(name[2],buffer,len2) == 0 ) // 能够正确获取信息
			{
				isOk = true;
				continue;
			}
            if( isOk ==  false 
				|| strcmp("\r\n",buffer) == 0 ) //去掉windows无用的空行
			{
				continue;
			}
			ret = true;
			resultList.push_back(string(buffer));
        }
    }
    _pclose(pipe); // 关闭管道 
    return ret; 
}


string getDeviceFingerPrint(){
    list<string> strList;
	list<string>::iterator it;
	hash<string> str_hash;
	size_t num ;
	char tmp[11] = {0};
	
	// 主板UUID存在，就使用主板UUID生成机器指纹
    if( getDevcieInfo("wmic csproduct get UUID",strList)
		&& (*strList.begin()).compare("FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF\r\n") != 0)
	{
		num = str_hash(*strList.begin());
		sprintf(tmp,"%u",num);
		//cout << string(tmp) << endl;
		return string(tmp);
    }
    
    // 主板UUID不存在，使用CPUID、BIOS序列号、硬盘序列号生成机器指纹
	string otherStr("");
	strList.clear();
	if( getDevcieInfo("wmic cpu get processorid",strList) ){
		otherStr.append( *strList.begin() );
    }
	strList.clear();
	if( getDevcieInfo("wmic bios get serialnumber",strList) ){
		otherStr.append( *strList.begin() );
    }
	strList.clear();
	if( getDevcieInfo("wmic diskdrive get serialnumber",strList) ){
		string allDiskNum("");
		// 硬盘可能有多块
		for(it = strList.begin();it != strList.end();it++)
		{
			allDiskNum.append(*it);
		}
		otherStr.append( *strList.begin() );
    }
	cout << str_hash(otherStr) << endl;
	num = str_hash(otherStr);
	sprintf(tmp,"%u",num);
	return string(tmp);
}
string getDeviceFingerPrint1()
{
    QUuid id = QUuid::createUuid();

    QString strId = id.toString();
    //qDebug() << strId;
    //输出结果："{b5eddbaf-984f-418e-88eb-cf0b8ff3e775}"

    strId.remove("{").remove("}").remove("-"); // 一般习惯去掉左右花括号和连字符
    //qDebug() << strId;

    string tem=strId.toStdString();
    return tem;
    /*
    hash<string> str_hash;
    size_t num=str_hash(tem);
    qDebug()<<num;*/
}
