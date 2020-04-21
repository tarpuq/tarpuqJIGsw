#include "duteditdialog.h"
#include "ui_duteditdialog.h"

#include <QFileDialog>
#include <QBuffer>
#include <QDebug>

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

    QPixmap top;
    top.loadFromData(QByteArray::fromBase64(profile->getTopPictureBase64()));
    ui->label_topPicture->setPixmap(top);

    QPixmap bottom;
    bottom.loadFromData(QByteArray::fromBase64(profile->getBottomPictureBase64()));
    ui->label_bottomPicture->setPixmap(bottom);
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

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    ui->label_topPicture->pixmap()->save(&buffer,"jpg");
    profile->setTopPictureBase64(buffer.data().toBase64());
    buffer.close();

    buffer.open(QIODevice::WriteOnly);
    ui->label_bottomPicture->pixmap()->save(&buffer,"jpg");
    profile->setBottomPictureBase64(buffer.data().toBase64());
    buffer.close();

    emit updated();
}

void DutEditDialog::on_pushButton_loadTopPicture_clicked()
{
    QString path;
    QString homePath = QDir::homePath();

    path = QFileDialog::getOpenFileName(this,
                                        "Ruta del perfil",
                                        homePath,
                                        "Imagenes (*.jpeg *.jpg *.png)");

    if(!path.isEmpty()){
        QPixmap pix;
        if(pix.load(path)){
            pix = pix.scaled(ui->label_topPicture->size(),Qt::KeepAspectRatio);
            ui->label_topPicture->setPixmap(pix);
        }
    }
}

void DutEditDialog::on_pushButton_loadBottomPicture_clicked()
{
    QString path;
    QString homePath = QDir::homePath();

    path = QFileDialog::getOpenFileName(this,
                                        "Ruta del perfil",
                                        homePath,
                                        "Imagenes (*.jpeg *.jpg *.png)");
    if(!path.isEmpty()){
        QPixmap pix;
        if(pix.load(path)){
            pix = pix.scaled(ui->label_bottomPicture->size(),Qt::KeepAspectRatio);
            ui->label_bottomPicture->setPixmap(pix);
        }
    }
}
