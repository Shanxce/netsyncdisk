#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>

namespace Ui {
class Register;
}


class Register : public QDialog
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = 0);
    ~Register();

private:
    Ui::Register *ui;

signals:
    void Sigal_Register(QString username,QString passwd);
public slots:
    void Reply_Register(int Status);
    void on_Button_register_clicked();
    void on_Button_return_clicked();
};

#endif // REGISTER_H
