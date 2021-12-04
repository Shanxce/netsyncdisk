#include "mainwindow.h"
#include <QApplication>
#include<iostream>
using namespace std;

#include <qtextcodec.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    while(true)
    {
        int login_ret=w.Dialog_Login->exec();
        if(login_ret==QDialog::Accepted)
         {
            break;
        }
        else if(login_ret==QDialog::Rejected)
            return 0;
        else
        {
            if(w.Dialog_Register->exec()==QDialog::Rejected)
                return 0;
        }
    }
    w.show();

    return a.exec();
}
