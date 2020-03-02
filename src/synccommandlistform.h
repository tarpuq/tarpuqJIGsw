#ifndef COMMANDLIST_H
#define COMMANDLIST_H

#include <QWidget>
#include <QTableWidget>
#include <QList>

#include "jigsynccommand.h"
#include "commandeditdialog.h"

namespace Ui {
class SyncCommandListForm;
}

class SyncCommandListForm : public QWidget
{
    Q_OBJECT

public:
    explicit SyncCommandListForm(QWidget *parent = nullptr);
    ~SyncCommandListForm();

    void insertCommand(JigSyncCommand *cmd);

    QList<JigSyncCommand *> *getCommandList() const;
    void setCommandList(QList<JigSyncCommand *> *value);

    void moveCommand(int from, int to);

    void scrollToTop();
    void scrollToItem(QTreeWidgetItem *item);

    void cleanUI();

    void refreshUI();

private slots:
    void on_treeWidget_customContextMenuRequested(const QPoint &pos);

    void on_actionEdit_triggered();

    void on_actionInsert_triggered();

    void on_actionRemove_triggered();

    void on_actionDuplicate_triggered();

    void on_actionMove_Up_triggered();

    void on_actionMove_Down_triggered();

    void on_actionMove_Last_triggered();

    void on_actionMove_First_triggered();

    void on_checkBox_stateChanged(int arg1);

    void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

signals:
    void commandEdited(JigSyncCommand *cmd);

private:
    Ui::SyncCommandListForm *ui;
    int tableRowReference;
    QList<JigSyncCommand *> *commandList;

};

#endif // COMMANDLIST_H
