#ifndef DUTEDITDIALOG_H
#define DUTEDITDIALOG_H

#include <QDialog>
#include "jigprofile.h"

namespace Ui {
class DutEditDialog;
}

class DutEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DutEditDialog(QWidget *parent = nullptr);
    ~DutEditDialog();

    void setProfile(JigProfile *value);

    void refreshUI();

signals:
    void dutUpdated();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DutEditDialog *ui;
    JigProfile *profile;
};

#endif // DUTEDITDIALOG_H
