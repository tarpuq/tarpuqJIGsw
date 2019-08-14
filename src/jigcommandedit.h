#ifndef JIGCOMMANDEDIT_H
#define JIGCOMMANDEDIT_H

#include "jigcommand.h"
#include <QDialog>
#include <QStringListModel>

namespace Ui {
class JigCommandEdit;
}

class JigCommandEdit : public QDialog
{
    Q_OBJECT

public:
    explicit JigCommandEdit(QWidget *parent = nullptr);
    ~JigCommandEdit();

    void loadCommand(int index, JigCommand *cmd);

public slots:
    void setInterfaces(QStringList interfaces);

    void setCommands(QStringList commands);

private slots:

    void on_buttonBox_accepted();

    void on_doubleSpinBox_mean_valueChanged(double arg1);

    void on_doubleSpinBox_deviation_valueChanged(double arg1);

    void on_checkBox_formula_toggled(bool checked);

private:
    Ui::JigCommandEdit *ui;
    JigCommand *command;
    QStringListModel *interfaces;
    QStringListModel *commands;
    int index;
};

#endif // JIGCOMMANDEDIT_H
