#ifndef JIGINTERFACE_H
#define JIGINTERFACE_H

#include <QHash>
#include <QIODevice>
#include <QObject>
#include <QSerialPort>
#include <QVariant>

class JigInterface : public QObject
{
    Q_OBJECT
public:
    explicit JigInterface(QObject *parent = nullptr);

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

    virtual int open();
    virtual void close(void);
    virtual void config();
    virtual bool isOpen(void);
    virtual bool isDone(void);
    virtual int getResult();
    virtual QStringList getAnswers();
    virtual void reset(void);

    virtual qint64 write(const QByteArray &data);
    bool dataReady();

    QByteArray rxBuffer;

signals:

protected:
    bool dataOk;
    bool isAnswerRequired;
    QStringList arguments;
    QStringList answers;
    QString expectedAnswers;

public slots:

private:
    QString alias;
    JigInterfaceType type;
};

#endif // JIGINTERFACE_H
