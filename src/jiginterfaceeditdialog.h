#ifndef JIGINTERFACEEDIT_H
#define JIGINTERFACEEDIT_H

#include <QDialog>
#include <QStringListModel>

#include "jiginterface.h"

#include <QLabel>
#include <QTableWidgetItem>

namespace Ui {
class InterfaceEditDialog;
}

class InterfaceEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InterfaceEditDialog(QWidget *parent = nullptr);
    ~InterfaceEditDialog();

    QList<JigInterface *> interfaces;
    QList<QHash<QString, QVariant>> parametersIndex;

    void setInterfaces(QList<JigInterface *> interfaces);

    int currentInterfaceIndex;

private slots:
    void on_listWidget_currentRowChanged(int currentInterfaceIndex);

    void on_comboBox_activated(int index);

    void on_buttonBox_accepted();

    void on_tableWidget_cellChanged(int row, int column);

private:
    Ui::InterfaceEditDialog *ui;
    QList<JigInterface::JigInterfaceType> typesIndex;
    QStringListModel *types;
};

#endif // JIGINTERFACEEDIT_H
