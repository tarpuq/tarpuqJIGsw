#include "messagesform.h"
#include "ui_messagesform.h"

#include <QScrollBar>

MessagesForm::MessagesForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessagesForm)
{
    ui->setupUi(this);
}

MessagesForm::~MessagesForm()
{
    delete ui;
}

void MessagesForm::setOutputMessage(QString msg)
{
    auto textCursor = ui->textBrowser_output->textCursor();
    textCursor.movePosition(QTextCursor::End);
    ui->textBrowser_output->setTextCursor(textCursor);

    ui->textBrowser_output->insertHtml(msg);
    ui->textBrowser_output->verticalScrollBar()->setValue(ui->textBrowser_output->verticalScrollBar()->maximum());
}

void MessagesForm::setDebugMessage(QString msg)
{
    auto textCursor = ui->textBrowser_debug->textCursor();
    textCursor.movePosition(QTextCursor::End);
    ui->textBrowser_debug->setTextCursor(textCursor);

    ui->textBrowser_debug->insertHtml(msg);
    ui->textBrowser_debug->verticalScrollBar()->setValue(ui->textBrowser_debug->verticalScrollBar()->maximum());
}

void MessagesForm::clearOutputMessage()
{
    ui->textBrowser_output->clear();
}

void MessagesForm::clearDebugMessage()
{
    ui->textBrowser_debug->clear();
}
