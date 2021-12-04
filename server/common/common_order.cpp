#include "../include/common_order.h"
#include "../include/common_sql.h"
#define md5_base "md5_base/"

map <int, file_info* > id_file;
map <char* ,int > file_id;
map <int,int > id_order;
map <string, string > changed_name;
map <string, int > changed_time;

static int id_cnt=0;

//向另一端发送请求的命令
int real_addfile(const char* filename,const char* md5,int tm, Socket_RIO* rio){
    fstream fout("log.log",ofstream::app);
    fout<<"send addFile to "<<rio->user_id<<"-"<<rio->cp_ip<<endl;
    fout<<"filename : "<<filename<<endl;
    fout.close();

    REQUEST request;
    
    int file_size_=0;
    if(md5==NULL){
        id_order[id_cnt]=addFile;
        request.addfile.base={sizeof(ADDFILE),id_cnt++,addFile};
        memcpy(request.addfile.fileMd5,"NULL",4);
        request.addfile.fileTime=tm;
        request.addfile.fileSize=file_size_;
        request.addfile.addrLen=strlen(filename);
        memcpy(request.addfile.fileAddr,filename,strlen(filename));
    }
    else{
        string addr=md5_base;
        addr+=md5;


        file_size_=file_size(addr.c_str());
        if(file_size_==0){
            return 0;
        }
        id_order[id_cnt]=addFile;
        request.addfile.base={sizeof(ADDFILE),id_cnt++,addFile};
        memcpy(request.addfile.fileMd5,md5,32);
        request.addfile.fileTime=tm;
        request.addfile.fileSize=file_size_;
        request.addfile.addrLen=strlen(filename);
        memcpy(request.addfile.fileAddr,filename,strlen(filename));
    }

    while(1){
        usleep(1000);
        if(rio->wlen()<sizeof(ADDFILE))
            continue;
        rio->write(&request,sizeof(ADDFILE));
        break;
    }
    return file_size_;
}
void real_delfile(const char* filename,const char* md5,Socket_RIO* rio){
    fstream fout("log.log",ofstream::app);
    fout<<"send delFile to "<<rio->user_id<<"-"<<rio->cp_ip<<endl;
    fout<<"filename : "<<filename<<endl;
    fout.close();

    REQUEST request;
    id_order[id_cnt]=delFile;
    request.delfile.base={sizeof(DELFILE),id_cnt++,delFile};
    request.delfile.fileTime=gettime(filename);

    memcpy(request.delfile.fileMd5,md5,32);

    request.delfile.addrLen=strlen(filename);
    memcpy(request.delfile.fileAddr,filename,strlen(filename));
    while(1){
        usleep(1000);
        if(rio->wlen()<sizeof(DELFILE))
            continue;
        rio->write(&request,sizeof(DELFILE));
        break;
    }
}
void real_modifyname(const char* filename,const char* newfilename,Socket_RIO* rio){
    fstream fout("log.log",ofstream::app);
    fout<<"send modifyFileName to "<<rio->user_id<<"-"<<rio->cp_ip<<endl;
    fout<<"prefilename : "<<filename<<",newfilename : "<<newfilename<<endl;
    fout.close();

    string pre_filename=filename;
    string new_filename=newfilename;
    changed_name[pre_filename]=new_filename;
    changed_time[pre_filename]=time(0)+5;
    
    REQUEST request;
    id_order[id_cnt]=modifyFileName;
    request.modifyname.base={sizeof(MODIFYNAME),id_cnt++,modifyFileName};
    request.modifyname.fileTime=gettime(filename);
    request.modifyname.addrLen=strlen(filename);
    memcpy(request.modifyname.fileAddr,filename,strlen(filename));
    request.modifyname.newaddrLen=strlen(newfilename);
    memcpy(request.modifyname.newfileAddr,newfilename,strlen(newfilename));
    while(1){
        usleep(1000);
        if(rio->wlen()<sizeof(MODIFYNAME))
            continue;
        rio->write(&request,sizeof(MODIFYNAME));
        break;
    }
}

