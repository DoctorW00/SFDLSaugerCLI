#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <QObject>
#include <QWebSocketServer>
#include <QtWebSockets>
#include <QByteArray>
#include <QList>
#include <QTimer>

#include <QtWebSockets/QWebSocket>
#include <QMap>

#include "jsonwebtoken.h"
#include "global.h"

extern QVector<dServer> Servers;
extern QVector<dFile> Files;
extern QMap<QWebSocket *, dConnectionInfo> connections;

class websocket : public QObject
{
    Q_OBJECT

public:
    explicit websocket(QObject *parent = nullptr);
    ~websocket();

public slots:
    void start();
    void start(qint16 port);
    void sendTextMessage();
    void sendTest();
    void setHostIP(QString ip);
    void setSSLUsage(bool option);
    void setCerFile(QString location);
    void setKeyFile(QString location);

private slots:
    void newConnection();
    void textMessageReceived(QString message);
    void binaryMessageReceived(QByteArray message);
    void bytesWritten(qint64 bytes);
    void serverError(QAbstractSocket::SocketError error);
    void disconnected();
    bool isValidJson(const QString& jsonString);
    QString doLogin(QString username);
    void doLogOut(QString token);

private:
    QString hostIP;
    QWebSocketServer *server;
    QWebSocket *socket;
    QWebSocket *client;
    QList<QWebSocket *> _clients;
    QTimer *updateClients;
    QSslCertificate m_sslLocalCertificate;
    QSslKey m_sslPrivateKey;
    bool sslUsage;
    QString sslCertFile;
    QString sslKeyFile;
    JsonWebToken *JWT;
};

#endif // WEBSOCKET_H
