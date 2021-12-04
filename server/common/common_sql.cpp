#include "../include/common_sql.h"


/**************************
路径分解函数
file_name形如 test1/ttest1/tttest1
***********************/
void decompose_addr(vector<string>& address, const char * file_name){
    string addnow="";
    int wz=0;
    while(file_name[wz]){
        if(file_name[wz]=='/' || file_name[wz]=='\\'){
            address.push_back(addnow);
            addnow="";
        }
        else{
            addnow+=file_name[wz];
        }
        wz++;
    }
    address.push_back(addnow);
}

/***********************
该md5表示的文件是否存在
    0表示不存在
    1表示存在
***********************/
bool file_md5_exist(const char* file_md5){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    /* 连接数据库，失败返回NULL
            1、mysqld没运行
            2、没有指定名称的数据库存在 */
    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* 设置字符集，否则读出的字符乱码，即使/etc/my.cnf中设置也不行 */
    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"select * from file_num WHERE file_md5='%s'", file_md5);
    /* 进行查询，成功返回0，不成功非0
            1、查询字符串存在语法错误
            2、查询不存在的数据表 */
    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    MYSQL_RES *result;
    /* 将查询结果存储起来，出现错误则返回NULL
        注意：查询结果为NULL，不会返回NULL */
    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        mysql_free_result(result);
        /* 关闭整个连接 */
        mysql_close(mysql);
        return -1;
    }

    bool flag=((int)mysql_num_rows(result)!=0);

    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);

    return flag;
}


/*********************************************
根据文件名和父亲节点编号查找文件
找不到返回0
报错返回-1
*********************************************/
int find_file_id(const char * file_name, int parent_id){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"select file_id from file_md5_info WHERE file_name='%s' AND file_parent=%d", file_name, parent_id);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        mysql_free_result(result);
        /* 关闭整个连接 */
        mysql_close(mysql);
        
        return -1;
    }

    if((int)mysql_num_rows(result)==0){
        cout<<"cant find "<<file_name<<" "<<parent_id<<endl;
        mysql_free_result(result);
        /* 关闭整个连接 */
        mysql_close(mysql);
        return 0;
    }

    row = mysql_fetch_row(result);
    
    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);

    int ans;
    str2int(row[0],ans);
    return ans;
}


/********************************************
直接调用mysql添加文件路径信息
如果是文件（MD5！=NULL），同时更新file_num表信息（+1）
********************************************/
int file_add_addr_real(const char* md5,const char * name,int parent,int tm){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"insert into file_md5_info (file_md5,file_name,file_parent,file_modifytime)values('%s','%s',%d,%d)", md5, name,parent,tm);
    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }
    if(md5!=NULL){
        sprintf(buff,"UPDATE file_num set file_num.file_num=(SELECT file_num.file_num+1 FROM file_num WHERE file_md5='%s') WHERE file_md5='%s'", md5,md5);
        if (mysql_query(mysql, buff))
        {
            cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
            mysql_close(mysql);
            return -1;
        }   
    }

    /* 关闭整个连接 */
    mysql_close(mysql);
    return 1;
}

/********************************************
直接调用mysql删除文件路径信息
如果是文件（MD5！=NULL），同时更新file_num表信息（-1）
********************************************/
int file_del_addr_real(int id,const char* md5){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"delete from file_md5_info where file_id=%d", id);
    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    if(md5!=NULL && strlen(md5)==32){
        cout<<1<<" "<<md5<<endl;
        sprintf(buff,"UPDATE file_num set file_num.file_num=(SELECT file_num.file_num-1 FROM file_num WHERE file_md5='%s') WHERE file_md5='%s'", md5,md5);
        if (mysql_query(mysql, buff))
        {
            cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
            mysql_close(mysql);
            return -1;
        }   
    }
    else{
        cout<<2<<endl;
        MYSQL_RES *result;
        MYSQL_ROW row;

        sprintf(buff,"SELECT file_id,file_md5 FROM file_md5_info WHERE file_parent=%d", id);
        if (mysql_query(mysql, buff))
        {
            cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
            mysql_free_result(result);
            mysql_close(mysql);
            return -1;
        }  

        if ((result = mysql_store_result(mysql)) == NULL)
        {
            cout << "mysql_store_result failed" << endl;
            mysql_free_result(result);
            mysql_close(mysql);
            return -1;
        }

        while ((row = mysql_fetch_row(result)) != NULL)
        {
            int ans;
            str2int(row[0],ans);
            file_del_addr_real(ans,row[1]);
        }
        
        /* 释放result */
        mysql_free_result(result);
    }

    /* 关闭整个连接 */
    mysql_close(mysql);
    return 1;
}


