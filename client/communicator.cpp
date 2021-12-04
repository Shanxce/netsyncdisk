#include "communicator.h"
enum STATUS{Login_PasswdFail,Login_UserFail,Login_sucess,Register_fail,Register_sucess,
            Register_Unstrong,Bind_Sourcenoexist,Bind_fail,Connect_dely};

std::atomic<int> Share_ifBind;
map<string,int> No_listen_file;
extern map<int,file_info* > id_file;
int LogfileLens=0;
static void WtoSpath(const WCHAR *Wpath, DWORD pathsize, char *Spath,const char *listen_path,int listen_path_lens)
{

    static WCHAR buf[PATH_LENS];
    memcpy(buf, Wpath, pathsize);
    buf[pathsize / 2] = L'\0';
    memcpy(Spath, listen_path, listen_path_lens);
    int ret=wcstombs(Spath + listen_path_lens, buf, 256);
    Spath[ ret+ listen_path_lens] = '\0';
}

static int Store_listen(const char * path,int pathlen,int ifcheck=false)
{

    string _path=string(path,pathlen);
    for(int i=0;i<pathlen;++i)
        if(_path[i]=='\\')
            _path[i]='/';
    cout<<"ifcheck:"<<ifcheck<<' '<<_path<<endl;
    if(!ifcheck)
    {
        No_listen_file[_path]=time(0)+5;
        return 1;
    }
    else
    {
        if(time(0)<No_listen_file[_path])
            return 0;
        else
        {
            No_listen_file.erase(_path);
            return 1;
        }
    }
}

Communicator::Communicator(QObject *parent) : QObject(parent)
{
    client_fd=-1;
    rio=NULL;
    ifbindaddr=ifconnect=false;
    SyncStatus=0;
}

void Communicator::DealUnfinishEvents(map<string, DWORD> &events, Socket_RIO *rio)
{
    for (auto iter = events.begin(); iter != events.end();)
    {
        const char *path = iter->first.c_str();
        int EventStatus = 0;
        switch (iter->second)
        {
        case FILE_ACTION_ADDED:
            EventStatus = real_addfile(path, rio);

            cout << "       Added: " << path << endl;
            break;
        case FILE_ACTION_MODIFIED:
            real_delfile(path, rio);
            EventStatus = real_addfile(path, rio);
            cout << "    Modified: " << path << endl;
            break;
        default:
            printf("Unknown action!\n");
            break;
        }
        if(EventStatus>=0)
            iter = events.erase(iter);
        else
            ++iter;
        Sleep(100);
    }
}

