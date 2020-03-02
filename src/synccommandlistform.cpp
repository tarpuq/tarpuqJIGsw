#include "synccommandlistform.h"
#include "ui_synccommandlistform.h"

#include <QMenu>
#include <QDebug>

SyncCommandListForm::SyncCommandListForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SyncCommandListForm)
{
    ui->setupUi(this);

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

SyncCommandListForm::~SyncCommandListForm()
{
    delete ui;
}

void SyncCommandListForm::insertCommand(JigSyncCommand *cmd)
{
    ui->treeWidget->addTopLevelItem(cmd->treeItem);

    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->setColumnWidth(1,100);
    ui->treeWidget->setColumnWidth(2,100);
    ui->treeWidget->setColumnWidth(3,100);
    ui->treeWidget->setColumnWidth(4,100);

    ui->treeWidget->setMaximumWidth(
        ui->treeWidget->columnWidth(0) + ui->treeWidget->columnWidth(1)
        + ui->treeWidget->columnWidth(2) + ui->treeWidget->columnWidth(3)
        + ui->treeWidget->columnWidth(4));

    static_cast<QMdiSubWindow *>(this->parent())->resize(ui->treeWidget->maximumWidth() + 50, 500);;
}

void SyncCommandListForm::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu(this);
    QModelIndex cell = ui->treeWidget->indexAt(pos);
    tableRowReference = cell.row();

    if (cell.isValid()) {
        menu.addAction(ui->actionEdit);
        menu.addAction(ui->actionInsert);
        menu.addAction(ui->actionRemove);
        menu.addSeparator();
        menu.addAction(ui->actionDuplicate);
        menu.addAction(ui->actionMove_Up);
        menu.addAction(ui->actionMove_Down);
        menu.addAction(ui->actionMove_First);
        menu.addAction(ui->actionMove_Last);
        menu.exec(ui->treeWidget->mapToGlobal(pos));
    }
}

void SyncCommandListForm::on_actionEdit_triggered()
{
    emit commandEdited(commandList->at(tableRowReference));
}

void SyncCommandListForm::on_actionInsert_triggered()
{
    JigSyncCommand *newCmd = new JigSyncCommand(this);

    newCmd->setOnOk("0");
    newCmd->setOnError("1");
    newCmd->setTimeOut(3000.0);
    newCmd->setId("id");
    newCmd->setName("Nuevo Comando");

    commandList->insert(tableRowReference,newCmd);
    ui->treeWidget->insertTopLevelItem(tableRowReference,newCmd->treeItem);
}

void SyncCommandListForm::on_actionRemove_triggered()
{
    QItemSelectionModel *select = ui->treeWidget->selectionModel();

    while(select->selectedRows().size()){
        tableRowReference = select->selectedRows().first().row();
        commandList->at(tableRowReference)->deleteLater();
        commandList->removeAt(tableRowReference);
        ui->treeWidget->takeTopLevelItem(tableRowReference);
    }
}

void SyncCommandListForm::on_actionDuplicate_triggered()
{
    JigSyncCommand *duplicated = new JigSyncCommand(this);
    JigSyncCommand *original = commandList->at(tableRowReference);
    *duplicated = *original;

    commandList->insert(tableRowReference, duplicated);
    ui->treeWidget->insertTopLevelItem(tableRowReference, duplicated->treeItem);
}

QList<JigSyncCommand *> *SyncCommandListForm::getCommandList() const
{
    return commandList;
}

void SyncCommandListForm::setCommandList(QList<JigSyncCommand *> *value)
{
    commandList = value;
}

void SyncCommandListForm::moveCommand(int from, int to)
{
    if ((from < 0) || (to < 0))
        return;

    if (to >= commandList->size())
        return;

    commandList->move(from,to);

    QTreeWidgetItem *item = ui->treeWidget->takeTopLevelItem(from);
    ui->treeWidget->insertTopLevelItem(to, item);
}

void SyncCommandListForm::scrollToTop()
{
    ui->treeWidget->scrollToTop();
}

void SyncCommandListForm::scrollToItem(QTreeWidgetItem *item)
{
    ui->treeWidget->scrollToItem(item);
}

void SyncCommandListForm::cleanUI()
{
    foreach (JigSyncCommand *cmd, *commandList) {
        cmd->clearMeasure();
        cmd->setStatus(JigSyncCommand::pending);
    }
}

void SyncCommandListForm::refreshUI()
{
    ui->treeWidget->clear();

    foreach (JigSyncCommand *cmd, *commandList) {
        insertCommand(cmd);
    }
}

void SyncCommandListForm::on_actionMove_Up_triggered()
{
    moveCommand(tableRowReference, tableRowReference - 1);
}

void SyncCommandListForm::on_actionMove_Down_triggered()
{
    moveCommand(tableRowReference, tableRowReference + 1);
}

void SyncCommandListForm::on_actionMove_Last_triggered()
{
    moveCommand(tableRowReference, commandList->size() - 1);
}

void SyncCommandListForm::on_actionMove_First_triggered()
{
    moveCommand(tableRowReference, 0);
}

void SyncCommandListForm::on_checkBox_stateChanged(int arg1)
{
    if (arg1 == Qt::Unchecked) {
        foreach (JigSyncCommand *cmd, *commandList) {
            cmd->setEnabled(false);
        }
    } else if (arg1 == Qt::Checked) {
        foreach (JigSyncCommand *cmd, *commandList) {
            cmd->setEnabled(true);
        }
    }
}

void SyncCommandListForm::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    if(column == 0){
        foreach(JigSyncCommand *cmd, *commandList){
            if(item == cmd->treeItem){
                if (item->checkState(column) == Qt::Checked) {
                    cmd->setEnabled(true);
                } else {
                    cmd->setEnabled(false);
                }
            }
        }
    }
}
