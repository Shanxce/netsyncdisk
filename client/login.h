#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

private:
    Ui::Login *ui;
signals:
    void Sigal_Login(QString username,QString passwd);
public slots:
    void Reply_Login(int Status);
    void on_Button_login_clicked();
    void on_Button_register_clicked();
    void on_Button_exit_clicked();
};

#endif // LOGIN_H
