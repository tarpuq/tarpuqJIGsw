#include "loraserverapi.h"

#include <algorithm>
#include <QtEndian>

LoraServerApi::LoraServerApi(QObject *parent)
    : QObject(parent)
{
    networkAccess = new QNetworkAccessManager(this);

    reconnectTimer = new QTimer;
    connect(reconnectTimer, &QTimer::timeout, [this]() {
        reconnectTimer->stop();
        login(apiHost, apiPort, apiUser, apiPswd);
    });

    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    reconnectTimerInterval = (12) * 60 * 60 * 1000;
}

QString LoraServerApi::IntEuiToStringEui(quint64 eui)
{
    QString euiS, temp, aux;
    int tmp, s;
    temp = temp.number(eui, 16);
    s = 16 - temp.length();
    if (s > 0) {
        aux.clear();
        for (int d = 0; d < s; d++)
            aux.append('0');
    }
    temp = aux + temp;
    for (tmp = temp.length(); tmp > 0; tmp -= 2) //dar la vuelta al id
        euiS += temp.mid(tmp - 2, 2);
    return euiS;
}

quint64 LoraServerApi::StringEuiToIntEui(QString eui)
{
    QString aux;
    int tmp;
    bool st;
    aux.clear();
    for (tmp = eui.length(); tmp > 0; tmp -= 2) //dar la vuelta al id
        aux += eui.mid(tmp - 2, 2);
    return aux.toULongLong(&st, 16);
}

void LoraServerApi::login(QString host, quint16 port, QString user, QString password)
{
    QStringList header;
    QStringList content;
    QJsonObject jsonObject;

    apiHost = host;
    apiPort = port;
    apiUser = user;
    apiPswd = password;

    //  Static URL data
    url.setScheme("https");
    url.setPort(port);
    url.setHost(host);

    //  Dynamic URL data
    url.setPath("/api/internal/login");
    url.setQuery("");

    //  Request data
    header.append("Content-Type");
    header.append("Accept");

    content.append("application/json");
    content.append("application/json");

    jsonObject.insert("username", user);
    jsonObject.insert("password", password);

    request.setUrl(url);

    for (int i = 0; i < header.length(); i++)
        request.setRawHeader(header.at(i).toLatin1(), content.at(i).toLatin1());

    last = loginReq;

    auto *reply = networkAccess->post(request, QJsonDocument(jsonObject).toJson());

    if (reply) {
        if (!reply->isFinished())
            connect(reply, &QNetworkReply::finished, this, &LoraServerApi::processReply);
        else
            delete reply; // broadcast replies return immediately
    } else {
    }

    reconnectTimer->start(reconnectTimerInterval);
}

void LoraServerApi::listDevice(QString devEUI)
{
    QStringList header;
    QStringList content;

    qulonglong eui = devEUI.toULongLong();
    ultEUI = IntEuiToStringEui(eui);

    url.setPath("/api/devices");

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("limit", "1");
    urlQuery.addQueryItem("search", ultEUI);

    url.setQuery(urlQuery);

    header.append("Content-Type");
    header.append("Grpc-Metadata-Authorization");

    content.append("application/json");
    content.append(apiJwt.toUtf8());

    request.setUrl(url);

    for (int i = 0; i < header.length(); i++)
        request.setRawHeader(header.at(i).toLatin1(), content.at(i).toLatin1());

    qDebug() << request.rawHeaderList();

    last = queryReq;

    auto *reply = networkAccess->get(request);

    if (reply) {
        if (!reply->isFinished())
            connect(reply, &QNetworkReply::finished, this, &LoraServerApi::processReply);
        else
            delete reply; // broadcast replies return immediately
    } else {
    }
}

