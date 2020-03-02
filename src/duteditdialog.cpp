#include "duteditdialog.h"
#include "ui_duteditdialog.h"

DutEditDialog::DutEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DutEditDialog)
{
    ui->setupUi(this);
}

DutEditDialog::~DutEditDialog()
{
    delete ui;
}

void DutEditDialog::setProfile(JigProfile *value)
{
    profile = value;
}

void DutEditDialog::refreshUI()
{
    ui->lineEdit_productCode->setText(profile->getProductCode());
    ui->lineEdit_productName->setText(profile->getProductName());
    ui->checkBox_isPanel->setChecked(profile->getIsPanelized());
    ui->spinBox_panelRows->setValue(profile->getPanelRows());
    ui->spinBox_panelCols->setValue(profile->getPanelCols());
    ui->lineEdit_fwPath->setText(profile->getFwPath());
}

void DutEditDialog::on_buttonBox_accepted()
{
    profile->setProductCode(ui->lineEdit_productCode->text());
    profile->setProductName(ui->lineEdit_productName->text());
    profile->setIsPanelized(ui->checkBox_isPanel->isChecked());
    profile->setPanelRows(ui->spinBox_panelRows->value());
    profile->setPanelCols(ui->spinBox_panelCols->value());
    profile->setFwPath(ui->lineEdit_fwPath->text());

    profile->setPanelAmount(profile->getPanelRows() * profile->getPanelCols());

    emit dutUpdated();
}
