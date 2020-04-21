#include "messagesform.h"
#include "ui_messagesform.h"

#include <QScrollBar>
#include <QDebug>

MessagesForm *debugOut;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();

    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }

    if(debugOut){
        debugOut->setDebugMessage(localMsg + "<br>");
    }
}

MessagesForm::MessagesForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessagesForm)
{
    ui->setupUi(this);

    debugOut = this;

    qInstallMessageHandler(myMessageOutput); // Install the handler
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

void MessagesForm::on_toolButton_triggered(QAction *arg1)
{
    qDebug() << arg1;
}
