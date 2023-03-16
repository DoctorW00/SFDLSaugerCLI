#include "webserver.h"

#include <QFile>
#include <QTextStream>
#include <QStandardPaths>

#ifdef QT_DEBUG
    #include <QDebug>
#endif

#ifdef Q_OS_WIN
    #include <windows.h>
#endif //Q_OS_WIN

#ifdef Q_OS_DARWIN
    #include <CoreFoundation/CFBundle.h>
    #include <ApplicationServices/ApplicationServices.h>
#endif // Q_OS_DARWIN

webserver::webserver(QObject *parent) : QTcpServer(parent)
{

}

void webserver::startServer(qint16 port)
{
    #ifdef QT_DEBUG
        qDebug() << "webserver::startServer port: " << port;
    #endif

    if(!port && port < 65535)
    {
        port = 8869;
    }

    if(this->listen(QHostAddress::Any, port))
    // if(this->listen(QHostAddress("127.0.0.1"), port))
    {
        qDebug() << "WWW listening @: " << this->serverAddress().toString() << ":" << port;
        openURL(port);
    }
    else
    {
        qDebug() << "WWW Error: can't start server @: " << this->serverAddress().toString() << ":" << port << " (" << this->errorString() << ")";
    }
}

void webserver::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "WWW client connecting: " << socketDescriptor;

    auto thread = new webserverthread(socketDescriptor, this);
    // auto thread = new webserverthread(sslSocket->socketDescriptor(), this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(error(QTcpSocket::SocketError)), this, SLOT(serverError(QTcpSocket::SocketError)));
    connect(thread, SIGNAL(socketError(QAbstractSocket::SocketError)), this, SLOT(serverError2(QAbstractSocket::SocketError)));

    qRegisterMetaType<QVector<QStringList>>("QVector<QStringList>");
    connect(thread, SIGNAL(sendSFDLUploads(QVector<QStringList>)), this, SLOT(handleSFDLUploads(QVector<QStringList>)));

    thread->start();
}

void webserver::serverError(QTcpSocket::SocketError error)
{
    qDebug() << "WWW: QTcpSocket error: " << error;
}

void webserver::serverError2(QAbstractSocket::SocketError error)
{
    qDebug() << "WWW: QAbstractSocket error: " << error;
}

void webserver::sslErrors(const QList<QSslError> &errors)
{
    foreach(const QSslError &error, errors)
    {
        qDebug() << "sslErrors: " << error.errorString();
    }
}

// save uploaded sfdl files to local file(s)
void webserver::handleSFDLUploads(QVector<QStringList> sfdlUploads)
{
    // get user temp path for sfdl files
    QString tmpPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if(!tmpPath.endsWith("/") && !tmpPath.endsWith("\\"))
    {
        tmpPath.append("/");
    }

    foreach(QStringList list, sfdlUploads)
    {
        QFile f(tmpPath + list.at(0));
        if(f.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&f);
            stream.setCodec("UTF-8");
            stream.setGenerateByteOrderMark(true);
            stream << list.at(1);
            stream.flush();

            qDebug() << "WWW: Successful created: " << tmpPath + list.at(0);

            f.close();

            // send new sfdl file to main thread
            sendSFDLFile(tmpPath + list.at(0));
        }
    }
}

void webserver::openURL(qint16 port)
{
    QString URL = "http://127.0.0.1:" + QString::number(port) + "/";

    // open url in browser (if windows)
    #ifdef Q_OS_WIN
        LPCWSTR openurl = (const wchar_t*) URL.utf16();

        QString OPEN = "open";
        LPCWSTR opennow = (const wchar_t*) OPEN.utf16();

        ShellExecute(NULL, opennow, openurl, NULL, NULL, SW_SHOWNORMAL);
    #endif //Q_OS_WIN

    // open url in browser (if mac os)
    #ifdef Q_OS_DARWIN
        CFURLRef url = CFURLCreateWithBytes (
              NULL,
              (UInt8*)URL.toStdString().c_str(),
              URL.length(),
              kCFStringEncodingASCII,
              NULL
            );
          LSOpenCFURLRef(url,0);
          CFRelease(url);
    #endif // Q_OS_DARWIN
}

