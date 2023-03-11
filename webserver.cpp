#include "webserver.h"

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

    /*
    QSslSocket *sslSocket = new QSslSocket(this);
    connect(sslSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
    connect(sslSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(serverError2(QAbstractSocket::SocketError)));

    if(!sslSocket->setSocketDescriptor(socketDescriptor))
    {
        qWarning("WWW Error: could not set socket descriptor!");
        sslSocket->deleteLater();
        exit(2);
    }

    QByteArray key;
    QByteArray cert;

    QFile fileKey(":/cert/red_local.key");
    if(fileKey.open(QIODevice::ReadOnly))
    {
        key = fileKey.readAll();
        fileKey.close();
    }
    else
    {
        qDebug() << fileKey.errorString();
    }

    QFile fileCert(":/cert/red_local.pem");
    if(fileCert.open(QIODevice::ReadOnly))
    {
        cert = fileCert.readAll();
        fileCert.close();
    }
    else
    {
        qDebug() << fileCert.errorString();
    }

    // QByteArray pasp ="1234567890";
    // QSslKey sslKey(key,QSsl::Rsa,QSsl::Pem,QSsl::PrivateKey,pasp);
    QSslKey sslKey(key,QSsl::Rsa);
    QSslCertificate sslCert(cert);

    sslSocket->setPrivateKey(sslKey);
    sslSocket->setLocalCertificate(sslCert);
    // sslSocket->addCaCertificates(QSslCertificate::fromPath(":/cert/blue_ca.pem"));
    sslSocket->addCaCertificates(":/cert/blue_ca.pem");
    sslSocket->setPeerVerifyMode(QSslSocket::VerifyNone);
    sslSocket->setProtocol(QSsl::TlsV1_2);

    QList<QSslCertificate> cert2 = QSslCertificate::fromPath(":/cert/red_local.pem");
    QSslError error2(QSslError::SelfSignedCertificate, cert2.at(0));

    QList<QSslCertificate> cert3 = QSslCertificate::fromPath(":/cert/blue_ca.pem");
    QSslError error3(QSslError::SelfSignedCertificate, cert3.at(0));

    QList<QSslError> expectedSslErrors;
    expectedSslErrors.append(error2);
    expectedSslErrors.append(error3);
    sslSocket->ignoreSslErrors(expectedSslErrors);

    sslSocket->startServerEncryption();
    // addPendingConnection(sslSocket);

    if(sslSocket->waitForEncrypted(3000))
    {
        #ifdef QT_DEBUG
            qDebug() << "WWW Socket is now SSL/TLS encrypted!";
        #endif

        addPendingConnection(sslSocket);
    }
    else
    {
        #ifdef QT_DEBUG
            qDebug() << "WWW Error: wait for encrypted failed!";
        #endif

        sslSocket->deleteLater();
        exit(2);
    }
    */

    auto thread = new webserverthread(socketDescriptor, this);
    // auto thread = new webserverthread(sslSocket->socketDescriptor(), this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(error(QTcpSocket::SocketError)), this, SLOT(serverError(QTcpSocket::SocketError)));
    connect(thread, SIGNAL(socketError(QAbstractSocket::SocketError)), this, SLOT(serverError2(QAbstractSocket::SocketError)));

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

/*
void webserver::incomingConnection(qintptr socketDescriptor)
{
    // We have a new connection
    qDebug() << socketDescriptor << " Connecting...";

    webserverthread *thread = new webserverthread(socketDescriptor, this);

    // connect signal/slot
    // once a thread is not needed, it will be beleted later
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
}
*/

/*
void webserver::startServer()
{
    int port = 8869;

    if(!this->listen(QHostAddress::Any, port))
    {
        qDebug() << "WWW started @: " << this->serverAddress().toString() << ":" << port;
    }
    else
    {
        qDebug() << "WWW Error: can't start server @: " << this->serverAddress().toString() << ":" << port << this->errorString() << this->serverError();
    }
}
*/


