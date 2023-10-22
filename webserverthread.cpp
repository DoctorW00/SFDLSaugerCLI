#include "webserverthread.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

webserverthread::webserverthread(qintptr ID, QObject *parent) : QThread(parent)
{
    this->socketDescriptor = ID;
}

void webserverthread::setServerBasePath(QString path)
{
    serverBasePath = path;

    if(!serverBasePath.endsWith("/"))
    {
        serverBasePath.append("/");
    }
}

void webserverthread::setSSLUsage(bool option)
{
    sslUsage = option;
}

void webserverthread::setCerFile(QString location)
{
    sslCertFile = location;
}

void webserverthread::setKeyFile(QString location)
{
    sslKeyFile = location;
}

void webserverthread::run()
{
    socketDescriptor = this->socketDescriptor;

    #ifdef QT_DEBUG
        qDebug() << "WWW thread started: " << QThread::currentThreadId() << socketDescriptor;
        qDebug() << "WWW thread ssl usage: " << sslUsage;
        qDebug() << "WWW thread server base path: " << serverBasePath;
    #endif

    // socket = new QTcpSocket();
    // socket = new QSslSocket;

    if(sslUsage == true)
    {
        if(sslCertFile.isEmpty())
        {
            sslCertFile = ":/cert/localhost.crt";
        }

        if(sslKeyFile.isEmpty())
        {
            sslKeyFile = ":/cert/localhost.decrypted.key";
        }

        sslsocket = new QSslSocket;

        if(!sslsocket->setSocketDescriptor(this->socketDescriptor))
        {
            emit error(sslsocket->error());
            return;
        }

        connect(sslsocket, SIGNAL(readyRead()), this, SLOT(readyReadSSL()), Qt::DirectConnection);
        connect(sslsocket, SIGNAL(encrypted()), this, SLOT(isencrypted()));
        connect(sslsocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        connect(sslsocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError2(QAbstractSocket::SocketError)));

        // const auto certs = QSslCertificate::fromPath(QLatin1String(":/cert/localhost.crt"));
        const auto certs = QSslCertificate::fromPath(QLatin1String(sslCertFile.toLatin1()));

        #ifdef QT_DEBUG
            qDebug() << "WWW: Certs: " << certs << QThread::currentThreadId() << socketDescriptor;
        #endif

        if(certs.length())
        {
            m_sslLocalCertificate = certs.at(0);
        }
        sslsocket->setLocalCertificate(m_sslLocalCertificate);

        // QFile keyFile(":/cert/localhost.decrypted.key");
        QFile keyFile(sslKeyFile);
        if(keyFile.open(QIODevice::ReadOnly))
        {
            QString keyFileData = keyFile.readAll();
            QByteArray keydata = keyFileData.toUtf8();

            QSslKey key(keydata, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
            m_sslPrivateKey = key;

            keyFile.close();

            #ifdef QT_DEBUG
                qDebug() << "WWW: SSL Key: " << m_sslPrivateKey << QThread::currentThreadId() << socketDescriptor;
            #endif
        }
        else
        {
            qDebug() << "WWW: Can't open SSL KeyFile: " << keyFile.fileName() << QThread::currentThreadId() << socketDescriptor;
        }

        sslsocket->setPrivateKey(m_sslPrivateKey);
        sslsocket->setProtocol(QSsl::TlsV1_2);
        sslsocket->startServerEncryption();
    }
    else
    {
        socket = new QTcpSocket();

        if(!socket->setSocketDescriptor(this->socketDescriptor))
        {
            emit error(socket->error());
            return;
        }

        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
        connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError2(QAbstractSocket::SocketError)));
    }

    exec();
}

void webserverthread::socketError2(QAbstractSocket::SocketError error)
{
    #ifdef QT_DEBUG
        qDebug() << "WWW: Socket error: " << error << QThread::currentThreadId() << socketDescriptor;
    #endif
}

void webserverthread::isencrypted()
{
    #ifdef QT_DEBUG
        qDebug() << "WWW: Connection is now SSL encrypted: " << QThread::currentThreadId() << socketDescriptor;
    #endif
}

void webserverthread::readyRead()
{
    #ifdef QT_DEBUG
        qDebug() << "WWW thread ready to read (no ssl)! " << QThread::currentThreadId() << socketDescriptor;
    #endif

    readLineFromSocket(socket);
}

void webserverthread::readyReadSSL()
{
    #ifdef QT_DEBUG
        qDebug() << "WWW thread ready to read (ssl)! " << QThread::currentThreadId() << socketDescriptor;
    #endif

    readLineFromSocket(sslsocket);
}