void Communicator::deal_events(FILE_NOTIFY_INFORMATION *events, Socket_RIO *rio)
{
    FILE_NOTIFY_INFORMATION *event = events;
    static char path[PATH_LENS], oldpath[PATH_LENS];
    while (true)
    {
        //判断该消息是否有后续，如果有则跳过当前
        FILE_NOTIFY_INFORMATION *nextevent = (FILE_NOTIFY_INFORMATION *)(*((uint8_t **)&event) + event->NextEntryOffset);
        if(event->NextEntryOffset
            &&nextevent->FileNameLength==event->FileNameLength
            &&!memcmp(nextevent->FileName,event->FileName,event->FileNameLength))
        {
            event = nextevent;
            continue;
        }
        DEBUG(event->Action);
        DWORD name_len = event->FileNameLength / sizeof(wchar_t);
        int spath_size;
        int EventStatus=0;
        ofstream fout("log",ios::binary | ios::out | ios::app);
        switch (event->Action)
        {
        case FILE_ACTION_ADDED:
            DEBUG("add event");
            WtoSpath(event->FileName, event->FileNameLength, path,listen_path,listen_path_lens);
            if(!Store_listen(path,strlen(path),1))
                break;
            EventStatus=real_addfile(path, rio);
            DEBUG(EventStatus);
            fout<<"event Added:"<<path<<endl;
            wprintf(L"       Added: %.*ls\n", name_len, event->FileName);
            break;
        case FILE_ACTION_REMOVED:
            WtoSpath(event->FileName, event->FileNameLength, path,listen_path,listen_path_lens);
            if(!Store_listen(path,strlen(path),1))
                break;
            real_delfile(path, rio);
            fout<<"event Removed:"<<path<<endl;
            wprintf(L"     Removed: %.*ls\n", name_len, event->FileName);
            break;
        case FILE_ACTION_MODIFIED:
            DEBUG("modify event");
            WtoSpath(event->FileName, event->FileNameLength, path,listen_path,listen_path_lens);
            if(!Store_listen(path,strlen(path),1))
                break;
            if(IsDirectory(path))
                break;
            real_delfile(path, rio);
            EventStatus=real_addfile(path, rio);
            DEBUG(EventStatus);
            fout<<"event Modified:"<<path<<endl;
            wprintf(L"    Modified: %.*ls\n", name_len, event->FileName);
            break;
        case FILE_ACTION_RENAMED_OLD_NAME:
            WtoSpath(event->FileName, event->FileNameLength, oldpath,listen_path,listen_path_lens);
            if(!Store_listen(oldpath,strlen(path),1))
                break;
            fout<<"event rename from:"<<oldpath<<endl;
            wprintf(L"Renamed from: %.*ls\n", name_len, event->FileName);
            break;
        case FILE_ACTION_RENAMED_NEW_NAME:
            WtoSpath(event->FileName, event->FileNameLength, path,listen_path,listen_path_lens);
            if(!Store_listen(path,strlen(path),1))
                break;
            real_modifyname(oldpath, path, rio);
            fout<<"event rename to:"<<path<<endl;
            wprintf(L"          to: %.*ls\n", name_len, event->FileName);
            break;
        default:
            printf("Unknown action!\n");
            break;
        }
        fout.close();
        this->record_Log();
        //未完成的事件做保存。已经完成的事件应该更新前面遗留的事件
        if(EventStatus<0)
            UnFinshEvent[path] = event->Action;
        else
        {
            map<string, DWORD>::iterator DeleteEvent=UnFinshEvent.find(path);
            if(DeleteEvent!=UnFinshEvent.end())
                UnFinshEvent.erase(DeleteEvent);
        }
        if (event->NextEntryOffset)
            event = nextevent;
        else
            break;
    }
}