/********************************************
直接调用mysql修改文件路径信息
********************************************/
int file_modify_addr_real(int id,const char* new_file_name){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"UPDATE file_md5_info set file_name='%s' where file_id=%d", new_file_name,id);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* 关闭整个连接 */
    mysql_close(mysql);
    return 1;
}

/********************************************
根据文件编号查找文件的md5码
-1失败，1成功
********************************************/
int find_file_md5(int id,char* md5){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"select file_md5 from file_md5_info WHERE file_id=%d", id);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        mysql_free_result(result);
        /* 关闭整个连接 */
        mysql_close(mysql);
        return -1;
    }

    if((int)mysql_num_rows(result)==0){
        mysql_free_result(result);
        /* 关闭整个连接 */
        mysql_close(mysql);
        return -1;
    }

    row = mysql_fetch_row(result);

    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);
    if(row[0]) 
        strcpy(md5,row[0]);
    return 1;
}

/********************************************
修改文件更新时间
-1失败，1成功
********************************************/
int modify_file_time(int id,int tm){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"UPDATE file_md5_info set file_modifytime=%d where file_id=%d", tm,id);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    /* 关闭整个连接 */
    mysql_close(mysql);
    return 1;
} 

/********************************************
修改文件名称
-1失败，1成功
********************************************/
int modify_file_name(int id,const char* filename){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"UPDATE file_md5_info SET file_name='%s' WHERE file_id=%d;",filename,id);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    /* 关闭整个连接 */
    mysql_close(mysql);
    return 1;
} 

/********************************************
查找一个文件夹下指定md5码的文件名称
-1失败，0不存在，1存在
********************************************/
int find_md5_file(const char *md5,int parent,char* filename){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"select file_name from file_md5_info WHERE file_md5='%s' and file_parent=%d", md5,parent);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        return -1;
    }

    if((int)mysql_num_rows(result)==0){
        /* 释放result */
        mysql_free_result(result);
        /* 关闭整个连接 */
        mysql_close(mysql);
        return 0;
    }

    row = mysql_fetch_row(result);
    
    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);

    if(row[0]) 
        strcpy(filename,row[0]);
    return 1;
}


