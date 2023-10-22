#ifndef WEBSERVERTHREAD_H
#define WEBSERVERTHREAD_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>

#include <QDebug>

#include <QByteArray>
#include <QTextStream>
#include <QDataStream>
#include <QFile>

#include <QVector>
#include <QStringList>

class webserverthread : public QThread
{
    Q_OBJECT

    public:
        explicit webserverthread(qintptr ID, QObject *parent = nullptr);
        void run();
        void setServerBasePath(QString path);
        void setSSLUsage(bool option);
        void setCerFile(QString location);
        void setKeyFile(QString location);

    signals:
        void error(QTcpSocket::SocketError socketerror);
        void socketError(QAbstractSocket::SocketError error);
        void sendSFDLUploads(QVector<QStringList> sfdlUploads);
        void clientDisconnected();

    public slots:
        void readyRead();
        void readyReadSSL();
        void disconnected();

    private slots:
        void socketError2(QAbstractSocket::SocketError error);
        QByteArray returnFileData(QString filename);
        QString mimeReturn(const QFile& file);
        void isencrypted();
        void readLineFromSocket(QIODevice* device);
        void processData(QString socketData);
        QByteArray templateReplace(QByteArray data);

    private:
        QTcpSocket *socket;
        QSslSocket *sslsocket;
        qintptr socketDescriptor;
        QSslCertificate m_sslLocalCertificate;
        QSslKey m_sslPrivateKey;
        QString serverBasePath;
        bool sslUsage;
        QString sslCertFile;
        QString sslKeyFile;
};

#endif // WEBSERVERTHREAD_H
