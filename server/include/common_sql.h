#pragma once
#include <iostream> // cin,cout等
#include <fstream>
#include <iomanip>  // setw等
#include <mysql.h>  // mysql特有
#include <vector>
#include <string.h>
#include "common_tongbu_server.h"
#include "common_md5.h"
#include "common_RIO_server.h"
#include "common_order.h"
using namespace std;

#define ADDFILE_ADDR 1
#define DELFILE_ADDR 2
#define MODIFYFILE_ADDR 3

int file_add_addr_real(const char* md5,const char * name,int parent,int tm);
int file_del_addr_real(int id,const char* md5);
int file_modify_addr_real(int id,const char* new_file_name);

int find_file_md5(int id,char* md5);
void decompose_addr(vector<string>& address, const char * file_name);
bool file_md5_exist(const char* file_md5);
int find_file_id(const char * file_name, int parent_id);
int modify_file_time(int id,int tm);
int modify_file_name(int id,const char* filename);
int find_md5_file(const char *md5,int parent,char* filename);

int file_solve_addr(map<string, set<Socket_RIO*> >& all_rio, Socket_RIO* rio, const char* user_id,  const char * file_name, int tm, int type,  const char * md5=NULL, const char* new_file_name=NULL);
int addr_change(const char* user_id,char* filename,const char* cp_ip);
int addr_change_back(const char* user_id,char* filename,const char* cp_ip);
int file_md5_add(const char* md5);
int add_folder(const char* user_name ,const char* foldername,const char* cp_ip);
int find_bind_folder(const char* user_name,char* bindAddr,const char* cp_ip);

int try_login(const char *username, const char *passwd,const char* cp_ip,char * bindAddr,char * bindAddr_server);
int try_register(const char *username, const char *passwd);

int find_bind_id_local(const char *user_id,const char *addrLocal,const char* cp_ip);
int change_file_bind(int id,int value,const char* addrLocal);
int try_unbundling(const char *user_id,const char *addrLocal,const char* cp_ip);
int find_bind_id_server(const char *user_id,const char *addrServer,const char* cp_ip);
int try_bundling(const char * user_id,const char *addrLocal,const char *addrServer,const char* cp_ip);

int init_file(Socket_RIO* rio,int id,const char* base,string more);
int find_bind_folder_server(const char* user_name,char* bindAddr,const char* cp_ip);
void sql_init_synchronous(Socket_RIO* rio);


