#ifndef JIGASYNCCOMMAND_H
#define JIGASYNCCOMMAND_H

#include <QObject>

class JigAsyncCommand : public QObject
{
    Q_OBJECT
public:
    explicit JigAsyncCommand(QObject *parent = nullptr);

signals:

public slots:
    void processCommand();
};

#endif // JIGASYNCCOMMAND_H