void webserverthread::processData(QString socketData)
{
    QString wwwRootPath;

    if(!serverBasePath.isEmpty())
    {
        wwwRootPath = serverBasePath;
    }
    else
    {
        // wwwRootPath = ":/webui/build/";
        wwwRootPath = ":/www/";
    }

    #ifdef QT_DEBUG
        qDebug() << "WWW ReadLine RawData: " << socketData;
    #endif

    QStringList headerItems = QString(socketData).split(QRegExp("\r\n"));
    QStringList tokens = QString(headerItems[0]).split(QRegExp("[ \r\n][ \r\n]*"));

    if(tokens[0] == "GET")
    {
        if(tokens[1] == "/")
        {
            // QString loadFile(":/www/index.html");
            // QString loadFile(":/react/saugergui/build/index.html");
            // QString loadFile(":/webui/build/index.html");

            QString loadFile(wwwRootPath + "index.html");

            QString mType = mimeReturn(loadFile);

            if(sslUsage == true)
            {
                sslsocket->write(mType.toUtf8() + returnFileData(loadFile));
                sslsocket->waitForBytesWritten();
                sslsocket->close();
            }
            else
            {
                socket->write(mType.toUtf8() + returnFileData(loadFile));
                socket->waitForBytesWritten();
                socket->close();
            }
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
                // loadFile = ":/www" + loadFile;
                // loadFile = ":/react/saugergui/build" + loadFile;
                // loadFile = ":/webui/build" + loadFile;
                loadFile = wwwRootPath + loadFile;

                #ifdef QT_DEBUG
                    qDebug() << "WWW (GET) loadFile: " << loadFile << QThread::currentThreadId() << socketDescriptor;
                #endif
            }

            QFile chkFile(loadFile);
            if(!chkFile.exists())
            {
                // loadFile = ":/www/404.html";
                loadFile = wwwRootPath + "404.html";

                #ifdef QT_DEBUG
                    qDebug() << "WWW (GET) loadFile not found 4040: " << loadFile << QThread::currentThreadId() << socketDescriptor;
                #endif
            }

            QString mType = mimeReturn(loadFile);

            if(sslUsage == true)
            {
                sslsocket->write(mType.toUtf8() + returnFileData(loadFile));
                sslsocket->waitForBytesWritten();
                sslsocket->flush();
                sslsocket->close();
            }
            else
            {
                socket->write(mType.toUtf8() + returnFileData(loadFile));
                socket->waitForBytesWritten();
                socket->flush();
                socket->close();
            }
        }
    }

    if(tokens[0] == "POST")
    {
        #ifdef QT_DEBUG
            qDebug() << "WWW (POST) tokens[0]: " << tokens[0] << QThread::currentThreadId() << socketDescriptor;
        #endif

        QVector<QStringList> sfdlUploads;
        QStringList sfdlData;

        for(int i = 0; i < headerItems.count(); i++)
        {
            if(QString(headerItems[i]).startsWith("Content-Disposition"))
            {
                QRegularExpression re(".*filename=\"(.*)\"");
                QRegularExpressionMatch match = re.match(QString(headerItems[i]));

                if(match.hasMatch())
                {
                    sfdlData.append(match.captured(1));
                }
            }

            if(QString(headerItems[i]).startsWith("<?xml"))
            {
                sfdlData.append(QString(headerItems[i]));
            }

            if(sfdlData.count() == 2)
            {
                sfdlUploads.append(sfdlData);
                sfdlData.clear();
            }
        }

        if(sfdlUploads.length())
        {
            sendSFDLUploads(sfdlUploads);
        }


        QString loadFile = tokens[1];

        if(loadFile.startsWith("/:/"))
        {
            loadFile.remove(0,1);
        }
        else
        {
            // loadFile = ":/www" + loadFile;
            // loadFile = ":/react/saugergui/build" + loadFile;
            // loadFile = ":/webui/build" + loadFile;
            loadFile = wwwRootPath + loadFile;

            #ifdef QT_DEBUG
                qDebug() << "WWW (POST) loadFile: " << loadFile << QThread::currentThreadId() << socketDescriptor;
            #endif
        }

        QFile chkFile(loadFile);
        if(!chkFile.exists())
        {
            // loadFile = ":/www/404.html";
            loadFile = wwwRootPath + "404.html";

            #ifdef QT_DEBUG
                qDebug() << "WWW (POST) loadFile not found 4040: " << loadFile << QThread::currentThreadId() << socketDescriptor;
            #endif
        }

        QString mType = mimeReturn(loadFile);

        if(sslUsage == true)
        {
            sslsocket->write(mType.toUtf8() + returnFileData(loadFile));
            sslsocket->waitForBytesWritten();
            sslsocket->flush();
            sslsocket->close();
        }
        else
        {
            socket->write(mType.toUtf8() + returnFileData(loadFile));
            socket->waitForBytesWritten();
            socket->flush();
            socket->close();
        }
    }
}

