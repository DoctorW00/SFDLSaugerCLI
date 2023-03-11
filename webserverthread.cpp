#include "webserverthread.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>

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

    socket = new QTcpSocket();

    if(!socket->setSocketDescriptor(this->socketDescriptor))
    {
        emit error(socket->error());
        return;
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError2(QAbstractSocket::SocketError)));

    socket->write("HTTP/1.0 200 Ok\r\n");
    socket->waitForBytesWritten();
    socket->close();

    exec();
}

void webserverthread::socketError2(QAbstractSocket::SocketError error)
{
    #ifdef QT_DEBUG
        qDebug() << "WWW: Socket error: " << error << QThread::currentThreadId() << socketDescriptor;
    #endif
}

void webserverthread::readyRead()
{
    #ifdef QT_DEBUG
        qDebug() << "WWW thread ready to read! " << QThread::currentThreadId() << socketDescriptor;
    #endif

    if(socket->canReadLine())
    {
        QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));

        #ifdef QT_DEBUG
            qDebug() << "WWW tokens: " << tokens << QThread::currentThreadId() << socketDescriptor;
        #endif

        if(tokens[0] == "GET")
        {
            #ifdef QT_DEBUG
                qDebug() << "WWW (GET) tokens[0]: " << tokens[0] << QThread::currentThreadId() << socketDescriptor;
            #endif

            if(tokens[1] == "/")
            {
                QString loadFile(":/www/index.html");
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
                    loadFile = ":/www" + loadFile;
                }

                QFile chkFile(loadFile);
                if(!chkFile.exists())
                {
                    loadFile = ":/www/404.html";
                }

                QString mType = mimeReturn(loadFile);
                socket->write(mType.toUtf8() + returnFileData(loadFile));

                socket->waitForBytesWritten();
                socket->close();

            }
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
