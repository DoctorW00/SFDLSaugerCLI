#ifndef WEBSERVER_H
#define WEBSERVER_H

// #include <QObject>

/*
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QTextStream>
#include <QDataStream>
#include <QFile>

class webserver : public QObject
{
    Q_OBJECT

public:
    explicit webserver(QObject *parent = nullptr);
    ~webserver();

public slots:
    void start(qint16 port);

private slots:
    void newConnection();
    void serverError(QAbstractSocket::SocketError error);
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();
    QByteArray returnFileData(QString filename);
    QString mimeReturn(const QFile& file);
    void openURL(qint16 port);

private:
    QTcpServer *server;
    QTcpSocket *socket;

};

*/

#include <QTcpServer>
#include "webserverthread.h"

#include <QSslSocket>
#include <QSslKey>
#include <QSslError>

class webserver : public QTcpServer
{
    Q_OBJECT

    public:
        explicit webserver(QObject *parent = 0);
        void startServer(qint16 port);

    private slots:
        void openURL(qint16 port); // open in default browser (win/mac)
        void serverError(QTcpSocket::SocketError error);
        void serverError2(QAbstractSocket::SocketError error);
        void sslErrors(const QList<QSslError> &errors);

    protected:
        void incomingConnection(qintptr socketDescriptor);


};

#endif // WEBSERVER_H