void Communicator::Login(QString _username,QString _passwd)
{
    ofstream fout("log",ios::binary | ios::out | ios::app);
    if(!this->ifconnect)
    {
        fout<<"login connect delay!"<<endl;
        fout.close();
        record_Log();
        emit Signal_Reply_Login(Connect_dely);
        return ;
    }
    string username=_username.toStdString();
    string passwd=_passwd.toStdString();
    if(real_login(username.c_str(),
                  passwd.c_str(),rio)<0)
    {
        fout<<"login passwd wrong!"<<endl;
        fout.close();
        record_Log();
        emit Signal_Reply_Login(Login_PasswdFail);
        return ;
    }

    for(int i=0;i<5;)
    {
        int prew=rio->rlen();
        int length;
        DEBUG(prew);
        int ret=rio->peek(&length);
        DEBUG(length);
        if(ret&&length<=prew)
        {
            rio->read(&response,length);
            DEBUG(response.base.type);
            if(response.base.type!=responseLogin)
                continue;
            else
            {
                if(response.response_login.status==YES)
                {
                    DEBUG("YES");
                    if(response.response_login.shareAddrLen==0||response.response_login.shareAddrLen_server==0)
                    {
                        this->SyncStatus=0;
                        this->ifbindaddr=false;
                    }
                    else
                    {
                        this->SyncStatus=1;
                        this->ifbindaddr=true;
                        response.response_login.shareAddr_server[response.response_login.shareAddrLen_server]='\0';
                        this->RemotePath=QString(response.response_login.shareAddr_server);
                        response.response_login.shareAddr[response.response_login.shareAddrLen]='\0';
                        this->SourcePath=QString(response.response_login.shareAddr);
                        strcpy(this->listen_path,this->SourcePath.toStdString().c_str());
                        this->listen_path_lens=strlen(this->listen_path);
                        if(this->listen_path[listen_path_lens-1]!='/')
                        {
                            listen_path[listen_path_lens]='/';
                            listen_path_lens+=1;
                            listen_path[listen_path_lens]='\0';
                            cout<<"this->listen"<<this->listen_path<<endl;
                        }


                    }

                    fout<<"login sucess!"<<endl;


                    emit Signal_Reply_Login(Login_sucess);
                    Share_ifBind=ifbindaddr;
                    if(ifbindaddr)
                    {
                        fout<<"bind "<<SourcePath.toStdString()<<" to "<<RemotePath.toStdString()<<endl;

                    }
                    emit Signal_setBind(SourcePath,RemotePath,ifbindaddr);
                    fout.close();
                    record_Log();
                    return;
                }
                else if(response.response_login.status==countWrong)
                {
                    DEBUG("CountWrong");
                    fout<<"login user wrong"<<endl;
                    fout.close();
                    record_Log();
                    emit Signal_Reply_Login(Login_UserFail);
                    return ;
                }
                else
                {
                    DEBUG("PassWordWrong");
                    fout<<"login user wrong"<<endl;
                    fout.close();
                    record_Log();
                    emit Signal_Reply_Login(Login_PasswdFail);
                    return ;
                }
            }
        }
        Sleep(1000);
        i++;
    }
    fout<<"login connect delay!"<<endl;
    fout.close();
    record_Log();
    emit Signal_Reply_Login(Connect_dely);
}
void Communicator::SendSetBind(QString _Source, QString _Remote,int Status)
{
    ofstream fout("log",ios::binary | ios::out | ios::app);
    string Source=_Source.toStdString(),Remote=_Remote.toStdString();
    cout<<"Sucess SendSetBind"<<endl;
    if(Status){

        HANDLE file = CreateFileA(Source.c_str(),
                             FILE_LIST_DIRECTORY,
                             FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                             NULL);
        if (file == INVALID_HANDLE_VALUE)
        {
            fout<<"bind source not exist!"<<endl;
            fout.close();
            record_Log();
            emit Signal_Reply_SendSetBind("", "",Bind_Sourcenoexist);
            return ;
        }
        else
        {
            CloseHandle(file);
        }
    }
    if(!this->ifconnect)
    {
        fout<<"bind connect delay!"<<endl;
        fout.close();
        record_Log();
        emit Signal_Reply_SendSetBind("","",Connect_dely);
        return ;
    }
    const char * c_Source=Source.c_str();
    const char * c_Remote=Remote.c_str();
    if(Status)
        real_bundling(c_Source,c_Remote,rio);
    else
        real_unbundling(c_Source,rio);


    for(int i=0;i<5;)
    {
        int prew=rio->rlen();
        int length;
        DEBUG(prew);
        int ret=rio->peek(&length);
        DEBUG(length);
        if(ret&&length<=prew)
        {
            rio->read(&response,length);
            DEBUG(response.base.type);
            if(response.base.type!=responseBase)
                continue;
            else
            {
                if(response.response_base.status==YES)
                {
                    DEBUG("bindYES");
                    if(Status==0)
                        this->SyncStatus=0;
                    Share_ifBind=this->ifbindaddr=Status;
                    this->RemotePath=_Remote;
                    this->SourcePath=_Source;
                    strcpy(listen_path,SourcePath.toStdString().c_str());
                    this->listen_path_lens=strlen(this->listen_path);
                    if(this->listen_path[listen_path_lens-1]!='/')
                    {
                        listen_path[listen_path_lens]='/';
                        listen_path_lens+=1;
                        listen_path[listen_path_lens]='\0';
                        cout<<"listen_path"<<listen_path<<endl;
                    }
                    if(Status)
                        fout<<"bind sucess!"<<endl;
                    else
                        fout<<"unbind sucess!"<<endl;
                    fout.close();
                    record_Log();
                    emit Signal_Reply_SendSetBind(_Source,_Remote,Status);
                    return ;
                }
                else
                {
                    fout<<"bind fail!"<<endl;

                    fout.close();
                    record_Log();
                    emit Signal_Reply_SendSetBind("","",Bind_fail);
                    return ;
                }
            }
        }
        Sleep(1000);
        i++;
    }

    fout<<"bind delay!"<<endl;

    fout.close();
    record_Log();
    emit Signal_Reply_SendSetBind("","",Connect_dely);
}
void Communicator::record_Log()
{
    static char log[256];
    ifstream fout("Log",ios::out | ios::binary);
    fout.seekg(LogfileLens,ios::beg);

    if(fout.is_open())
    {
        while(1)
        {
            fout.read(log, 255);
            int readcout=fout.gcount();
            if(readcout==0)
                break;
            else
            {
                log[readcout]='\0';
                LogfileLens+=readcout;
                QString tem(log);
                emit Signal_Log(tem);
            }
        }
    }
    fout.close();
}

