#ifndef COMMANDLIST_H
#define COMMANDLIST_H

#include <QWidget>
#include <QTableWidget>
#include <QList>

#include "jigsynccommand.h"
#include "jigcommandeditdialog.h"

namespace Ui {
class SyncCommandList;
}

class SyncCommandList : public QWidget
{
    Q_OBJECT

public:
    explicit SyncCommandList(QWidget *parent = nullptr);
    ~SyncCommandList();

    void clearTable();
    void insertCommand(JigSyncCommand *cmd);

    QList<JigSyncCommand *> *getCommandList() const;
    void setCommandList(QList<JigSyncCommand *> *value);

    void moveCommand(int from, int to);

    void scrollToTop();
    void scrollToItem(QTableWidgetItem *item);

private slots:
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);

    void on_actionEdit_triggered();

    void on_actionInsert_triggered();

    void on_actionRemove_triggered();

    void on_actionDuplicate_triggered();

    void on_actionMove_Up_triggered();

    void on_actionMove_Down_triggered();

    void on_actionMove_Last_triggered();

    void on_actionMove_First_triggered();

    void on_checkBox_stateChanged(int arg1);

    void on_tableWidget_itemChanged(QTableWidgetItem *item);

signals:
    void commandEdited(JigSyncCommand *cmd);

private:
    Ui::SyncCommandList *ui;
    int tableRowReference;
    QList<JigSyncCommand *> *commandList;

};

#endif // COMMANDLIST_H
