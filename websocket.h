#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <QObject>
#include <QWebSocketServer>
#include <QtWebSockets>
#include <QByteArray>
#include <QList>
#include <QTimer>

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

private slots:
    void newConnection();
    void textMessageReceived(QString message);
    void binaryMessageReceived(QByteArray message);
    void bytesWritten(qint64 bytes);
    void serverError(QAbstractSocket::SocketError error);
    void disconnected();

private:
    QWebSocketServer *server;
    QWebSocket *socket;
    QWebSocket *client;
    QList<QWebSocket *> _clients;
    QTimer *updateClients;

};

#endif // WEBSOCKET_H