/*
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

#ifdef Q_OS_WIN
    #include <windows.h>
#endif //Q_OS_WIN

#ifdef Q_OS_DARWIN
    #include <CoreFoundation/CFBundle.h>
    #include <ApplicationServices/ApplicationServices.h>
#endif // Q_OS_DARWIN

webserver::webserver(QObject *parent) : QObject(parent){}

webserver::~webserver()
{
    server->close();
    delete server;
    delete socket;
}

void webserver::start(qint16 port)
{
    if(!port)
    {
        port = 8869;
    }

    server = new QTcpServer;

    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    connect(server, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(serverError(QAbstractSocket::SocketError)));
    connect(server, SIGNAL(acceptError(QAbstractSocket::SocketError)), server, SLOT(deleteLater()));

    if(server->listen(QHostAddress::Any, port))
    {
        qDebug() << "Webserver started @ port: " << server->serverPort();

        // open url in browser
        openURL(server->serverPort());
    }
    else
    {
        qDebug() << "Error: Webserver did not start: " << server->errorString();
    }
}

void webserver::newConnection()
{
    socket = server->nextPendingConnection();

    qDebug() << "Webserver new connection from " << socket->peerAddress() << socket->peerPort();

    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(serverError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), socket, SLOT(deleteLater()));

    socket->write("HTTP/1.0 200 Ok\r\n");
    socket->waitForBytesWritten();

    socket->close();
}

void webserver::serverError(QAbstractSocket::SocketError error)
{
    qDebug() << "WWW: Socket error: " << error;
}

void webserver::disconnected()
{
    qDebug() << "disconnected!";
    qDebug() << "==============================================================";
}

void webserver::bytesWritten(qint64 bytes)
{
    qDebug() << bytes << " bytes written!";
}

void webserver::readyRead()
{
    qDebug() << "reading: ";

    if(socket->canReadLine())
    {
        QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));

        qDebug() << "tokens: " << tokens;

        if(tokens[0] == "GET")
        {
            qDebug() << "tokens[0]: " << tokens[0];

            if(tokens[1] == "/")
            {
                QString loadFile(":/www/index.html");
                QString mType = mimeReturn(loadFile);

                socket->write(mType.toUtf8());
                socket->write(returnFileData(loadFile));
            }
            else
            {
                QString loadFile = tokens[1];

                if(loadFile.startsWith("/:/"))
                {
                    loadFile.remove(0,1);
                }
                else
                {
                    loadFile = ":/www" + loadFile;
                }

                QFile chkFile(loadFile);
                if(!chkFile.exists())
                {
                    loadFile = ":/www/404.html";
                }

                QString mType = mimeReturn(loadFile);

                socket->write(mType.toUtf8());
                socket->write(returnFileData(loadFile));
            }
        }
    }
}

QByteArray webserver::returnFileData(QString filename)
{
    QFile file(filename);

    if(!file.open(QFile::ReadOnly))
    {
        qDebug() << "Can't open file for reading!";
        return QByteArray();
    }

    QByteArray textOut = file.readAll();

    file.close();

    return textOut;
}

QString webserver::mimeReturn(const QFile& file)
{
    QMimeDatabase mimeDatabase;
    QMimeType mimeType;

    mimeType = mimeDatabase.mimeTypeForFile(QFileInfo(file));

    if(QFileInfo(file).absoluteFilePath() == ":/www/404.html")
    {
        return "HTTP/1.0 404 Not Found\r\nContent-Type: text/html; charset=\"utf-8\"\r\n\r\n";
    }

    if(mimeType.inherits("text/html"))
    {
        return "HTTP/1.0 200 Ok\r\nContent-Type: text/html; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("text/plain"))
    {
        return "HTTP/1.0 200 Ok\r\nContent-Type: text/plain; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("application/xhtml+xml"))
    {
        return "HTTP/1.0 200 Ok\r\nContent-Type: application/xhtml+xml; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("application/xml"))
    {
        return "HTTP/1.0 200 Ok\r\nContent-Type: application/xml; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("text/xml"))
    {
        return "HTTP/1.0 200 Ok\r\nContent-Type: text/xml; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("text/css"))
    {
        return "HTTP/1.0 200 Ok\r\nContent-Type: text/css; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("image/gif"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        return "HTTP/1.0 200 Ok\r\nContent-Type: image/gif; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/vnd.microsoft.icon"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        return "HTTP/1.0 200 Ok\r\nContent-Type: image/vnd.microsoft.icon; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/jpeg"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        return "HTTP/1.0 200 Ok\r\nContent-Type: image/jpeg; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/png"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        return "HTTP/1.0 200 Ok\r\nContent-Type: image/png; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/svg+xml"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        return "HTTP/1.0 200 Ok\r\nContent-Type: image/svg+xml; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/tiff"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        return "HTTP/1.0 200 Ok\r\nContent-Type: image/tiff; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/webp"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        return "HTTP/1.0 200 Ok\r\nContent-Type: image/webp; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("text/javascript"))
    {
        return "HTTP/1.0 200 Ok\r\nContent-Type: text/javascript; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("application/json"))
    {
        return "HTTP/1.0 200 Ok\r\nContent-Type: application/json; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("font/otf"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        return "HTTP/1.0 200 Ok\r\nContent-Type: font/otf; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("font/ttf"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        return "HTTP/1.0 200 Ok\r\nContent-Type: font/ttf; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("font/woff"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        return "HTTP/1.0 200 Ok\r\nContent-Type: font/woff; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("font/woff2"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        return "HTTP/1.0 200 Ok\r\nContent-Type: font/woff2; Content-Length: " + fileSize + "\r\n\r\n";
    }

    return QString();
}

void webserver::openURL(qint16 port)
{
    QString URL = "http://localhost:" + QString::number(port) + "/";

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
*/