/********************************************
根据用户以及路径，
1、添加文件
    -1报错，0添加成功，1已存在
2、删除文件
    -1报错，0文件不存在，1删除成功
3、修改文件名
    -1报错，0文件不存在，1修改成功
********************************************/
int file_solve_addr(map<string, set<Socket_RIO*> >& all_rio,Socket_RIO* rio ,const char* user_id,  const char * file_name, int tm, int type,  const char * md5, const char* new_file_name){
    vector<string>address;
    decompose_addr(address, file_name);

    int father_id=find_file_id(user_id,0);
    for(int i=0;i<address.size()-1;i++){
        father_id=find_file_id(address[i].c_str(),father_id);
        if(father_id<=0){
            cout<<"address wrong"<<endl;
            return -1;
        }
    }

    int flag=find_file_id(address[address.size()-1].c_str(),father_id);
    if(flag==-1){//报错
        return -1;
    }
    else if(flag==0){//文件不存在
        cout<<"no such file"<<endl;
        if(type==ADDFILE_ADDR){
            if(md5==NULL){
                cout<<"folder"<<endl;
                file_add_addr_real(md5,address[address.size()-1].c_str(),father_id,tm);
            }
            else{
                char filename_server[256];
                int flagmd5=find_md5_file(md5,father_id,filename_server);
                if(flagmd5==0){
                    //不存在md5相同文件
                    cout<<"no such md5"<<endl;
                    file_add_addr_real(md5,address[address.size()-1].c_str(),father_id,tm);
                }
                else{
                    cout<<"md5 exist"<<endl;
                    cout<<"------------"<<endl;
                    cout<<"quick send"<<endl;
                    cout<<"------------"<<endl;
                    char uu_file_name[256];
                    strcpy(uu_file_name,file_name);
                    addr_change_back(user_id,uu_file_name,rio->cp_ip);
                    vector<string>address__;
                    decompose_addr(address__, uu_file_name);
                    string use="";
                    for(int i=0;i<address__.size()-1;i++){
                        use+=address__[i]+"/";
                    }
                    use+=filename_server;
                    cout<<uu_file_name<<" "<<use<<endl;
                    real_modifyname(uu_file_name, use.c_str(),rio);

                    string now=rio->user_id;
                    char usepp[256]={0},newuse[256]={0};
                    for(Socket_RIO* rio_now : all_rio[now]){
                        if(rio_now==rio) continue;
                        strcpy(usepp,uu_file_name);
                        strcpy(newuse,use.c_str());
                        if(addr_change_back(rio->user_id,usepp,rio_now->cp_ip)==1 && 
                            addr_change_back(rio->user_id,newuse,rio_now->cp_ip)==1){
                            real_modifyname(usepp,newuse,rio_now); 
                        }    
                    }
                }
            }
            
        }
        return 0;
    }
    else{//同名文件存在
        cout<<"same name file exist"<<endl;
        if(type==ADDFILE_ADDR){
            cout<<flag<<endl;
            if(md5==NULL){
                //文件夹
                cout<<"folder"<<endl;
            }
            else{
                char md5now[40]={0};
                find_file_md5(flag,md5now);
                if(strncmp(md5now,md5,32)==0){
                    //MD5相同，更新修改时间
                    cout<<"md5 exist"<<endl;
                    cout<<"------------"<<endl;
                    cout<<"quick send"<<endl;
                    cout<<"------------"<<endl;
                    modify_file_time(flag,tm);
                }
                else{
                    //MD5不同
                    cout<<"no such md5"<<endl;
                    string newfilename=address[address.size()-1]+".server";
                    int inow;
                    while((inow=find_file_id(newfilename.c_str(),father_id))!=0){
                        if(inow==-1) return -1;
                        else{
                            newfilename+=".server";
                        }
                    }


                    // real_modifyname(filename,newfilename,rio);

                    modify_file_name(flag,newfilename.c_str());
                    file_add_addr_real(md5,address[address.size()-1].c_str(),father_id,tm);
                } 
            }
            
        }
        if(type==DELFILE_ADDR){
            file_del_addr_real(flag,md5);
        }
        else if(type==MODIFYFILE_ADDR){
            vector<string>address_;
            decompose_addr(address_, new_file_name);
            file_modify_addr_real(flag,address_[address_.size()-1].c_str());
        }
        return 1;
    }
}


/********************************************
将客户端传来的实文件路径转化为服务端的虚路径
-1报错，0匹配失败，1匹配成功
********************************************/
int addr_change(const char* user_id,char* filename,const char* cp_ip){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"SELECT addr_server,addr_local FROM folder_info WHERE user_no='%s' AND isbind=1 AND cp_ip='%s'",user_id,cp_ip);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        return -1;
    }

    char* newfilename=new char[260];
    bool flag=0;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        if(strncmp(row[1],filename,strlen(row[1]))==0 ){
            strcpy(newfilename,row[0]);
            strcat(newfilename,&filename[strlen(row[1])]);
            strcpy(filename,newfilename);
            flag=1;
            break;
        }
    }
    
    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);
    delete[] newfilename;
    
    if(flag==0){
        cout<<"address wrong"<<endl;
        return 0;
    }
    else{
        return 1;
    }
}

