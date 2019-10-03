#ifndef JIGPASSWORDVALIDATOR_H
#define JIGPASSWORDVALIDATOR_H

#include <QDialog>

namespace Ui {
class JigLoginDialog;
}

class JigLoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JigLoginDialog(QWidget *parent = nullptr);
    ~JigLoginDialog();

signals:
    void isLogin(bool value);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::JigLoginDialog *ui;
};

#endif // JIGPASSWORDVALIDATOR_H
