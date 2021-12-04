#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include<atomic>
#include"communicator.h"
#include<QThread>
#include<iostream>
#include"login.h"
#include"register.h"
namespace Ui {
class MainWindow;
}
extern std::atomic<int> Share_ifBind;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Register*Dialog_Register;
    Login*Dialog_Login;
    int ifBind;
    QString SourcePath,RemotePath;
    
private:
    Ui::MainWindow *ui;
    Communicator *communicator;
    QThread * CommunicatorThread;

    void ShowShareDir();

public slots:
    void setBind(QString Source, QString Remote,int Status);
    void Reply_SetBind(QString Source, QString Remote,int Status);
signals:
    void Signal_Connect();
    void Signal_Listen();
    void Signal_SendSetBind(QString Source, QString Remote,int Status);
    
public slots:
    void Log(QString logcontent);
    void on_Button_PathConnect_clicked();
    void deal_Connect_Status(int Status);
};

#endif // MAINWINDOW_H