/********************************************
将服务端的虚路径转化为客户端的实文件路径
-1报错，0匹配失败，1匹配成功
********************************************/
int addr_change_back(const char* user_id,char* filename,const char* cp_ip){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"SELECT addr_server,addr_local FROM folder_info WHERE user_no='%s' AND isbind=1 AND cp_ip='%s'",user_id,cp_ip);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        return -1;
    }

    char* newfilename=new char[260];
    bool flag=0;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        if(strncmp(row[0],filename,strlen(row[0]))==0 ){
            strcpy(newfilename,row[1]);
            strcat(newfilename,&filename[strlen(row[0])]);
            strcpy(filename,newfilename);
            flag=1;
            break;
        }
    }
    
    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);
    delete[] newfilename;
    
    if(flag==0){
        cout<<"address wrong back"<<endl;
        return 0;
    }
    else{
        return 1;
    }
}

/********************************************
添加md5信息
-1失败，1成功
********************************************/
int file_md5_add(const char* md5){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"insert into file_num (file_md5,file_num)values('%s',0)",md5);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    /* 关闭整个连接 */
    mysql_close(mysql);
    return 1;
}

/********************************************
添加网络端虚拟文件夹
-1失败，1成功
********************************************/
int add_folder(const char* user_name ,const char* foldername,const char* cp_ip){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"INSERT INTO folder_info (user_no,addr_server,addr_local,isbind,cp_ip)VALUES('%s','%s',NULL,0,'%s')",user_name,foldername,cp_ip);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    /* 关闭整个连接 */
    mysql_close(mysql);
    return 1;
}


/********************************************
寻找同步盘本地地址
-1失败，1成功
********************************************/
int find_bind_folder(const char* user_name,char* bindAddr,const char* cp_ip){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"SELECT addr_local FROM folder_info WHERE user_no='%s' AND isbind=1 AND cp_ip='%s'",user_name,cp_ip);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    if ((row = mysql_fetch_row(result)) != NULL)
    {
        if(row[0]) 
            strcpy(bindAddr,row[0]);
    }
    
    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);
    
    return 1;
}

/********************************************
注册
-1错误，0账户重复，1成功
********************************************/
int try_register(const char *username, const char *passwd){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    /* 连接数据库，失败返回NULL
            1、mysqld没运行
            2、没有指定名称的数据库存在 */
    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    /* 设置字符集，否则读出的字符乱码，即使/etc/my.cnf中设置也不行 */
    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"select * from user_info WHERE user_no='%s'", username);
    /* 进行查询，成功返回0，不成功非0
            1、查询字符串存在语法错误
            2、查询不存在的数据表 */
    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    MYSQL_RES *result;
    /* 将查询结果存储起来，出现错误则返回NULL
        注意：查询结果为NULL，不会返回NULL */
    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        return -1;
    }

    bool flag=((int)mysql_num_rows(result)!=0);

    if(flag){
        cout<<"account already exist"<<endl;
    }
    else{
        cout<<"add new account success "<<username<<endl;
        sprintf(buff,"insert into user_info (user_no,user_passwd)values('%s','%s')", username, md5(passwd,strlen(passwd)).c_str());
        /* 进行查询，成功返回0，不成功非0
                1、查询字符串存在语法错误
                2、查询不存在的数据表 */
        if (mysql_query(mysql, buff))
        {
            cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
            mysql_close(mysql);
            return -1;
        }

        
        file_add_addr_real(NULL,username,0,time(0));//虚目录添加
        int Id__=find_file_id(username, 0);
        file_add_addr_real(NULL,"ShareDir0",Id__,time(0));
        file_add_addr_real(NULL,"ShareDir1",Id__,time(0));
        file_add_addr_real(NULL,"ShareDir2",Id__,time(0));
        file_add_addr_real(NULL,"ShareDir3",Id__,time(0));
        file_add_addr_real(NULL,"ShareDir4",Id__,time(0));
        file_add_addr_real(NULL,"ShareDir5",Id__,time(0));
    }

    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);

    if(flag){
        return 0;
    }
    else{
        return 1;
    }
}