void real_sendfile(int id,Socket_RIO* rio,map<string, set<Socket_RIO*> >& all_rio){
    cout<<"send requireFile to "<<rio->user_id<<"-"<<rio->cp_ip<<endl;
    
    file_info* info=id_file[id];
    
    if(info->recvLen >= info->fileLen){
        fstream fout("log.log",ofstream::app);
        fout<<"file send all success"<<endl;
        fout<<info->file_name<<" "<<rio->user_id<<" "<<rio->cp_ip<<endl;
        fout.close();

        string now=rio->user_id;
        char use[256]={0},md5_[40];
        strcpy(md5_,info->file_md5);
        for(Socket_RIO* rio_now : all_rio[now]){
            if(rio_now==rio) continue;
            strcpy(use,info->file_name);
            cout<<rio->cp_ip<<"-"<<rio_now->cp_ip<<endl;
            addr_change(rio->user_id,use,rio->cp_ip);
            
            if(addr_change_back(rio->user_id,use,rio_now->cp_ip)==1){
                string use__=use;
                if(changed_name.count(use__) && changed_time[use__]>time(0)) use__=changed_name[use__];
                real_addfile(use__.c_str(),md5_,time(0),rio_now);
            }
        }

        delete[] info->file_name;
        delete[] info->file_md5_name;
        delete[] info->file_md5;
        delete[] info;
        id_file.erase(id);
        return;
    }
        
    int endPos=min(info->recvLen+SEND_FILE_SIZE,info->fileLen);
    cout<<"require "<<info->file_name<<",interval:"<<info->recvLen<<"-"<<info->fileLen<<endl;

    REQUEST request;
    request.sendfile.base={sizeof(SENDFILE),id,sendFile};
    memcpy(request.sendfile.fileMd5,info->file_md5,strlen(info->file_md5));
    request.sendfile.startPos=info->recvLen;
    request.sendfile.endPos=endPos;
    request.sendfile.addrLen=strlen(info->file_name);
    memcpy(request.sendfile.fileAddr,info->file_name,strlen(info->file_name));  
    // cout<<request.sendfile.addrLen<<endl;  
    while(1){
        usleep(1000);
        if(rio->wlen()<sizeof(SENDFILE))
            continue;
        rio->write(&request,sizeof(SENDFILE));
        break;
    }  
}
void real_responsebase(int id,int status,Socket_RIO* rio){
    cout<<"send responseBase"<<endl;

    REQUEST request;
    request.response_base={sizeof(RESPONSE_BASE),id,
        responseBase,status};
    while(1){
        usleep(1000);
        if(rio->wlen()<sizeof(RESPONSE_BASE))
            continue;
        rio->write(&request,sizeof(RESPONSE_BASE));
        break;
    }
}