void LoraServerApi::createDevice(QString devEUI)
{
    QStringList header;
    QStringList content;
    QJsonObject deviceObject;
    QJsonObject jsonObject;

    qulonglong eui = devEUI.toULongLong();
    ultEUI = IntEuiToStringEui(eui);

    QString name = "medidor" + QString::number(eui - 11637100000);

    url.setPath("/api/devices");
    url.setQuery("");

    header.append("Content-Type");
    header.append("Grpc-Metadata-Authorization");

    content.append("application/json");
    content.append(apiJwt.toUtf8());

    deviceObject.insert("applicationID", QJsonValue::fromVariant(applicationId));
    deviceObject.insert("description", QJsonValue::fromVariant(name));
    deviceObject.insert("devEUI", QJsonValue::fromVariant(ultEUI));
    deviceObject.insert("deviceProfileID", QJsonValue::fromVariant(deviceProfileId));
    deviceObject.insert("name", QJsonValue::fromVariant(name));
    deviceObject.insert("referenceAltitude", 0);
    deviceObject.insert("skipFCntCheck", QJsonValue::fromVariant(false));

    jsonObject.insert("device", deviceObject);

    request.setUrl(url);

    for (int i = 0; i < header.length(); i++)
        request.setRawHeader(header.at(i).toLatin1(), content.at(i).toLatin1());

    last = addReq;

    auto *reply = networkAccess->post(request, QJsonDocument(jsonObject).toJson());

    if (reply) {
        if (!reply->isFinished())
            connect(reply, &QNetworkReply::finished, this, &LoraServerApi::processReply);
        else
            delete reply; // broadcast replies return immediately
    } else {
    }
}

void LoraServerApi::setKeys(QString devEUI, QString appKey, QString nwkKey)
{
    QStringList header;
    QStringList content;
    QJsonObject deviceKeysObject;
    QJsonObject jsonObject;

    qulonglong eui = devEUI.toULongLong();
    ultEUI = IntEuiToStringEui(eui);

    url.setPath("/api/devices/" + ultEUI + "/keys");
    url.setQuery("");

    header.append("Content-Type");
    header.append("Grpc-Metadata-Authorization");

    content.append("application/json");
    content.append(apiJwt.toUtf8());

    deviceKeysObject.insert("appKey", QJsonValue::fromVariant(appKey));
    deviceKeysObject.insert("devEUI", QJsonValue::fromVariant(ultEUI));
    deviceKeysObject.insert("nwkKey", QJsonValue::fromVariant(nwkKey));

    jsonObject.insert("deviceKeys", deviceKeysObject);

    request.setUrl(url);

    for (int i = 0; i < header.length(); i++)
        request.setRawHeader(header.at(i).toLatin1(), content.at(i).toLatin1());

    last = keysReq;

    auto *reply = networkAccess->post(request, QJsonDocument(jsonObject).toJson());

    if (reply) {
        if (!reply->isFinished())
            connect(reply, &QNetworkReply::finished, this, &LoraServerApi::processReply);
        else
            delete reply; // broadcast replies return immediately
    } else {
    }
}

void LoraServerApi::deleteDevice(QString devEUI)
{
    QStringList header;
    QStringList content;

    qulonglong eui = devEUI.toULongLong();
    ultEUI = IntEuiToStringEui(eui);

    url.setPath("/api/devices/" + ultEUI);
    url.setQuery("");

    header.append("Content-Type");
    header.append("Grpc-Metadata-Authorization");

    content.append("application/json");
    content.append(apiJwt.toUtf8());

    request.setUrl(url);

    for (int i = 0; i < header.length(); i++)
        request.setRawHeader(header.at(i).toLatin1(), content.at(i).toLatin1());

    last = removeReq;

    auto *reply = networkAccess->deleteResource(request);

    if (reply) {
        if (!reply->isFinished())
            connect(reply, &QNetworkReply::finished, this, &LoraServerApi::processReply);
        else
            delete reply; // broadcast replies return immediately
    } else {
    }
}

void LoraServerApi::processReply(void)
{
    auto reply = qobject_cast<QNetworkReply *>(sender());

    QByteArray data = reply->readAll();

    qDebug() << data;

    QJsonDocument respJson;
    QJsonObject objJson;

    respJson = QJsonDocument::fromJson(data);
    objJson = respJson.object();

    if (data.contains("authentication failed")) {
        isApiConnected = false;
    }

    switch (last) {
    case none:

        break;
    case loginReq:
        if (data.length() > 0) {
            apiJwt = objJson["jwt"].toString();
            qDebug() << "Logueado api";
            isApiConnected = true;
        } else {
            qDebug() << "No logueado api";
            reconnectTimer->start(10000); /************ reconexion *************/
        }
        break;
    case addReq:
        qDebug() << ("agregado contador: " + QString::number(objJson["fCnt"].toInt()));
        break;
    case removeReq:
        //emit MensajeRecibido("Contador: " + QString::number(objJson["fCnt"].toInt()));
        qDebug() << ("quitado contador: " + QString::number(objJson["fCnt"].toInt()));
        break;
    case keysReq:
        break;
    case queryReq:
        qDebug() << ("consulta ontador: " + QString::number(objJson["fCnt"].toInt()));
        break;
    }

    last = none;

    reply->deleteLater();
}
