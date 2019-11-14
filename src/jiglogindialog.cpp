#include "jiglogindialog.h"
#include "ui_jiglogindialog.h"

JigLoginDialog::JigLoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JigLoginDialog)
{
    ui->setupUi(this);
}

JigLoginDialog::~JigLoginDialog()
{
    delete ui;
}

void JigLoginDialog::on_buttonBox_accepted()
{
    if(ui->lineEdit->text() == "tarpuqPass"){
        emit isLogin(true);
    } else {
        emit isLogin(false);
    }
    ui->lineEdit->clear();
}
