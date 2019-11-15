#include "console.h"
#include "ui_console.h"

Console::Console(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Console)
{
    ui->setupUi(this);
}

Console::~Console()
{
    delete ui;
}

void Console::on_pushButton_clicked()
{
    ui->textBrowser->clear();
}
