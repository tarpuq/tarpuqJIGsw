#ifndef JIGCOMMANDEDIT_H
#define JIGCOMMANDEDIT_H

#include "jigsynccommand.h"
#include <QDialog>
#include <QStringListModel>

#include "jiginterface.h"

namespace Ui {
class CommandEditDialog;
}

class CommandEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommandEditDialog(QWidget *parent = nullptr);
    ~CommandEditDialog();

    void loadCommand(int index, JigSyncCommand *cmd);

    void setTtyCommandsList(const QStringList &value);

    void setPluginCommandsList(const QStringList &value);

    void setUsbCommandsList(const QStringList &value);

public slots:
    void setInterfaces(QHash<QString, JigInterface *> *interfacesList);

    void setAppCommands(QStringList commandsList);

private slots:

    void on_buttonBox_accepted();

    void on_doubleSpinBox_meanFixed_valueChanged(double arg1);

    void on_doubleSpinBox_deviation_valueChanged(double arg1);

    void on_checkBox_meanFormula_toggled(bool checked);

    void on_comboBox_tool_currentIndexChanged(const QString &arg1);

private:
    Ui::CommandEditDialog *ui;
    JigSyncCommand *command;
    QHash<QString, JigInterface *> *interfaces;
    QStringListModel *interfacesList;
    QStringListModel *commandsList;
    QStringList ttyCommandList;
    QStringList pluginCommandList;
    QStringList usbCommandList;
    QStringList appCommandList;
    int index;
};

#endif // JIGCOMMANDEDIT_H
