#ifndef JIGCOMMANDEDIT_H
#define JIGCOMMANDEDIT_H

#include "jigsynccommand.h"
#include <QDialog>
#include <QStringListModel>

namespace Ui {
class JigCommandEditDialog;
}

class JigCommandEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JigCommandEditDialog(QWidget *parent = nullptr);
    ~JigCommandEditDialog();

    void loadCommand(int index, JigSyncCommand *cmd);

public slots:
    void setInterfaces(QStringList interfaces);

    void setCommands(QStringList commands);

private slots:

    void on_buttonBox_accepted();

    void on_doubleSpinBox_mean_valueChanged(double arg1);

    void on_doubleSpinBox_deviation_valueChanged(double arg1);

    void on_checkBox_formula_toggled(bool checked);

private:
    Ui::JigCommandEditDialog *ui;
    JigSyncCommand *command;
    QStringListModel *interfaces;
    QStringListModel *commands;
    int index;
};

#endif // JIGCOMMANDEDIT_H
