#include "../include/common_order.h"
#define DEBUG(x) std::cout<<__LINE__<<"\t\t"<<#x<<':'<<x<<endl;

map<int,file_info* > id_file;
map<int,int > id_order;
static int id_cnt=0;

//发送的包
int real_addfile(const char* filename,Socket_RIO* rio){
    cout<<"real addfile"<<endl;
    ofstream fout("log",ios::app|ios::out|ios::binary);

    REQUEST request;
    int file_size_=1;

    if(IsDirectory(filename)){
        cout<<"folder---"<<endl;
        cout<<filename<<endl;
        memcpy(request.addfile.fileMd5,"NULL",4);
        fout<<"add dir to server : "<<filename<<endl;
    }
    else{
        cout<<"file---"<<endl;
        cout<<filename<<endl;
        file_size_=file_size(filename);
        if(file_size_<=0){
            fout<<"no need send : "<<filename<<endl;
            fout.close();
            return file_size_;
        }
        fout<<"add file to server : "<<filename<<endl;
        fout.close();
        memcpy(request.addfile.fileMd5,md5file(filename).c_str(),32);
    }

    id_order[id_cnt]=addFile;
    request.addfile.base={sizeof(ADDFILE),id_cnt++,addFile};

    request.addfile.fileTime=gettime(filename);
    request.addfile.fileSize=file_size_;
    request.addfile.addrLen=strlen(filename);
    memcpy(request.addfile.fileAddr,filename,strlen(filename));
    
    while(1){
        if(rio->wlen()<sizeof(ADDFILE))
        {
            Sleep(100);
            continue;
        }
        rio->write(&request,sizeof(ADDFILE));
        break;
    }
    return file_size_;
}
void real_delfile(const char* filename,Socket_RIO* rio){
    ofstream fout("log",ios::app|ios::out|ios::binary);
    REQUEST request;
    id_order[id_cnt]=delFile;
    request.delfile.base={sizeof(DELFILE),id_cnt++,delFile};
    request.delfile.fileTime=gettime(filename);
    if(IsDirectory(filename)==true){
        memcpy(request.delfile.fileMd5,"NULL",4);
        fout<<"del dir to server : "<<filename<<endl;
    }
    else{
        memcpy(request.delfile.fileMd5,md5file(filename).c_str(),32);
        fout<<"del file to server : "<<filename<<endl;
    }
    fout.close();
    request.delfile.addrLen=strlen(filename);
    memcpy(request.delfile.fileAddr,filename,strlen(filename));
    while(1){
        if(rio->wlen()<sizeof(DELFILE))
        {
            Sleep(100);
            continue;
        }
        rio->write(&request,sizeof(DELFILE));
        break;
    }
}
void real_modifyname(const char* filename,char* newfilename,Socket_RIO* rio){
    REQUEST request;

    id_order[id_cnt]=modifyFileName;
    request.modifyname.base={sizeof(MODIFYNAME),id_cnt++,modifyFileName};
    request.modifyname.fileTime=gettime(filename);
    request.modifyname.addrLen=strlen(filename);
    memcpy(request.modifyname.fileAddr,filename,strlen(filename));
    request.modifyname.newaddrLen=strlen(newfilename);
    memcpy(request.modifyname.newfileAddr,newfilename,strlen(newfilename));

    ofstream fout("log",ios::app|ios::out|ios::binary);
    fout<<"modify name from "<<filename<<" to "<<newfilename<<"to server"<<endl;
    fout.close();
    while(1){
        if(rio->wlen()<sizeof(MODIFYNAME))
        {
            Sleep(100);
            continue;
        }
        rio->write(&request,sizeof(MODIFYNAME));
        break;
    }
}
void real_sendfile(int id,Socket_RIO* rio){
    file_info* info=id_file[id];
    ofstream fout("log",ios::app|ios::out|ios::binary);

    if(info->recvLen >= info->fileLen){
        fout<<info->file_name<<" require finished"<<endl;
        fout.close();
        delete[] info->file_name;
        delete[] info->file_md5;
        delete[] info;
        id_file.erase(id);
        id_order.erase(id);
        return;
    }
        
    int endPos=min(info->recvLen+SEND_FILE_SIZE,info->fileLen);
    cout<<"require "<<info->file_name<<endl;

    REQUEST request;
    request.sendfile.base={sizeof(SENDFILE),id,sendFile};
    memcpy(request.sendfile.fileMd5,info->file_md5,strlen(info->file_md5));
    request.sendfile.startPos=info->recvLen;
    request.sendfile.endPos=endPos;
    request.sendfile.addrLen=strlen(info->file_name);
    memcpy(request.sendfile.fileAddr,info->file_name,strlen(info->file_name));  
     cout<<request.sendfile.addrLen<<endl;


    fout<<"require "<<info->file_name<<"("<<request.sendfile.startPos<<","
       <<request.sendfile.endPos<<")"<<endl;

    while(1){
        DEBUG(rio->wlen());
        if(rio->wlen()<sizeof(SENDFILE))
        {
            Sleep(100);
            continue;
        }
        rio->write(&request,sizeof(SENDFILE));
        break;
    }  
}

