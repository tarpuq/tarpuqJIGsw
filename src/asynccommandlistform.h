#ifndef ASYNCCOMMANDLIST_H
#define ASYNCCOMMANDLIST_H

#include <QWidget>
#include "jigsynccommand.h"

namespace Ui {
class AsyncCommandListForm;
}

class AsyncCommandListForm : public QWidget
{
    Q_OBJECT

public:
    explicit AsyncCommandListForm(QWidget *parent = nullptr);
    ~AsyncCommandListForm();

    QList<JigSyncCommand *> *getCommandList() const;
    void setCommandList(QList<JigSyncCommand *> *value);

    void refreshButtons();

private slots:
    void on_pushButton_opcional1_clicked();

    void on_pushButton_opcional2_clicked();

    void on_pushButton_opcional3_clicked();

    void on_pushButton_opcional4_clicked();

    void on_pushButton_opcional5_clicked();

    void on_pushButton_opcional6_clicked();

    void on_pushButton_opcional7_clicked();

    void on_pushButton_opcional8_clicked();

    void on_pushButton_opcional9_clicked();

    void on_actionEditOptionalCommand_triggered();

signals:
    void opcionalCommandExecuted(JigSyncCommand *cmd);

private:
    Ui::AsyncCommandListForm *ui;
    QList<JigSyncCommand *> *commandList;

};

#endif // ASYNCCOMMANDLIST_H
