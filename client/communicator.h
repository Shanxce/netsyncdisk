#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <QObject>
#include<map>
#include<string>
#include<atomic>
#include "include/common_tongbu.h"
#include "include/common_md5.h"
#include "include/common_RIO.h"
#include "include/common_order.h"
#include<string>
using namespace std;
#define PATH_LENS 256
#define PIPE_NAME "\\\\.\\Pipe\\test"
#define DEBUG(x) cout <<__LINE__<<"\t\t" #x << ':' << x << endl


#define server_IP "10.60.102.252"
#define server_Port 20161
struct EVENT
{
    DWORD Action;
    DWORD PathLength;
    char Path[PATH_LENS];
};
class Communicator : public QObject
{
    Q_OBJECT
public:
    explicit Communicator(QObject *parent = nullptr);

public:
    char  listen_path[PATH_LENS];
    int listen_path_lens;
    map<string, DWORD> UnFinshEvent;
    int client_fd;
    Socket_RIO *rio;
    bool ifconnect,ifbindaddr;
    int SyncStatus;
    REQUEST response;
    QString SourcePath,RemotePath;

private:
    void DealUnfinishEvents(map<string, DWORD> &events, Socket_RIO *rio);
    void deal_events(FILE_NOTIFY_INFORMATION *events, Socket_RIO *rio);
    int connect_server();
    void record_Log();

signals:
    void Signal_Reply_Login(int Status);
    void Signal_Reply_Register(int Status);
    void Signal_setBind(QString Source, QString Remote,int Status);
    void Signal_Reply_SendSetBind(QString Source, QString Remote,int Status);
    void Signal_Connect_Status(int Status);
    void Signal_Log(QString logcontent);
    
public slots:
    void Listen();
    void Connect();
    void Init_Sync();
    void Login(QString username,QString passwd);
    void Register(QString username,QString passwd);
    void SendSetBind(QString Source, QString Remote,int Status);
};

#endif // COMMUNICATOR_H