int init_folder_add(const char* user_name,const char* cp_ip){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"SELECT * FROM folder_info WHERE user_no='%s' AND cp_ip='%s'",user_name,cp_ip);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    if ((row = mysql_fetch_row(result)) == NULL)
    {
        add_folder(user_name,"ShareDir0",cp_ip); //添加文件夹信息
        add_folder(user_name,"ShareDir1",cp_ip); //添加文件夹信息
        add_folder(user_name,"ShareDir2",cp_ip); //添加文件夹信息
        add_folder(user_name,"ShareDir3",cp_ip); //添加文件夹信息
        add_folder(user_name,"ShareDir4",cp_ip); //添加文件夹信息
        add_folder(user_name,"ShareDir5",cp_ip); //添加文件夹信息
    }
    
    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);
    
    return 1;
}


/********************************************
注册
-1失败，账号不存在-2，密码错误0，1成功
********************************************/
int try_login(const char *username, const char *passwd,const char* cp_ip,char * bindAddr,char * bindAddr_server){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"select * from user_info WHERE user_no='%s'", username);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    MYSQL_RES *result;

    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        return -1;
    }

    bool flag=((int)mysql_num_rows(result)!=0);
    bool login=false;

    if(flag){
        MYSQL_ROW row;
        row = mysql_fetch_row(result);
        if(strncmp(md5(passwd,strlen(passwd)).c_str(),row[2],32)==0){
            login=true;
            find_bind_folder(username,bindAddr,cp_ip);
            find_bind_folder_server(username,bindAddr_server,cp_ip);

            init_folder_add(username,cp_ip);
        }
    }

    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);

    if(!flag){
        return countWrong;
    }
    else if(!login){
        return passwdWrong;
    }
    else{
        return YES;
    }
}

/********************************************
查找指定用户、客户端地址的文件夹编号
-1失败，0未找到，其他返回编号
********************************************/
int find_bind_id_local(const char *user_id,const char *addrLocal,const char* cp_ip){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"SELECT folder_info_id FROM folder_info WHERE user_no='%s' AND addr_local='%s' AND cp_ip='%s'",user_id,addrLocal,cp_ip);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    int flag=0;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        str2int(row[0],flag);
    }
    
    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);
    
    return flag;
}


/********************************************
修改绑定文件夹的绑定信息
-1失败，1成功
********************************************/
int change_file_bind(int id,int value,const char* addrLocal){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"UPDATE folder_info SET isbind=%d WHERE folder_info_id=%d;",value,id);
    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }
    sprintf(buff,"UPDATE folder_info SET addr_local='%s' WHERE folder_info_id=%d",addrLocal,id);
    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    /* 关闭整个连接 */
    mysql_close(mysql);
    return 1;
}

/********************************************
文件夹解绑
-1失败，1成功
********************************************/
int try_unbundling(const char *user_id,const char *addrLocal,const char* cp_ip){
    int flag = find_bind_id_local(user_id,addrLocal,cp_ip);
    if(flag==-1){
        cout<<addrLocal<<" unbundling failed,sql wrong"<<endl;
        return -1;
    }
    else if(flag==0){
        cout<<addrLocal<<" unbundling failed,no such address"<<endl;
        return -1;
    }
    else{
        if(change_file_bind(flag,0,"")==-1){
            cout<<addrLocal<<" unbundling failed,sql wrong"<<endl;
            return -1;
        }
        else{
            cout<<addrLocal<<" unbundling success"<<endl;
            return 1;
        }
    }
}


/********************************************
查找指定用户、服务端地址的文件夹编号
-1失败，0未找到，其他返回编号
********************************************/
int find_bind_id_server(const char *user_id,const char *addrServer,const char* cp_ip){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"SELECT folder_info_id FROM folder_info WHERE user_no='%s' AND addr_server='%s' AND cp_ip='%s'",user_id,addrServer,cp_ip);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        return -1;
    }

    int flag=0;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        str2int(row[0],flag);
    }
    
    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);
    
    return flag;
}