void webserverthread::readLineFromSocket(QIODevice* device)
{
    #ifdef QT_DEBUG
        qDebug() << "WWW readLineFromSocket: " << device;
    #endif

    if (device == socket && socket->canReadLine())
    {
        QString line = socket->readLine();
        processData(line);
    }
    else if (device == sslsocket && sslsocket->canReadLine())
    {
        QString line = sslsocket->readLine();
        processData(line);
    }
}

void webserverthread::disconnected()
{
    #ifdef QT_DEBUG
        qDebug() << "WWW client disconnected: " << QThread::currentThreadId() << socketDescriptor;
        qDebug() << "WWW thread deleting: " << QThread::currentThreadId();
    #endif

    if(sslUsage == true)
    {
        if(sslsocket->isOpen())
        {
            sslsocket->close();
        }

        sslsocket->deleteLater();
    }
    else
    {
        if(socket->isOpen())
        {
            socket->close();
        }

        socket->deleteLater();
    }

    exit(0);
}

QByteArray webserverthread::returnFileData(QString filename)
{
    QFile file(filename);

    if(!file.open(QFile::ReadOnly))
    {
        #ifdef QT_DEBUG
            qDebug() << "WWW can't open file: " << filename << QThread::currentThreadId() << socketDescriptor;
        #endif

        return QByteArray();
    }

    QByteArray textOut = file.readAll();

    file.close();

    return templateReplace(textOut);
}

// some basic template stuff
QByteArray webserverthread::templateReplace(QByteArray data)
{
    // set app name
    data.replace("{{#appName}}", QString(APP_PRODUCT).toUtf8());
    data.replace("{{#appVersion}}", QString(APP_VERSION).toUtf8());

    // set the correct websocket
    if(sslUsage == true)
    {
        data.replace("{{#websocket}}", "wss");
    }
    else
    {
        data.replace("{{#websocket}}", "ws");
    }

    return data;
}

QString webserverthread::mimeReturn(const QFile& file)
{
    QString wwwRootPath;

    if(!serverBasePath.isEmpty())
    {
        wwwRootPath = serverBasePath;
    }
    else
    {
        // wwwRootPath = ":/webui/build/";
        wwwRootPath = ":/www/";
    }

    QMimeDatabase mimeDatabase;
    QMimeType mimeType;

    mimeType = mimeDatabase.mimeTypeForFile(QFileInfo(file));

    QString returnString = QString();

    // if(QFileInfo(file).absoluteFilePath() == ":/www/404.html")
    if(QFileInfo(file).absoluteFilePath() == wwwRootPath + "404.html")
    {
        returnString = "HTTP/1.0 404 Not Found\r\nContent-Type: text/html; charset=\"utf-8\"\r\n\r\n";
    }

    if(mimeType.inherits("text/html"))
    {
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: text/html; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("application/xhtml+xml"))
    {
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: application/xhtml+xml; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("application/xml"))
    {
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: application/xml; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("text/xml"))
    {
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: text/xml; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("text/css"))
    {
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: text/css; charset=\"utf-8\"\r\n\r\n";
    }
    else if(mimeType.inherits("image/gif"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: image/gif; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/vnd.microsoft.icon"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: image/vnd.microsoft.icon; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/jpeg"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: image/jpeg; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/png"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: image/png; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/webp"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: image/webp; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/avif"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: image/avif; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/svg+xml"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: image/svg+xml; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("image/tiff"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        return "HTTP/1.0 200 Ok\r\nContent-Type: image/tiff; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("text/javascript"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: text/javascript; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("application/json"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: application/json; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("font/otf"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: font/otf; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("font/ttf"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: font/ttf; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("font/woff"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: font/woff; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("font/woff2"))
    {
        QString fileSize = QString::number(QFileInfo(file).size());
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: font/woff2; Content-Length: " + fileSize + "\r\n\r\n";
    }
    else if(mimeType.inherits("text/plain") && !mimeType.inherits("text/css"))
    {
        returnString = "HTTP/1.0 200 Ok\r\nContent-Type: text/plain; charset=\"utf-8\"\r\n\r\n";
    }

    #ifdef QT_DEBUG
        qDebug() << "WWW returnSring: " << returnString << QFileInfo(file).fileName() << QThread::currentThreadId() << socketDescriptor;
    #endif

    // return QString();
    return returnString;
}
