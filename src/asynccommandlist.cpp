#include "asynccommandlist.h"
#include "ui_asynccommandlist.h"
#include <QMessageBox>

AsyncCommandList::AsyncCommandList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AsyncCommandList)
{
    ui->setupUi(this);

    ui->pushButton_opcional1->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->pushButton_opcional1->addAction(ui->actionEditOptionalCommand);

    ui->pushButton_opcional2->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->pushButton_opcional2->addAction(ui->actionEditOptionalCommand);

    ui->pushButton_opcional3->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->pushButton_opcional3->addAction(ui->actionEditOptionalCommand);

    ui->pushButton_opcional4->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->pushButton_opcional4->addAction(ui->actionEditOptionalCommand);

    ui->pushButton_opcional5->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->pushButton_opcional5->addAction(ui->actionEditOptionalCommand);

    ui->pushButton_opcional6->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->pushButton_opcional6->addAction(ui->actionEditOptionalCommand);

    ui->pushButton_opcional7->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->pushButton_opcional7->addAction(ui->actionEditOptionalCommand);

    ui->pushButton_opcional8->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->pushButton_opcional8->addAction(ui->actionEditOptionalCommand);

    ui->pushButton_opcional9->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->pushButton_opcional9->addAction(ui->actionEditOptionalCommand);
}

AsyncCommandList::~AsyncCommandList()
{
    delete ui;
}

void AsyncCommandList::on_pushButton_opcional1_clicked()
{
    emit opcionalCommandExecuted(commandList->at(0));
}

void AsyncCommandList::on_pushButton_opcional2_clicked()
{
    emit opcionalCommandExecuted(commandList->at(1));
}

void AsyncCommandList::on_pushButton_opcional3_clicked()
{
    emit opcionalCommandExecuted(commandList->at(2));
}

void AsyncCommandList::on_pushButton_opcional4_clicked()
{
    emit opcionalCommandExecuted(commandList->at(3));
}

void AsyncCommandList::on_pushButton_opcional5_clicked()
{
    emit opcionalCommandExecuted(commandList->at(4));
}

void AsyncCommandList::on_pushButton_opcional6_clicked()
{
    emit opcionalCommandExecuted(commandList->at(5));
}

void AsyncCommandList::on_pushButton_opcional7_clicked()
{
    emit opcionalCommandExecuted(commandList->at(6));
}

void AsyncCommandList::on_pushButton_opcional8_clicked()
{
    emit opcionalCommandExecuted(commandList->at(8));
}

void AsyncCommandList::on_pushButton_opcional9_clicked()
{
    emit opcionalCommandExecuted(commandList->at(9));
}

QList<JigSyncCommand *> *AsyncCommandList::getCommandList() const
{
    return commandList;
}

void AsyncCommandList::setCommandList(QList<JigSyncCommand *> *value)
{
    commandList = value;
}

void AsyncCommandList::refreshButtons()
{
    if (commandList->size() > 0) {
        ui->pushButton_opcional1->setText(commandList->at(0)->getName());
        ui->pushButton_opcional2->setText(commandList->at(1)->getName());
        ui->pushButton_opcional3->setText(commandList->at(2)->getName());
        ui->pushButton_opcional4->setText(commandList->at(3)->getName());
        ui->pushButton_opcional5->setText(commandList->at(4)->getName());
        ui->pushButton_opcional6->setText(commandList->at(5)->getName());
        ui->pushButton_opcional7->setText(commandList->at(6)->getName());
        ui->pushButton_opcional8->setText(commandList->at(7)->getName());
        ui->pushButton_opcional9->setText(commandList->at(8)->getName());
        ui->pushButton_opcional1->setEnabled(true);
        ui->pushButton_opcional2->setEnabled(true);
        ui->pushButton_opcional3->setEnabled(true);
        ui->pushButton_opcional4->setEnabled(true);
        ui->pushButton_opcional5->setEnabled(true);
        ui->pushButton_opcional6->setEnabled(true);
        ui->pushButton_opcional7->setEnabled(true);
        ui->pushButton_opcional8->setEnabled(true);
        ui->pushButton_opcional9->setEnabled(true);
    } else {
        QMessageBox::warning(this, "Advertencia", "El perfil abierto no tiene comandos opcionales.");
        ui->pushButton_opcional1->setText("Opcional 1");
        ui->pushButton_opcional2->setText("Opcional 2");
        ui->pushButton_opcional3->setText("Opcional 3");
        ui->pushButton_opcional4->setText("Opcional 4");
        ui->pushButton_opcional5->setText("Opcional 5");
        ui->pushButton_opcional6->setText("Opcional 6");
        ui->pushButton_opcional7->setText("Opcional 7");
        ui->pushButton_opcional8->setText("Opcional 8");
        ui->pushButton_opcional9->setText("Opcional 9");
        ui->pushButton_opcional1->setEnabled(false);
        ui->pushButton_opcional2->setEnabled(false);
        ui->pushButton_opcional3->setEnabled(false);
        ui->pushButton_opcional4->setEnabled(false);
        ui->pushButton_opcional5->setEnabled(false);
        ui->pushButton_opcional6->setEnabled(false);
        ui->pushButton_opcional7->setEnabled(false);
        ui->pushButton_opcional8->setEnabled(false);
        ui->pushButton_opcional9->setEnabled(false);
    }
}

void AsyncCommandList::on_actionEditOptionalCommand_triggered()
{
    QMessageBox::information(this, "Editar comandos opcionales", "Proximamente");
}
