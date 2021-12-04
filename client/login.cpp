#include "login.h"
#include "ui_login.h"
enum STATUS{Login_PasswdFail,Login_UserFail,Login_sucess,Register_fail,Register_sucess,
            Register_Unstrong,Bind_Sourcenoexist,Bind_fail,Connect_dely};

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
}
void Login::on_Button_login_clicked()
{
    QString username=ui->Input_user->text();
    QString passwd=ui->Input_passwd->text();
    emit Sigal_Login(username,passwd);
}

void Login::on_Button_register_clicked()
{
    this->done(10);
}

void Login::on_Button_exit_clicked()
{
    this->reject();
}
void Login::Reply_Login(int Status)
{
    if(Status==Login_sucess)
    {
        this->accept();
    }
    else if(Status==Login_PasswdFail)
    {
        ui->Label_reminder->setText("密码错误！");
    }
    else if(Status==Login_UserFail)
    {
        ui->Label_reminder->setText("用户名错误!");
    }
    else
    {
        ui->Label_reminder->setText("网络未连接！");
    }
}
Login::~Login()
{
    delete ui;
}