int real_register(const char* username, const char* passwd,Socket_RIO* rio){
    REQUEST request;
    id_order[id_cnt]=sendRegister;
    request.register_.base={sizeof(REGISTER),id_cnt++,sendRegister};
    request.register_.passwdLen=strlen(passwd);
    request.register_.usenameLen=strlen(username);
    if(request.register_.passwdLen>=20 || request.register_.usenameLen>=20 || request.register_.passwdLen<=6){
        return -1;
    }
    memcpy(request.register_.passwd,passwd,strlen(passwd));
    memcpy(request.register_.username,username,strlen(username));
    while(1){
        if(rio->wlen()<sizeof(REGISTER))
        {
            Sleep(100);
            continue;
        }
        rio->write(&request,sizeof(REGISTER));
        break;
    }
    return 0;
}
int real_login(const char* username, const char* passwd,Socket_RIO* rio){
    string now=getDeviceFingerPrint();
    REQUEST request;
    id_order[id_cnt]=sendLogin;
    request.login.base={sizeof(LOGIN),id_cnt++,sendLogin};
    request.login.passwdLen=strlen(passwd);
    request.login.usenameLen=strlen(username);
    request.login.cp_ipLen=now.size();
    if(request.login.passwdLen>=20 || request.login.usenameLen>=20){
        return -1;
    }
    memcpy(request.login.passwd,passwd,strlen(passwd));
    memcpy(request.login.username,username,strlen(username));
    memcpy(request.login.cp_ip,now.c_str(),now.size());
    while(1){
        if(rio->wlen()<sizeof(LOGIN))
        {
            Sleep(100);
            continue;
        }
        rio->write(&request,sizeof(LOGIN));
        break;
    }
    return 0;
}

void real_bundling(const char* localAddr,const char* serverAddr,Socket_RIO* rio){
    REQUEST request;
    cout<<"budling localAddr:"<<localAddr<<' '<<serverAddr<<endl;
    id_order[id_cnt]=sendBundling;
    request.bundling.base={sizeof(BUNDLING),id_cnt++,sendBundling};
    request.bundling.addrLocalLen=strlen(localAddr);
    request.bundling.addrServerLen=strlen(serverAddr);
    memcpy(request.bundling.addrLocal,localAddr,strlen(localAddr));
    memcpy(request.bundling.addrServer,serverAddr,strlen(serverAddr));
    cout<<"budling localAddr:"<<strlen(localAddr)<<' '<<strlen(serverAddr)<<endl;
    while(1){
        if(rio->wlen()<sizeof(BUNDLING))
        {
            Sleep(100);
            continue;
        }
        rio->write(&request,sizeof(BUNDLING));
        break;
    }
}
void real_unbundling(const char* addr,Socket_RIO* rio){
    REQUEST request;
    id_order[id_cnt]=sendUnbundling;
    request.unbundling.base={sizeof(UNBUNDLING),id_cnt++,sendUnbundling};
    request.unbundling.addrLen=strlen(addr);
    memcpy(request.unbundling.addr,addr,strlen(addr));
    while(1){
        if(rio->wlen()<sizeof(UNBUNDLING))
        {
            Sleep(100);
            continue;
        }
        rio->write(&request,sizeof(UNBUNDLING));
        break;
    }
}
void real_need_init(Socket_RIO* rio){
    REQUEST request;
    request.base={sizeof(REQUEST_BASE),0,need_init_synchronous};
    while(1){
        if(rio->wlen()<sizeof(REQUEST_BASE))
        {
            Sleep(100);
            continue;
        }
        rio->write(&request,sizeof(REQUEST_BASE));
        break;
    }
}

