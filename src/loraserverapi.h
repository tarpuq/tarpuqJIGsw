#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QtNetwork>

class LoraServerApi : public QObject
{
    Q_OBJECT
public:
    explicit LoraServerApi(QObject *parent = nullptr);

    enum lastRequest { none, loginReq, addReq, removeReq, keysReq, queryReq };

public:
    void login(QString host, quint16 port, QString user, QString password);

    void listDevice(QString devEUI);
    void createDevice(QString devEUI);
    void deleteDevice(QString devEUI);
    void setKeys(QString devEUI, QString appKey, QString nwkKey);

    void setApplicationId(QString id) { this->applicationId = id; }
    QString getApplicationId(void) { return this->applicationId; }

    void setDeviceProfileId(QString id) { this->deviceProfileId = id; }
    QString getDeviceProfileId(void) { return this->deviceProfileId; }

private slots:
    void processReply(void);

private:
    QNetworkAccessManager *networkAccess;

    lastRequest last;

    QString applicationId;
    QString deviceProfileId;

    QTimer *reconnectTimer;
    QString apiHostPort, apiHost, apiUser, apiPswd;
    QString ultTipoPost, ultEUI, ultAcc;
    QString apiJwt; //, euiResp;
    quint16 apiPort;
    qint32 reconnectTimerInterval, tOutApi;
    bool bdCntd;
    bool isApiConnected;
    QString IntEuiToStringEui(quint64 eui);
    quint64 StringEuiToIntEui(QString eui);
    QUrl url;

    QNetworkRequest request;
};

#endif // DEVICE_H
