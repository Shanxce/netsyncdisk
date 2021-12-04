#include "register.h"
#include "ui_register.h"
enum STATUS{Login_PasswdFail,Login_UserFail,Login_sucess,Register_fail,Register_sucess,
            Register_Unstrong,Bind_Sourcenoexist,Bind_fail,Connect_dely};

Register::Register(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
}
void Register::on_Button_register_clicked()
{
    QString username=ui->Input_user->text();
    QString passwd=ui->Input_passwd->text();
    int Status=1;
    emit Sigal_Register(username,passwd);
}
void Register::on_Button_return_clicked()
{
    this->accept();
}
void Register::Reply_Register(int Status)
{
    if(Status==Register_sucess)
    {
        this->accept();
    }
    else if(Status==Register_Unstrong)
    {
        ui->Label_reminder->setText("密码强度不够！");
    }
    else
    {
        ui->Label_reminder->setText("用户名重复!");
    }
}

Register::~Register()
{
    delete ui;
}
