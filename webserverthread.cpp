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

void webserverthread::run()
{
    socketDescriptor = this->socketDescriptor;

    #ifdef QT_DEBUG
        qDebug() << "WWW thread started: " << QThread::currentThreadId() << socketDescriptor;
    #endif

    // socket = new QTcpSocket();
    socket = new QSslSocket;

    if(!socket->setSocketDescriptor(this->socketDescriptor))
    {
        emit error(socket->error());
        return;
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
    connect(socket, SIGNAL(encrypted()), this, SLOT(isencrypted()));

    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError2(QAbstractSocket::SocketError)));

    // ssl
    const auto certs = QSslCertificate::fromPath(QLatin1String(":/cert/localhost.crt"));

    #ifdef QT_DEBUG
        qDebug() << "WWW: Certs: " << certs << QThread::currentThreadId() << socketDescriptor;
    #endif

    if(certs.length())
    {
        m_sslLocalCertificate = certs.at(0);
    }
    socket->setLocalCertificate(m_sslLocalCertificate);

    QFile keyFile(":/cert/localhost.decrypted.key");
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

    socket->setPrivateKey(m_sslPrivateKey);
    socket->setProtocol(QSsl::TlsV1_2);
    socket->startServerEncryption();

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
        qDebug() << "WWW thread ready to read! " << QThread::currentThreadId() << socketDescriptor;
    #endif

    if(socket->canReadLine())
    {
        QString socketData;
        while (socket->canReadLine())
        {
            socketData += socket->readLine();
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
                QString loadFile(":/react/saugergui/build/index.html");
                QString mType = mimeReturn(loadFile);

                socket->write(mType.toUtf8() + returnFileData(loadFile));

                socket->waitForBytesWritten();
                socket->close();
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
                    loadFile = ":/react/saugergui/build" + loadFile;

                    #ifdef QT_DEBUG
                        qDebug() << "WWW (GET) loadFile: " << loadFile << QThread::currentThreadId() << socketDescriptor;
                    #endif
                }

                QFile chkFile(loadFile);
                if(!chkFile.exists())
                {
                    loadFile = ":/www/404.html";

                    #ifdef QT_DEBUG
                        qDebug() << "WWW (GET) loadFile not found 4040: " << loadFile << QThread::currentThreadId() << socketDescriptor;
                    #endif
                }

                QString mType = mimeReturn(loadFile);
                socket->write(mType.toUtf8() + returnFileData(loadFile));

                socket->waitForBytesWritten();
                socket->flush();
                socket->close();

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
                loadFile = ":/react/saugergui/build" + loadFile;

                #ifdef QT_DEBUG
                    qDebug() << "WWW (POST) loadFile: " << loadFile << QThread::currentThreadId() << socketDescriptor;
                #endif
            }

            QFile chkFile(loadFile);
            if(!chkFile.exists())
            {
                loadFile = ":/www/404.html";

                #ifdef QT_DEBUG
                    qDebug() << "WWW (POST) loadFile not found 4040: " << loadFile << QThread::currentThreadId() << socketDescriptor;
                #endif
            }

            QString mType = mimeReturn(loadFile);
            socket->write(mType.toUtf8() + returnFileData(loadFile));

            socket->waitForBytesWritten();
            socket->flush();
            socket->close();

        }
    }
}

void webserverthread::disconnected()
{
    #ifdef QT_DEBUG
        qDebug() << "WWW client disconnected: " << QThread::currentThreadId() << socketDescriptor;
    #endif

    if(socket->isOpen())
    {
        socket->close();
    }

    #ifdef QT_DEBUG
        qDebug() << "WWW thread deleting: " << QThread::currentThreadId();
    #endif

    socket->deleteLater();
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

    return textOut;
}

QString webserverthread::mimeReturn(const QFile& file)
{
    QMimeDatabase mimeDatabase;
    QMimeType mimeType;

    mimeType = mimeDatabase.mimeTypeForFile(QFileInfo(file));

    QString returnString = QString();

    if(QFileInfo(file).absoluteFilePath() == ":/www/404.html")
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