//从另一端接收请求的处理函数
void delfile(REQUEST& response,int fd_now,Socket_RIO* rio,map<string, set<Socket_RIO*> >& all_rio){
    char* filename=new char[256];
    memcpy(filename,response.delfile.fileAddr,response.delfile.addrLen);
    filename[response.delfile.addrLen]=0;
    addr_change_normal(filename);

    addr_change(rio->user_id,filename,rio->cp_ip);

    char md5[40];
    strncpy(md5,response.delfile.fileMd5,32);
    md5[32]=0;
    int flag=file_solve_addr(all_rio, rio ,rio->user_id,filename,response.delfile.fileTime, DELFILE_ADDR, md5, NULL);

    cout<<"delete "<<filename<<endl;

    string now=rio->user_id;
    char use[256]={0};
    for(Socket_RIO* rio_now : all_rio[now]){
        if(rio_now==rio) continue;
        strcpy(use,filename);
        addr_change(rio->user_id,use,rio->cp_ip);
        if(addr_change_back(rio->user_id,use,rio_now->cp_ip)==1){
            string use__=use;
            if(changed_name.count(use__) && changed_time[use__]>time(0)) use__=changed_name[use__];
            real_delfile(use__.c_str(),md5,rio_now);
        }
    }

    REQUEST request;
    request.response_base={sizeof(RESPONSE_BASE),response.base.id,
        responseBase,(flag==1)?YES:NO};
    while(1){
        if(rio->wlen()<sizeof(RESPONSE_BASE))
            continue;
        rio->write(&request,sizeof(RESPONSE_BASE));
        break;
    }
    delete[] filename;
}
void addfile(REQUEST& response,int fd_now,Socket_RIO* rio,map<string, set<Socket_RIO*> >& all_rio){

    char* filename=new char[256];
    char* inner_filename=new char[256];
    memcpy(filename,response.addfile.fileAddr,response.addfile.addrLen);
    filename[response.addfile.addrLen]=0;
    memcpy(inner_filename,response.addfile.fileAddr,response.addfile.addrLen);
    inner_filename[response.addfile.addrLen]=0;
    addr_change_normal(filename);
    addr_change_normal(inner_filename);

    addr_change(rio->user_id,inner_filename,rio->cp_ip);

    char* md5=new char[40];
    memcpy(md5,response.addfile.fileMd5,32);
    md5[32]=0;
    char md5_[40];
    strcpy(md5_,md5);

    int flag__;

    if(strncmp(md5,"NULL",4)==0){//文件夹
        file_solve_addr(all_rio, rio, rio->user_id,inner_filename,response.addfile.fileTime,ADDFILE_ADDR, NULL, NULL);
        string now=rio->user_id;
        char use[256]={0};
        for(Socket_RIO* rio_now : all_rio[now]){
            if(rio_now==rio) continue;
            strcpy(use,filename);
            addr_change(rio->user_id,use,rio->cp_ip);
            if(addr_change_back(rio->user_id,use,rio_now->cp_ip)==1){
                string use__=use;
                if(changed_name.count(use__) && changed_time[use__]>time(0)) use__=changed_name[use__];
                real_addfile(use__.c_str(),NULL,response.addfile.fileTime,rio_now);
            }
                
        }
        flag__=FILE_EXIST;
    }
    else{//文件
        int flag=file_md5_exist(md5);
        if(flag==1){  //文件存在
            cout<<"file exist"<<endl;
            flag__=FILE_EXIST;
        }
        else{   //文件不存在
            flag__=SENDING;
            file_md5_add(md5);

            char* md5filename=new char[260];
            strcpy(md5filename,md5_base);
            strcat(md5filename,md5); 
            
            FILE *fp = fopen(md5filename, "wb+");
            fclose(fp);

            id_file[id_cnt]=new file_info;
            id_file[id_cnt]->file_name=filename;
            id_file[id_cnt]->file_md5_name=md5filename;
            id_file[id_cnt]->file_md5=md5;
            id_file[id_cnt]->fileLen=response.addfile.fileSize;
            id_file[id_cnt]->recvLen=0;

            file_id[filename]=id_cnt;
            
            real_sendfile(id_cnt,rio,all_rio);
            id_cnt++;
        }

        file_solve_addr(all_rio, rio ,rio->user_id,inner_filename,response.addfile.fileTime,ADDFILE_ADDR, md5, NULL);
        if(flag==1){
            string now=rio->user_id;
            char use[256]={0};
            for(Socket_RIO* rio_now : all_rio[now]){
                if(rio_now==rio) continue;
                strcpy(use,filename);
                addr_change(rio->user_id,use,rio->cp_ip);
                if(addr_change_back(rio->user_id,use,rio_now->cp_ip)==1){
                    string use__=use;
                    if(changed_name.count(use__) && changed_time[use__]>time(0)) use__=changed_name[use__];
                    real_addfile(use__.c_str(),md5_,time(0),rio_now);
                }
            }
        }
    }
    delete[] inner_filename;

    REQUEST request;
    request.response_base={sizeof(RESPONSE_BASE),0,
        responseBase,flag__};
    while(1){
        usleep(1000);
        if(rio->wlen()<sizeof(RESPONSE_BASE))
            continue;
        rio->write(&request,sizeof(RESPONSE_BASE));
        break;
    }
}
int sendfile(REQUEST& response,int client_fd,Socket_RIO* rio){
    char* md5=new char[33];
    memcpy(md5,response.sendfile.fileMd5,32);
    md5[32]=0;
    string realfile=md5_base;
    realfile+=md5;

    cout<<"send interval:"<<realfile<<endl;
    cout<<response.sendfile.startPos<<"-"<<response.sendfile.endPos<<endl;
    
    FILE *fp=fopen(realfile.c_str(),"rb");
    fseek(fp,response.sendfile.startPos,SEEK_SET); 

    REQUEST request;
    request.response_send.base={sizeof(RESPONSE_SEND),response.sendfile.base.id,responseSend};
    request.response_send.status=YES;
    request.response_send.startPos=response.sendfile.startPos;
    request.response_send.endPos=response.sendfile.endPos;
    int llen=response.sendfile.endPos;
    fread(request.response_send.content,sizeof(char),
        response.sendfile.endPos-response.sendfile.startPos,fp);
    
    while(1){
        usleep(1000);
        if(rio->wlen()<sizeof(RESPONSE_SEND))
            continue;
        rio->write(&request,sizeof(RESPONSE_SEND));
        break;
    }
    fclose(fp);
    delete[] md5;
    return llen>=file_size(realfile.c_str());
}
void modifyfilename(REQUEST& response,int fd_now,Socket_RIO* rio,map<string, set<Socket_RIO*> >& all_rio){
    char* filename=new char[256];
    memcpy(filename,response.modifyname.fileAddr,response.modifyname.addrLen);
    filename[response.modifyname.addrLen]=0;
    char* newfilename=new char[256];
    memcpy(newfilename,response.modifyname.newfileAddr,response.modifyname.newaddrLen);
    newfilename[response.modifyname.newaddrLen]=0;

    addr_change_normal(filename);
    addr_change_normal(newfilename);

    string pre_filename=filename;
    string new_filename=newfilename;
    changed_name[pre_filename]=new_filename;
    changed_time[pre_filename]=time(0)+5;

    // int flag=rename(filename,newfilename);
    addr_change(rio->user_id,filename,rio->cp_ip);
    int flag=file_solve_addr(all_rio, rio ,rio->user_id,filename,response.modifyname.fileTime, MODIFYFILE_ADDR, NULL, newfilename);

    cout<<"modify "<<filename<<" "<<newfilename<<endl;
    string now=rio->user_id;
    char use[256]={0},newuse[256]={0};
    for(Socket_RIO* rio_now : all_rio[now]){
        if(rio_now==rio) continue;
        strcpy(use,filename);
        addr_change(rio->user_id,use,rio->cp_ip);
        strcpy(newuse,newfilename);
        addr_change(rio->user_id,newuse,rio->cp_ip);
        if(addr_change_back(rio->user_id,use,rio_now->cp_ip)==1 && 
            addr_change_back(rio->user_id,newuse,rio_now->cp_ip)==1){
            real_modifyname(use,newuse,rio_now); 
        }    
    }

    REQUEST request;
    request.response_base={sizeof(RESPONSE_BASE),response.base.id,
        responseBase,(flag==1)?YES:NO};
    while(1){
        usleep(1000);
        if(rio->wlen()<sizeof(RESPONSE_BASE))
            continue;
        rio->write(&request,sizeof(RESPONSE_BASE));
        break;
    }
    delete[] filename;
    delete[] newfilename;
}
void register_in(REQUEST& response,Socket_RIO* rio){
    char passwd[25],username[25];
    memcpy(passwd,response.register_.passwd,response.register_.passwdLen);
    passwd[response.register_.passwdLen]=0;
    memcpy(username,response.register_.username,response.register_.usenameLen);
    username[response.register_.usenameLen]=0;
    int flag=try_register(username,passwd);
    if(flag==1){
        cout<<username<<" register success"<<endl;
    }
    else{
        cout<<username<<" register failed"<<endl;
    }

    REQUEST request;
    request.response_base={sizeof(RESPONSE_BASE),response.base.id,
        responseBase,(flag==1)?YES:NO};
    while(1){
        usleep(1000);
        if(rio->wlen()<sizeof(RESPONSE_BASE))
            continue;
        rio->write(&request,sizeof(RESPONSE_BASE));
        break;
    }
}
void login_in(REQUEST& response,Socket_RIO* rio,map<string, set<Socket_RIO*> >& all_rio){
    char passwd[25],username[25],cp_ip[50];
    memcpy(passwd,response.login.passwd,response.login.passwdLen);
    passwd[response.login.passwdLen]=0;
    memcpy(username,response.login.username,response.login.usenameLen);
    username[response.login.usenameLen]=0;
    memcpy(cp_ip,response.login.cp_ip,response.login.cp_ipLen);
    cp_ip[response.login.cp_ipLen]=0;

    char bindAddr[256]={0};
    char bindAddr_server[256]={0};
    int flag=try_login(username,passwd,cp_ip,bindAddr,bindAddr_server);
    if(flag==YES){
        cout<<username<<" login success"<<endl;
        string now=username;
        all_rio[now].insert(rio);
        strcpy(rio->user_id,username);
        strcpy(rio->cp_ip,cp_ip);
    }
    else if(flag==passwdWrong){
        cout<<username<<" password wrong"<<endl;
    }
    else if(flag==countWrong){
        cout<<username<<" no such account"<<endl;
    }

    REQUEST request;
    request.response_login.base={sizeof(RESPONSE_LOGIN),response.base.id,
        responseLogin};
    request.response_login.status=flag;
    memcpy(request.response_login.shareAddr,bindAddr,strlen(bindAddr));
    request.response_login.shareAddrLen=strlen(bindAddr);
    memcpy(request.response_login.shareAddr_server,bindAddr_server,strlen(bindAddr_server));
    request.response_login.shareAddrLen_server=strlen(bindAddr_server);
    while(1){
        usleep(1000);
        if(rio->wlen()<sizeof(RESPONSE_LOGIN))
            continue;
        rio->write(&request,sizeof(RESPONSE_LOGIN));
        break;
    }
}
void unbundling(REQUEST& response,Socket_RIO* rio){
    char addr[256];
    memcpy(addr,response.unbundling.addr,response.unbundling.addrLen);
    addr[response.unbundling.addrLen]=0;
    addr_change_normal(addr);

    int flag=try_unbundling(rio->user_id,addr,rio->cp_ip);

    REQUEST request;
    request.response_base={sizeof(RESPONSE_BASE),response.base.id,
        responseBase,(flag==1)?YES:NO};
    while(1){
        usleep(1000);
        if(rio->wlen()<sizeof(RESPONSE_BASE))
            continue;
        rio->write(&request,sizeof(RESPONSE_BASE));
        break;
    }
}
void bundling(REQUEST& response,Socket_RIO* rio){
    char* addrLocal=new char[256];
    memcpy(addrLocal,response.bundling.addrLocal,response.bundling.addrLocalLen);
    addrLocal[response.bundling.addrLocalLen]=0;
    addr_change_normal(addrLocal);

    char* addrServer=new char[256];
    memcpy(addrServer,response.bundling.addrServer,response.bundling.addrServerLen);
    addrServer[response.bundling.addrServerLen]=0;
    addr_change_normal(addrServer);

    cout<<"local address:"<<addrLocal<<", server address:"<<addrServer<<endl;
    int flag=try_bundling(rio->user_id,addrLocal,addrServer,rio->cp_ip);

    REQUEST request;
    request.response_base={sizeof(RESPONSE_BASE),response.base.id,
        responseBase,(flag==1)?YES:NO};
    while(1){
        usleep(1000);
        if(rio->wlen()<sizeof(RESPONSE_BASE))
            continue;
        rio->write(&request,sizeof(RESPONSE_BASE));
        break;
    }
    delete[] addrLocal;
    delete[] addrServer;
}

void init_synchronous(Socket_RIO* rio){
    sql_init_synchronous(rio);
}


//接收请求回复
void responsesend(REQUEST& response,int fd_now,Socket_RIO* rio,map<string, set<Socket_RIO*> >& all_rio){
    file_info* info=id_file[response.response_send.base.id]; 
    if(info->recvLen>response.response_send.startPos){
        return;
    }
    if(file_id[info->file_name]!=response.response_send.base.id){
        return;
    }
    FILE *fp = fopen(info->file_md5_name, "a");
    fseek(fp,response.response_send.startPos,SEEK_SET);
    fwrite(response.response_send.content,1
        ,response.response_send.endPos-response.response_send.startPos,fp);
    fclose(fp);
    info->recvLen=response.response_send.endPos;

    cout<<info->file_name<<" recv "<<response.response_send.endPos<<" bytes.("<<100.0*response.response_send.endPos/info->fileLen<<")\n";

    real_sendfile(response.response_send.base.id,rio,all_rio);
}
void responsebase(REQUEST& response,int fd_now){
    ;
}

