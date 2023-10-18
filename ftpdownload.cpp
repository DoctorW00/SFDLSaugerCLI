#include "ftpdownload.h"

FTPDownload::FTPDownload(QStringList data) : data(data)
{
    mutex.lock();
    _working = false;
    _abort = false;

    serverID = this->data.at(0).toInt();
    fileID = this->data.at(1).toInt();
    host = this->data.at(2);
    port = this->data.at(3);
    user = this->data.at(4);
    pass = this->data.at(5);
    dir = this->data.at(6);
    dlpath = this->data.at(7);
    dlfile = this->data.at(8);

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

    #ifdef QT_DEBUG
        qDebug() << "ftpdownload vars:";
        qDebug() << "serverID: " << serverID;
        qDebug() << "fileID: " << fileID;
        qDebug() << "host: " << host;
        qDebug() << "port: " << port;
        qDebug() << "user: " << user;
        qDebug() << "pass: " << pass;
        qDebug() << "dir: " << dir;
        qDebug() << "dlpath: " << dlpath;
        qDebug() << "dlfile: " << dlfile;
        qDebug() << "------------------------------------------";
    #endif

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
    emit statusUpdateFile(fileID, 1);

    mutex.lock();
    _working = true;
    mutex.unlock();

    timer = new QTimer(this);
    timer->setSingleShot(1);
    timer->start(60);

    ftpLoop = new QEventLoop(this);
    ftp = new QFtp(this);
    // ftp->setObjectName(id);


    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(ftpTimeout()));
    connect(ftp, SIGNAL(commandStarted(int)), this, SLOT(ftpCommandStarted(int)));
    connect(ftp, SIGNAL(commandFinished(int,bool)), this, SLOT(ftpCommandFinished(int,bool)));
    connect(ftp, SIGNAL(rawCommandReply(int,QString)), this, SLOT(ftpRawCommandReply(int,QString)));
    connect(ftp, SIGNAL(stateChanged(int)), this, SLOT(ftpstateChanged(int)));

    connect(ftp, SIGNAL(done(bool)), this, SLOT(isDone(bool)));
    connect(ftp, SIGNAL(dataTransferProgress(qint64,qint64)), this, SLOT(updateProgress(qint64, qint64)));

    // check and create local file path if needed
    QDir chkDlPath(dlpath);
    if(!chkDlPath.exists())
    {
        chkDlPath.mkpath(dlpath);
    }

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

            if(timer->isActive())
            {
                timer->stop();
            }
        }
        else
        {
            #ifdef QT_DEBUG
                qDebug() << "ftpdownload error: dlpath + dlfile is not writeable! " << dlpath + dlfile;
            #endif
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

            if(timer->isActive())
            {
                timer->stop();
            }
        }
        else
        {
            #ifdef QT_DEBUG
                qDebug() << "ftpdownload error: dlpath + dlfile is not writeable! " << dlpath + dlfile;
            #endif
        }
    }

    file->flush();
    file->close();
}

void FTPDownload::isDone(bool)
{
    if(timer->isActive())
    {
        timer->stop();
    }

    if(ftp->error())
    {
        /*
        QFtp::NoError           0	No error occurred.
        QFtp::HostNotFound      2	The host name lookup failed.
        QFtp::ConnectionRefused	3	The server refused the connection.
        QFtp::NotConnected      4	Tried to send a command, but there is no connection to a server.
        QFtp::UnknownError      1	An error other than those specified above occurred.
        */

        #ifdef QT_DEBUG
            qDebug() << dlfile << ": ftpdownload error: " << ftp->error() << ftp->errorString();
        #endif

        if(ftp->error() == 1)
        {
            emit statusUpdateFile(fileID, 2);
        }

        if(ftp->error() == 2)
        {
            emit statusUpdateFile(fileID, 2);
        }

        if(ftp->error() == 3)
        {
            emit statusUpdateFile(fileID, 3);
        }

        if(ftp->error() == 4)
        {
            emit statusUpdateFile(fileID, 4);
        }
    }
    else
    {
        emit statusUpdateFile(fileID, 10);
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
        emit doProgress(serverID, fileID, read, total, false, false);
    }
}

void FTPDownload::ftpTimeout()
{
    if(timer->isActive())
    {
        timer->stop();
    }

    if(ftpLoop->isRunning())
    {
        ftpLoop->exit(0);
    }

    ftp->abort();
    ftp->close();
    ftp->deleteLater();


    #ifdef QT_DEBUG
        qDebug() << dlfile << ": FTPDownload: ftp timeout!";
    #endif

    delete ftp;
}

void FTPDownload::ftpstateChanged(int state)
{
    state;

#ifdef QT_DEBUG
    qDebug() << dlfile << ": ftpstateChanged: " << state;
#endif
}

void FTPDownload::ftpRawCommandReply(int code, const QString & cmd)
{
    code;
    cmd;

#ifdef QT_DEBUG
    qDebug() << dlfile << ": ftpRawCommandReply: " << code << cmd;
#endif
}

void FTPDownload::ftpCommandStarted(int id)
{
    id;

#ifdef QT_DEBUG
    if(id == 0)
    {
        #ifdef QT_DEBUG
            qDebug() << dlfile << ": ftp commandStarted (Unconnected) id: " << id;
        #endif
    }

    if(id == 1)
    {
        #ifdef QT_DEBUG
            qDebug() << dlfile << ": ftp commandStarted (HostLookup) id: " << id;
        #endif
    }

    if(id == 2)
    {
        #ifdef QT_DEBUG
            qDebug() << dlfile << ": ftp commandStarted (Connecting) id: " << id;
        #endif
    }

    if(id == 3)
    {
        #ifdef QT_DEBUG
            qDebug() << dlfile << ": ftp commandStarted (Connected) id: " << id;
        #endif
    }

    if(id == 4)
    {
        #ifdef QT_DEBUG
            qDebug() << dlfile << ": ftp commandStarted (LoggedIn) id: " << id;
        #endif
    }

    if(id == 5)
    {
        #ifdef QT_DEBUG
            qDebug() << dlfile << ": ftp commandStarted (Closing) id: " << id;
        #endif
    }
#endif
}

void FTPDownload::ftpCommandFinished(int, bool error)
{
    if(ftp->currentCommand() == QFtp::ConnectToHost)
    {
        if(error)
        {
            #ifdef QT_DEBUG
                qDebug() << dlfile << ": ftp ConnectToHost error: " << error;
            #endif
        }
    }

    if(ftp->currentCommand() == QFtp::Login)
    {
        if(error)
        {
            #ifdef QT_DEBUG
                qDebug() << dlfile << ": ftp Login error: " << error;
            #endif
        }
    }

    if(ftp->currentCommand() == QFtp::Get)
    {
        if(error)
        {
            #ifdef QT_DEBUG
                qDebug() << dlfile << ": ftp Get error: " << error;
            #endif
        }
    }

    if(ftp->currentCommand() == QFtp::List)
    {
        if(error)
        {
            #ifdef QT_DEBUG
                qDebug() << dlfile << ": ftp List error: " << error;
            #endif
        }
    }
}
