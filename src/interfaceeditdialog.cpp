#include "jiginterfaceeditdialog.h"
#include "ui_interfaceeditdialog.h"

#include <QDebug>
#include <QHash>

InterfaceEditDialog::InterfaceEditDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InterfaceEditDialog)
{
    ui->setupUi(this);

    types = new QStringListModel(this);

    ui->comboBox->setModel(types);

    QStringList typesList;
    typesList << "app";
    typesList << "plugin";
    typesList << "usb";
    typesList << "tty";

    types->setStringList(typesList);

    ui->comboBox->setCurrentIndex(0);
}

InterfaceEditDialog::~InterfaceEditDialog()
{
    delete ui;
}

void InterfaceEditDialog::setInterfaces(QList<JigInterface *> interfaces)
{
    this->interfaces = interfaces;
    typesIndex.clear();
    parametersIndex.clear();
    ui->listWidget->clear();

    foreach (JigInterface *interface, interfaces) {
        typesIndex << interface->getType();
        parametersIndex << interface->parameters;
        ui->listWidget->addItem(interface->getAlias());
    }
}

void InterfaceEditDialog::on_listWidget_currentRowChanged(int currentRow)
{
    if (currentRow >= 0) {
        this->currentInterfaceIndex = currentRow;

        ui->lineEdit->setText(interfaces[currentRow]->getAlias());
        ui->comboBox->setCurrentIndex(typesIndex[currentRow] - 1);

        ui->tableWidget->clear();
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setHorizontalHeaderLabels(QList<QString>{"Campo", "Valor"});

        //      Parameters
        ui->tableWidget->blockSignals(true);
        for (int i = 0; i < interfaces[currentRow]->parameters.size(); i++) {
            QString fieldKey = parametersIndex[currentRow].keys().at(i);
            QVariant value = parametersIndex[currentRow][fieldKey];

            QTableWidgetItem *fieldItem = new QTableWidgetItem(fieldKey);
            QTableWidgetItem *valueItem = new QTableWidgetItem(value.toString());

            ui->tableWidget->insertRow(i);
            ui->tableWidget->setItem(i, 0, fieldItem);
            ui->tableWidget->setItem(i, 1, valueItem);
        }
        ui->tableWidget->blockSignals(false);
    }
}

void InterfaceEditDialog::on_comboBox_activated(int index)
{
    typesIndex[currentInterfaceIndex] = static_cast<JigInterface::JigInterfaceType>(index + 1);
}

void InterfaceEditDialog::on_buttonBox_accepted()
{
    for (int i = 0; i < interfaces.size(); i++) {
        interfaces[i]->setType(typesIndex[i]);
        interfaces[i]->parameters = parametersIndex[i];
    }
}

void InterfaceEditDialog::on_tableWidget_cellChanged(int row, int column)
{
    if (column == 1) {
        QString key = ui->tableWidget->item(row, 0)->text();
        QString value = ui->tableWidget->item(row, 1)->text();
        parametersIndex[currentInterfaceIndex][key] = value;
    }
}
