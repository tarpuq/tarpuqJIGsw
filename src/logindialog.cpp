#include "jiglogindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_buttonBox_accepted()
{
    if(ui->lineEdit->text() == "tarpuqPass"){
        emit isLogin(true);
    } else {
        emit isLogin(false);
    }
    ui->lineEdit->clear();
}