//接受的包
void delfile(REQUEST& response,int fd_now,Socket_RIO* rio){

    ofstream fout("log",ios::app|ios::out|ios::binary);


    char* filename=new char[256];
    memcpy(filename,response.delfile.fileAddr,response.delfile.addrLen);
    filename[response.delfile.addrLen]=0;
    bool flag=DeleteFile_(filename);


    fout<<"delete "<<filename<<endl;
    fout.close();

    REQUEST request;
    request.response_base={sizeof(RESPONSE_BASE),response.base.id,
        responseBase,flag?YES:NO};
    rio->write(&request,sizeof(RESPONSE_BASE));
    delete[] filename;
}
void addfile(REQUEST& response,int fd_now,Socket_RIO* rio){
    char* filename=new char[256];
    memcpy(filename,response.addfile.fileAddr,response.addfile.addrLen);
    filename[response.addfile.addrLen]=0;
    char* md5=new char[33];
    memcpy(md5,response.addfile.fileMd5,32);
    md5[32]=0;

    ofstream fout("log",ios::app|ios::out|ios::binary);

    cout<<filename<<endl;
    
    int flag;
    
    if(strncmp("NULL",md5,4)==0){
        cout<<"folder"<<endl;
        if (0 != access(filename, 0))
        {
            fout<<"add dir "<<filename<<endl;
            cout<<"no such folder"<<endl;
            // if this folder not exist, create a new one.
            mkdir(filename);   // 返回 0 表示创建成功，-1 表示失败
        }
        flag=FILE_EXIST;
        delete[] filename;
        delete[] md5;
    }
    else {
        if (0 != access(filename, 0)){
            cout<<"no such file"<<endl;
            flag=SENDING;
            cout<<response.addfile.fileSize<<endl;
            FILE *fp = fopen(filename, "wb+");
            fclose(fp);
            id_file[id_cnt]=new file_info;
            id_file[id_cnt]->file_name=filename;
            id_file[id_cnt]->file_md5=md5;
            id_file[id_cnt]->fileLen=response.addfile.fileSize;
            id_file[id_cnt]->recvLen=0;
            id_order[id_cnt]=sendFile;
            real_sendfile(id_cnt,rio);
            id_cnt++;
            fout<<"add file "<<filename<<endl;
        }
        else{
            cout<<"文件已存在"<<endl;
            flag=FILE_EXIST;
            delete[] filename;
            delete[] md5;
            fout<<"add file exist"<<endl;
        }
    }
    fout.close();
    REQUEST request;
    request.response_base={sizeof(RESPONSE_BASE),response.base.id,
        responseBase,flag};
    rio->write(&request,sizeof(RESPONSE_BASE));
    DEBUG("respose wirte");

}
int sendfile(REQUEST& response,int client_fd,Socket_RIO* rio){
    char* filename=new char[256];
    memcpy(filename,response.sendfile.fileAddr,response.sendfile.addrLen);
    filename[response.sendfile.addrLen]=0;
    int file_size_=file_size(filename);
    FILE *fp=fopen(filename,"rb");
    DEBUG((int)fp);
    fseek(fp,response.sendfile.startPos,SEEK_SET); 

    REQUEST request;
    request.response_send.base={sizeof(RESPONSE_SEND),response.sendfile.base.id,responseSend};
    request.response_send.status=YES;
    request.response_send.startPos=response.sendfile.startPos;
    request.response_send.endPos=response.sendfile.endPos;
    fread(request.response_send.content,sizeof(char),
        response.sendfile.endPos-response.sendfile.startPos,fp);
    ofstream fout("log",ios::app|ios::out|ios::binary);
    fout<<"send file:"<<filename<<"("<<request.response_send.startPos<<","
       <<request.response_send.endPos<<")"<<endl;
    fout.close();
    while(1){

        DEBUG(rio->wlen());
        if(rio->wlen()<sizeof(RESPONSE_SEND))
        {
            Sleep(100);
            continue;
        }
        rio->write(&request,sizeof(RESPONSE_SEND));
        break;
    }  
    fclose(fp);
    delete[] filename;
    return response.sendfile.endPos>=file_size(filename);
}
void modifyfilename(REQUEST& response,int fd_now,Socket_RIO* rio){
    char* filename=new char[256];
    memcpy(filename,response.modifyname.fileAddr,response.modifyname.addrLen);
    filename[response.modifyname.addrLen]=0;
    char* newfilename=new char[256];
    memcpy(newfilename,response.modifyname.newfileAddr,response.modifyname.newaddrLen);
    newfilename[response.modifyname.newaddrLen]=0;
    int flag=rename(filename,newfilename);


    cout<<"modify "<<filename<<" "<<newfilename<<endl;
    ofstream fout("log",ios::app|ios::out|ios::binary);
    fout<<"modify name from "<<filename<<" to "<<newfilename<<endl;
    fout.close();

    REQUEST request;
    request.response_base={sizeof(RESPONSE_BASE),response.base.id,
        responseBase,(!flag)?YES:NO};
    while(1){

        DEBUG(rio->wlen());
        if(rio->wlen()<sizeof(RESPONSE_BASE))
        {
            Sleep(100);
            continue;
        }
        rio->write(&request,sizeof(RESPONSE_BASE));
        break;
    }
    delete[] filename;
    delete[] newfilename;
}

