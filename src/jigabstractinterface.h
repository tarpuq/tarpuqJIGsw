#ifndef JIGABSTRACTINTERFACE_H
#define JIGABSTRACTINTERFACE_H

#include <QHash>
#include <QIODevice>
#include <QObject>
#include <QSerialPort>
#include <QVariant>

class JigAbstractInterface : public QObject
{
    Q_OBJECT
public:
    explicit JigAbstractInterface(QObject *parent = nullptr);

    enum JigInterfaceType { none, app, plugin, usb, tty };
    Q_ENUM(JigInterfaceType)

    QHash<QString, QVariant> parameters;

    QStringList getArguments() const;
    void setIfArguments(const QStringList &value);
    void setIfArguments(const QString &value, const QHash<QString, QByteArray> &wildcard);

    QStringList getAnswers() const;
    void setAnswers(const QString &value);

    void setAlias(const QString &alias);
    QString getAlias() const;

    void setTypeStr(const QString &type);
    QString getTypeStr() const;

    void setType(const JigInterfaceType &type);
    JigInterfaceType getType() const;

    virtual int open() = 0;
    virtual void close(void) = 0;
    virtual void config() = 0;
    virtual bool isOpen(void) = 0;
    virtual int getResult() = 0;
    virtual void reset(void) = 0;

    bool isDone(void);

    virtual qint64 write(const QByteArray &data) = 0;
    bool dataReady();

    QByteArray rxBuffer;

signals:

protected:
    bool dataOk;
    bool isAnswerRequired;
    QStringList arguments;
    QStringList answers;
    QString expectedAnswers;
    bool done;

public slots:

private:
    QString alias;
    JigInterfaceType type;
};

#endif // JIGABSTRACTINTERFACE_H