void Communicator::Register(QString _username,QString _passwd)
{
    if(!this->ifconnect)
    {
        emit Signal_Reply_Register(Connect_dely);
        return ;
    }
    string username=_username.toStdString(),passwd=_passwd.toStdString();
    if(real_register(username.c_str(),
                  passwd.c_str(),rio)<0)
    {
        emit Signal_Reply_Register(Register_Unstrong);
        return ;
    }


    for(int i=0;i<5;)
    {
        int prew=rio->rlen();
        int length;
        DEBUG("peek");
        if(rio->peek(&length)&&length<=prew)
        {
            rio->read(&response,length);
            DEBUG(response.base.type);
            if(response.base.type!=responseBase)
                continue;
            else
            {
                if(response.response_base.status==YES)
                {
                    DEBUG("sucess");
                    emit Signal_Reply_Register(Register_sucess);
                    return;
                }
                else
                {
                    emit Signal_Reply_Register(Register_fail);
                    return ;
                }
            }
        }
        Sleep(1000);
        i++;
    }
    emit Signal_Reply_Register(Connect_dely);
}

int Communicator::connect_server()
{
    /*if(client_fd>=0)
    {
        close(client_fd);
    }*/
    //Load WinSock
    WORD wVersionRequested;
    WSADATA wsaData;
    int err, iLen;
    wVersionRequested = MAKEWORD(2, 2);            //create 16bit data
    err = WSAStartup(wVersionRequested, &wsaData); //load win socket
    if (err != 0)
    {
        cout << "Load WinSock Failed!";
        return -1;
    }
    struct sockaddr_in mysock;
    memset(&mysock, 0, sizeof(mysock));
    mysock.sin_family = AF_INET;
    cout<<server_Port<<' '<<server_IP<<endl;
    mysock.sin_port = htons(server_Port);
    mysock.sin_addr.s_addr = inet_addr(server_IP);
    if(my_connect(client_fd, mysock)<0)
        return -1;

    if(rio!=NULL)
    {
        delete rio;
    }
    rio = new Socket_RIO(client_fd);
    return 0;
}

void Communicator::Connect()
{
    if(connect_server()>=0)
        this->ifconnect=true;
    else
        this->ifconnect=false;
    emit Signal_Connect_Status(this->ifconnect);
}