//接受的回复
void responsesend(REQUEST& response,int fd_now,Socket_RIO* rio){
    file_info* info=id_file[response.response_send.base.id];
    DEBUG(file_size(info->file_name));
    int ret=truncate(info->file_name, response.response_send.startPos);
    if(ret!=response.response_send.startPos)
        cout<<__LINE__<<"ret error"<<response.response_send.startPos<<endl;
    FILE *fp = fopen(info->file_name, "ab");

    DEBUG(fp);

    fseek(fp,response.response_send.startPos,SEEK_SET);
    int flag= fwrite(response.response_send.content,1
        ,response.response_send.endPos-response.response_send.startPos,fp);
    DEBUG(response.response_send.endPos);
    DEBUG(response.response_send.startPos);

    fclose(fp);

    DEBUG(flag);
    DEBUG(file_size(info->file_name));
    info->recvLen=response.response_send.endPos;
    ofstream fout("log",ios::app|ios::out|ios::binary);
    fout<<"save file:"<<info->file_name<<"("<<response.response_send.startPos<<","
       <<response.response_send.endPos<<")"<<endl;
    fout.close();
    cout<<"recv: "<<response.response_send.startPos<<"-"<<response.response_send.endPos<<endl;

    real_sendfile(response.response_send.base.id,rio);
}
void responsebase(REQUEST& response,int fd_now){
    int order=id_order[response.response_base.base.id];
    if(order==sendRegister){
        cout<<"Register "<<response.response_base.status<<endl;
    }
}
void responselogin(REQUEST& response,int fd_now){
    cout<<"Login "<<response.response_login.status<<endl;
    cout<<response.response_login.shareAddrLen<<endl;
    response.response_login.shareAddr[response.response_login.shareAddrLen]=0;
    cout<<response.response_login.shareAddr<<endl;
}


void init_synchronous(const char* foldername,Socket_RIO* rio){
    cout<<"fileFile:"<<foldername<<endl;
    findFile(foldername,rio);
}