/********************************************
文件夹绑定
-1失败，1成功
********************************************/
int try_bundling(const char * user_id,const char *addrLocal,const char *addrServer,const char* cp_ip){
    int flag = find_bind_id_server(user_id,addrServer,cp_ip);
    if(flag==-1){
        cout<<addrLocal<<" "<<addrServer<<" bundling failed,sql falied"<<endl;
        return -1;
    }
    else if(flag==0){
        cout<<addrLocal<<" "<<addrServer<<"bundling failed,no such address"<<endl;
        return -1;
    }
    else{
        // cout<<addrLocal<<endl;
        if(change_file_bind(flag,1,addrLocal)==-1){
            cout<<addrLocal<<" "<<addrServer<<" bundling failed,sql wrong"<<endl;
            return -1;
        }
        else{
            cout<<addrLocal<<" "<<addrServer<<" bundling success"<<endl;
            return 1;
        }
    }
}


/********************************************
向客户端同步文件，每发一个包死循环接收
********************************************/
int init_file(Socket_RIO* rio,int id,const char* base,string more){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"select * from file_md5_info WHERE file_parent=%d", id);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        return -1;
    }

    REQUEST response;
    ofstream fout;
    int tim;
    string mmore;
    string use;
    
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        mmore=more;
        mmore+="/";
        mmore+=row[2];
        use=base+mmore;
        
        str2int(row[4],tim);

        int outflag=(real_addfile(use.c_str(),strcmp(row[1],"(null)")?row[1]:NULL,tim,rio)==0);
        while(outflag==0){
            int lnow=rio->rlen();
            if(lnow==0){
                usleep(1000);
                // cout<<"lnow==0"<<endl;
                continue;
            }
            int length;
            rio->peek(&length);
            if (lnow < length){
                usleep(1000);
                // cout<<lnow<<"-"<<length<<endl;
                continue;
            }
            

            rio->read(&response, length);
            if(response.base.type==sendFile){
                fout.open("log.log",ofstream::app);
                printTime(time(0),fout);
                fout<<"recv sendFile from"<<rio->user_id<<"-"<<rio->cp_ip<<endl;
                fout.close();
                if(sendfile(response,0,rio)==1){
                    cout<<"send already success"<<endl;
                    break;
                }
            }
            else if(response.base.type==responseBase){
                fout.open("log.log",ofstream::app);
                printTime(time(0),fout);
                fout<<"recv responseBase"<<endl;
                fout.close();
                if(response.response_base.status==FILE_EXIST){
                    cout<<"dont need send"<<endl;
                    break;
                }
            }
            else{
                cout<<response.base.type<<endl;
                cout<<"error"<<endl;
                break;
            }  
        }
        int ans;
        str2int(row[0],ans);
        init_file(rio,ans,base,mmore);
    }

    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);

    return 1;
}

/**************************
根据本地路径，找服务器绑定路径
***********************/
int find_bind_folder_server(const char* user_name,char* bindAddr,const char* cp_ip){
    MYSQL *mysql;
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "u1851506","u1851506","db1851506", 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    mysql_set_character_set(mysql, "gbk");

    char buff[500];
    sprintf(buff,"SELECT addr_server FROM folder_info WHERE user_no='%s' AND isbind=1 AND cp_ip='%s'",user_name,cp_ip);

    if (mysql_query(mysql, buff))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        return -1;
    }

    if ((row = mysql_fetch_row(result)) != NULL)
    {
        if(row[0]) 
            strcpy(bindAddr,row[0]);
    }
    
    /* 释放result */
    mysql_free_result(result);
    /* 关闭整个连接 */
    mysql_close(mysql);
    
    return 1;
}

/**************************
服务端向客户端同步函数
***********************/
void sql_init_synchronous(Socket_RIO* rio){
    char addrServer[256]={0};
    find_bind_folder_server(rio->user_id,addrServer,rio->cp_ip);

    int file_id = find_file_id(addrServer, find_file_id(rio->user_id,0));//找到该文件编号
    
    // cout<<file_id<<endl;
    // cout<<addrServer<<endl;
    addr_change_back(rio->user_id, addrServer,rio->cp_ip);
    string more="";
    // cout<<addrServer<<endl;
    init_file(rio,file_id,addrServer,more);
    real_responsebase(0,INIT_END,rio);
}

