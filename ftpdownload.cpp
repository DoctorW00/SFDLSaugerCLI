#include "ftpdownload.h"

FTPDownload::FTPDownload(QStringList data) : data(data)
{
    mutex.lock();
    _working = false;
    _abort = false;

    id = this->data.at(0);
    host = this->data.at(1);
    port = this->data.at(2);
    user = this->data.at(3);
    pass = this->data.at(4);
    dir = this->data.at(5);
    dlpath = this->data.at(6);
    dlfile = this->data.at(7);

    /*
    QString proxyHost = this->data.at(9);
    QString proxyPort = this->data.at(10);
    QString proxyUser = this->data.at(11);
    QString proxyPass = this->data.at(12);

    // set proxy
    if(!proxyHost.isEmpty() && !proxyPort.isEmpty())
    {
        proxy.setType(QNetworkProxy::Socks5Proxy);

        proxy.setHostName(proxyHost);
        proxy.setPort(proxyPort.toInt());

        if(!proxyUser.isEmpty() && !proxyPass.isEmpty())
        {
            proxy.setUser(proxyUser);
            proxy.setPassword(proxyPass);
        }

        QNetworkProxy::setApplicationProxy(proxy);
    }
    */

    mutex.unlock();
}

FTPDownload::~FTPDownload()
{
    if(_abort == false)
    {
        delete ftp;
        delete file;
    }
}

void FTPDownload::abort()
{
    mutex.lock();
    _abort = true;
    _working = false;
    mutex.unlock();
}

void FTPDownload::finishedDownload()
{
    mutex.lock();
    _working = false;
    mutex.unlock();
}

void FTPDownload::process()
{
    emit statusUpdateFile(id, _tableRow, tr("Wird geladen ..."), 1);

    mutex.lock();
    _working = true;
    mutex.unlock();

    // mutex.lock();

    ftpLoop = new QEventLoop(this);
    ftp = new QFtp(this);
    // ftp->setObjectName(id);

    connect(ftp, SIGNAL(done(bool)), this, SLOT(isDone(bool)));
    connect(ftp, SIGNAL(dataTransferProgress(qint64,qint64)), this, SLOT(updateProgress(qint64, qint64)));

    file = new QFile(this);
    file->setFileName(dlpath + dlfile);

    if(file->size())
    {
        // continue download
        if(file->open(QIODevice::Append) && file->isWritable())
        {

            ftpLoop->connect(ftp, SIGNAL(done(bool)), ftpLoop, SLOT(quit()));
            ftp->connectToHost(host, port.toInt());
            ftp->login(user, pass);
            ftp->cd(dir);

            ftp->m_fileSize = file->size(); // set file size

            ftp->get(dlfile, file);
            ftp->close();
            ftpLoop->exec();
        }
    }
    else
    {
        // download from scratch
        if(file->open(QIODevice::WriteOnly) && file->isWritable())
        {
            ftpLoop->connect(ftp, SIGNAL(done(bool)), ftpLoop, SLOT(quit()));
            ftp->connectToHost(host, port.toInt());
            ftp->login(user, pass);
            ftp->cd(dir);
            ftp->m_fileSize = 0;
            ftp->get(dlfile, file);
            ftp->close();
            ftpLoop->exec();
        }
    }

    file->flush();
    file->close();

    // mutex.unlock();
}

void FTPDownload::isDone(bool)
{
    if(ftp->error())
    {
        /*
        QFtp::NoError           0	No error occurred.
        QFtp::HostNotFound      2	The host name lookup failed.
        QFtp::ConnectionRefused	3	The server refused the connection.
        QFtp::NotConnected      4	Tried to send a command, but there is no connection to a server.
        QFtp::UnknownError      1	An error other than those specified above occurred.
        */

        if(ftp->error() == 1)
        {
            emit statusUpdateFile(id, _tableRow, tr("Unbekannter Fehler"), 1);
        }

        if(ftp->error() == 2)
        {
            emit statusUpdateFile(id, _tableRow, tr("Host nicht gefunden"), 2);
        }

        if(ftp->error() == 3)
        {
            emit statusUpdateFile(id, _tableRow, tr("Verbindung verweigert"), 3);
        }

        if(ftp->error() == 4)
        {
            emit statusUpdateFile(id, _tableRow, tr("Nicht verbunden"), 4);
        }
    }
    else
    {
        emit statusUpdateFile(id, _tableRow, tr("Fertig!"), 10);
    }

    if(file->isOpen())
    {
        file->close();
    }

    mutex.lock();
    _working = false;
    mutex.lock();

    emit finished();
}

void FTPDownload::updateProgress(qint64 read, qint64 total)
{
    if(_working && !_abort)
    {
        emit doProgress(id, _tableRow, read, total, false, false);
    }
}
