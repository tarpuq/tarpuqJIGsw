#include "commandlist.h"
#include "ui_commandlist.h"

#include <QMenu>
#include <QMdiSubWindow>

CommandList::CommandList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommandList)
{
    ui->setupUi(this);

    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

CommandList::~CommandList()
{
    delete ui;
}

void CommandList::clearTable()
{
    ui->tableWidget->clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(5);

    QStringList headers;
    headers << "Prueba"
            << "Mínimo"
            << "Medido"
            << "Máximo"
            << "Estado";
    ui->tableWidget->setHorizontalHeaderLabels(headers);
}

void CommandList::insertCommand(JigSyncCommand *cmd)
{
    int currentRow = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(currentRow);

    ui->tableWidget->setItem(currentRow, 0, cmd->nameItem);
    ui->tableWidget->setItem(currentRow, 1, cmd->minItem);
    ui->tableWidget->setItem(currentRow, 2, cmd->measuredItem);
    ui->tableWidget->setItem(currentRow, 3, cmd->maxItem);
    ui->tableWidget->setItem(currentRow, 4, cmd->resultItem);

    ui->tableWidget->resizeColumnToContents(0);
    ui->tableWidget->setColumnWidth(1, 125);
    ui->tableWidget->setMaximumWidth(
        ui->tableWidget->columnWidth(0) + ui->tableWidget->columnWidth(1)
        + ui->tableWidget->columnWidth(2) + ui->tableWidget->columnWidth(3)
        + ui->tableWidget->columnWidth(4) + 50);

    static_cast<QMdiSubWindow *>(this->parent())->resize(ui->tableWidget->maximumWidth(), 500);
}

void CommandList::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu(this);
    QModelIndex cell = ui->tableWidget->indexAt(pos);
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
        menu.exec(ui->tableWidget->mapToGlobal(pos));
    }
}

void CommandList::on_actionEdit_triggered()
{
    emit commandEdited(commandList->at(tableRowReference));
}

void CommandList::on_actionInsert_triggered()
{
    JigSyncCommand *command = new JigSyncCommand(this);

    command->setOnOk("0");
    command->setOnError("1");
    command->setTimeOut(3000.0);
    command->setId("id");
    command->setName("Nuevo Comando");

    ui->tableWidget->insertRow(tableRowReference);
    ui->tableWidget->setItem(tableRowReference, 0, command->nameItem);
    ui->tableWidget->setItem(tableRowReference, 1, command->minItem);
    ui->tableWidget->setItem(tableRowReference, 2, command->measuredItem);
    ui->tableWidget->setItem(tableRowReference, 3, command->maxItem);
    ui->tableWidget->setItem(tableRowReference, 4, command->resultItem);

    commandList->insert(tableRowReference,command);
}

void CommandList::on_actionRemove_triggered()
{
    commandList->at(tableRowReference)->deleteLater();
    commandList->removeAt(tableRowReference);
    ui->tableWidget->removeRow(tableRowReference);
}

void CommandList::on_actionDuplicate_triggered()
{
    JigSyncCommand *duplicated = new JigSyncCommand(this);
    JigSyncCommand *original = commandList->at(tableRowReference);
    *duplicated = *original;

    commandList->insert(tableRowReference, duplicated);

    ui->tableWidget->insertRow(tableRowReference);
    ui->tableWidget->setItem(tableRowReference, 0, duplicated->nameItem);
    ui->tableWidget->setItem(tableRowReference, 1, duplicated->minItem);
    ui->tableWidget->setItem(tableRowReference, 2, duplicated->measuredItem);
    ui->tableWidget->setItem(tableRowReference, 3, duplicated->maxItem);
    ui->tableWidget->setItem(tableRowReference, 4, duplicated->resultItem);

}

QList<JigSyncCommand *> *CommandList::getCommandList() const
{
    return commandList;
}

void CommandList::setCommandList(QList<JigSyncCommand *> *value)
{
    commandList = value;
}

void CommandList::moveCommand(int from, int to)
{
    QList<QTableWidgetItem *> items;

    if ((from < 0) || (to < 0))
        return;

    if (to >= commandList->size())
        return;

    commandList->move(from,to);

    for (int i = 0; i < 5; i++)
        items << ui->tableWidget->takeItem(from, i);

    ui->tableWidget->removeRow(from);

    ui->tableWidget->insertRow(to);

    for (int i = 0; i < 5; i++)
        ui->tableWidget->setItem(to, i, items[i]);
}

void CommandList::scrollToTop()
{
    ui->tableWidget->scrollToTop();
}

void CommandList::scrollToItem(QTableWidgetItem *item)
{
    ui->tableWidget->scrollToItem(item);
}

void CommandList::on_actionMove_Up_triggered()
{
    moveCommand(tableRowReference, tableRowReference - 1);
}

void CommandList::on_actionMove_Down_triggered()
{
    moveCommand(tableRowReference, tableRowReference + 1);
}

void CommandList::on_actionMove_Last_triggered()
{
    moveCommand(tableRowReference, commandList->size() - 1);
}

void CommandList::on_actionMove_First_triggered()
{
    moveCommand(tableRowReference, 0);
}

void CommandList::on_checkBox_stateChanged(int arg1)
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

void CommandList::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
    if (item->column() == 0) {
        if (item->checkState() == Qt::Checked) {
            commandList->at(item->row())->setEnabled(true);
        } else {
            commandList->at(item->row())->setEnabled(false);
        }
    }
}
