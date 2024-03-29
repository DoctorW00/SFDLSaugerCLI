#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QTcpServer>
#include "webserverthread.h"

#include <QSslSocket>
#include <QSslKey>
#include <QSslError>
#include <QSslCertificate>

class webserver : public QTcpServer
{
    Q_OBJECT

public:
    explicit webserver(QObject *parent = 0);
    void startServer(qint16 port);
    void setHostIP(QString ip);
    void setGUIpath(QString path);
    void setSSLUsage(bool option);
    void setCerFile(QString location);
    void setKeyFile(QString location);

private slots:
    void openURL(qint16 port); // open in default browser (win/mac)
    void serverError(QTcpSocket::SocketError error);
    void serverError2(QAbstractSocket::SocketError error);
    void sslErrors(const QList<QSslError> &errors);
    void handleSFDLUploads(QVector<QStringList> sfdlUploads);

protected:
    void incomingConnection(qintptr socketDescriptor);

signals:
    void sendSFDLFile(QString sfdlFile);

private:
    QString hostIP;
    QString guiPath;
    bool sslUsage;
    QString sslCertFile;
    QString sslKeyFile;

};

#endif // WEBSERVER_H
