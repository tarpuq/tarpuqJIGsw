#ifndef JIGPASSWORDVALIDATOR_H
#define JIGPASSWORDVALIDATOR_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

signals:
    void isLogin(bool value);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::LoginDialog *ui;
};

#endif // JIGPASSWORDVALIDATOR_H