void Communicator::Init_Sync()
{
    cout<<"INIT_SYNC"<<endl;
    ofstream fout("log",ios::binary | ios::out | ios::app);

    fout<<"sync start,from client to server"<<endl;
    fout.close();
    record_Log();
    cout<<listen_path<<endl;
    init_synchronous(listen_path,rio);

    fout.open("log",ios::binary | ios::out | ios::app);
    fout<<"sync from client to server over,sync from server to client"<<endl;
    fout.close();
    record_Log();

    real_need_init(rio);
    while(1){

        int lnow=rio->rlen();

        if(lnow==0) continue;
        DEBUG(lnow);
        int length;
        rio->peek(&length);
        DEBUG(length);
        if(lnow<length) continue;
        rio->read(&response, length);
        if (response.base.type == delFile)
        {
            DEBUG(delFile);
            delfile(response, client_fd, rio);
        }
        else if (response.base.type == addFile)
        {
            DEBUG(addFile);
            addfile(response, client_fd, rio);
        }
        else if (response.base.type == sendFile)
        {
            DEBUG(sendFile);
            //cout <<"******send file:"<< response.sendfile.startPos << " " << response.sendfile.endPos << endl;
            sendfile(response, client_fd, rio);
        }
        else if (response.base.type == modifyFileName)
        {
            DEBUG(modifyFileName);
            modifyfilename(response, client_fd, rio);
        }
        else if (response.base.type == responseSend)
        {
            DEBUG(responseSend);
            responsesend(response, client_fd, rio);
        }
        else if (response.base.type == responseBase)
        {
            DEBUG(response.response_base.status);
            if(response.response_base.status==INIT_END)
                break;
        }
    }
    fout.open("log",ios::binary | ios::out | ios::app);
    fout<<"sync from server to client over"<<endl;
    fout.close();
    record_Log();
}
void Communicator::Listen()
{
    setlocale(LC_CTYPE, "");

    //如果Sync=1，那么就是说上次已经同步过了，只需要以客户端为准改远程，
    //如果是0那么说明还没有初始同步，记得同步结束之后要将Sync转为1
    DEBUG(SyncStatus);
    if(!SyncStatus)
    {
        Init_Sync();
        DEBUG("Init_Sync ac");
        SyncStatus=1;
    }
    else
    {

    }
    ofstream fout("log",ios::binary | ios::out | ios::app);
    fout<<"start listen"<<endl;
    fout.close();
    record_Log();

    Sleep(100);
    HANDLE file = CreateFileA(listen_path,
                             FILE_LIST_DIRECTORY,
                             FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                             NULL);
    if (file == INVALID_HANDLE_VALUE)
    {
        fout.open("log",ios::binary | ios::out | ios::app);
        fout<<"creat listen file fail!"<<endl;
        fout.close();
        record_Log();
        return ;
    }
    OVERLAPPED overlapped;
    overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
    uint8_t change_buf[4096];
    DEBUG("pre raed changes");

    fout.open("log",ios::binary | ios::out | ios::app);
    fout<<"watching "<<listen_path<<"for changes..."<<endl;
    fout.close();
    record_Log();

    printf("watching %5s for changes...\n", listen_path);
    BOOL success = ReadDirectoryChangesW(
        file, change_buf, 4096, TRUE,
        FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_LAST_WRITE,
        NULL, &overlapped, NULL);
    int i=0;
    while (Share_ifBind)
    {
        Sleep(100);
        DWORD result = WaitForSingleObject(overlapped.hEvent, 0);
        if (result == WAIT_OBJECT_0)
        {
            DWORD bytes_transferred;
            GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

            FILE_NOTIFY_INFORMATION *events = (FILE_NOTIFY_INFORMATION *)change_buf;
            deal_events(events, rio);
            // Queue the next event

            BOOL success = ReadDirectoryChangesW(
                file, change_buf, 4096, TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME |
                    FILE_NOTIFY_CHANGE_DIR_NAME |
                    FILE_NOTIFY_CHANGE_LAST_WRITE,
                NULL, &overlapped, NULL);
        }

        DealUnfinishEvents(UnFinshEvent,rio);

        int length;
        /*
        if(!rio->peek(&length))
            DEBUG("peekwrong");
        else
            DEBUG(length);
        DEBUG(rio->rlen());*/

        int rlens=rio->rlen();

        if (!rio->peek(&length) ||  rlens< length)
            continue;
        rio->read(&response, length);
        if (response.base.type == delFile)
        {
            DEBUG(delFile);
            Store_listen(response.delfile.fileAddr,response.delfile.addrLen);
            delfile(response, client_fd, rio);
        }
        else if (response.base.type == addFile)
        {
            DEBUG(addFile);
            Store_listen(response.addfile.fileAddr,response.addfile.addrLen);
            for(auto i=No_listen_file.begin();i!=No_listen_file.end();++i)
                cout<<"map:"<<i->first<<" "<<i->second<<endl;
            addfile(response, client_fd, rio);
        }
        else if (response.base.type == sendFile)
        {
            DEBUG(sendFile);
            //cout <<"******send file:"<< response.sendfile.startPos << " " << response.sendfile.endPos << endl;
            sendfile(response, client_fd, rio);
        }
        else if (response.base.type == modifyFileName)
        {
            DEBUG(modifyFileName);

            Store_listen(response.modifyname.fileAddr,response.modifyname.addrLen);
            Store_listen(response.modifyname.newfileAddr,response.modifyname.newaddrLen);
            modifyfilename(response, client_fd, rio);
        }
        else if (response.base.type == responseSend)
        {
            DEBUG(responseSend);
            for(auto i=No_listen_file.begin();i!=No_listen_file.end();++i)
                cout<<"map:"<<i->first<<" "<<i->second<<endl;


            file_info* info=id_file[response.response_send.base.id];
            Store_listen(info->file_name,strlen(info->file_name));
            responsesend(response, client_fd, rio);
        }
        else if (response.base.type == responseBase)
        {
            DEBUG(responseBase);
            responsebase(response, client_fd);
        }
        
    }
    //如果连接已经断开那么发送一个链接断开的信号（这个信号是连接状态），然后让进程调用死循环connect。
}
