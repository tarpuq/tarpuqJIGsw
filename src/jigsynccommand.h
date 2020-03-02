#ifndef JIGCOMMAND_H
#define JIGCOMMAND_H

#include <QObject>

#include <QTreeWidgetItem>

class JigSyncCommand : public QObject
{
    Q_OBJECT
public:
    explicit JigSyncCommand(QObject *parent = nullptr);
    JigSyncCommand(QObject *parent, const JigSyncCommand &other);
    JigSyncCommand &operator=(const JigSyncCommand &other);

    enum JigCommandState { pending, delay, running, jump, ok, fail };
    Q_ENUM(JigCommandState)

    enum JigCommandColumn { colTest, colMin, colMeasure, colMax, colState };
    Q_ENUM(JigCommandColumn)

    QString messageOnOk;
    QString messageOnError;
    QString messageNotice;

    QTreeWidgetItem *treeItem;

    double deviation;
    double mean;
    double offset;
    double max;
    double min;
    bool retry;
    bool end;
    bool runOnJump;

    bool isCr;
    bool isLf;
    bool isCrc16;
    bool isOptional;

    QString getInterfaceName() const;
    void setInterfaceName(const QString &value);

    QString getInterfaceCommand() const;
    void setInterfaceCommand(const QString &value);

    QString getInterfaceArguments() const;
    void setInterfaceArguments(const QString &value);

    QString getInterfaceAnswer() const;
    void setInterfaceAnswer(const QString &value);

    QString getTxCommand() const;
    void setTxCommand(const QString &value);

    QString getTxArguments() const;
    void setTxArguments(const QString &value);

    QString getRxCommand() const;
    void setRxCommand(const QString &value);

    QString getRxAnswers() const;
    void setRxAnswers(const QString &value);

    QString getOnOk() const;
    void setOnOk(const QString &value);

    QString getOnError() const;
    void setOnError(const QString &value);

    JigCommandState getState() const;
    void setStatus(const JigCommandState &value);

    bool getEnabled() const;
    void setEnabled(bool value);

    QString getId() const;
    void setId(const QString &value);

    QString getName() const;
    void setName(const QString &value);

    QString getDescription() const;
    void setDescription(const QString &value);

    int getTimeOut() const;
    void setTimeOut(int value);

    int getTimeDelay() const;
    void setTimeDelay(int value);

    bool getIsCr() const;
    void setIsCr(bool value);

    bool getIsLf() const;
    void setIsLf(bool value);

    bool getIsCrc16() const;
    void setIsCrc16(bool value);

    bool getIsOptional() const;
    void setIsOptional(bool value);

    bool getUseMeanFormula() const;
    void setUseMeanFormula(bool value);

    QString getMeanFormula() const;
    void setMeanFormula(const QString &value);

    QString getMeasFormula() const;
    void setMeasFormula(const QString &value);

    void refreshState();
    void setMeasureError(QString value);

private:
    QString id;
    QString name;
    QString description;

    QString ifName;
    QString ifCommand;
    QString ifArguments;
    QString ifAnswer;

    QString txCommand;
    QString txArguments;
    QString rxCommand;
    QString rxAnswers;

    QString onOk;
    QString onError;

    QString measMeanFormula;
    bool useMeanFormula;
    QString measFormula;

    bool enabled;

    int timeOut;
    int timeDelay;

    JigCommandState state;

public slots:
    bool isTxCommand(void);
    bool isRxCommand(void);
    bool isTxArguments(void);
    bool isRxAnswers(void);
    void setMeasureRange(double colMin, double colMax);
    void setMeasureParameters(double mean, double deviation, double offset);
    int processMeasure(QTreeWidgetItem *item, QString value);
    int setMeasureItem(QTreeWidgetItem *item, double value);
    bool compareDate(QString date);
    void setStatusItem(QTreeWidgetItem *item, JigCommandState status);
    void clearMeasure();
    bool isOnOk(void);
    bool isOnError(void);

    int processAnswers(QStringList answers, QHash<QString, QByteArray> *wildcard, int index, QList<bool> *flags, QStringList *report);
};

#endif // JIGCOMMAND_H
