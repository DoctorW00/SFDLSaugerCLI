#ifndef WEBSERVERTHREAD_H
#define WEBSERVERTHREAD_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>

#ifdef QT_DEBUG
    #include <QDebug>
#endif

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

    signals:
        void error(QTcpSocket::SocketError socketerror);
        void socketError(QAbstractSocket::SocketError error);
        void sendSFDLUploads(QVector<QStringList> sfdlUploads);
        void clientDisconnected();

    public slots:
        void readyRead();
        void disconnected();

    private slots:
        void socketError2(QAbstractSocket::SocketError error);
        QByteArray returnFileData(QString filename);
        QString mimeReturn(const QFile& file);
        void isencrypted();

    private:
        // QTcpSocket *socket;
        QSslSocket *socket;
        qintptr socketDescriptor;
        QSslCertificate m_sslLocalCertificate;
        QSslKey m_sslPrivateKey;
};

#endif // WEBSERVERTHREAD_H
