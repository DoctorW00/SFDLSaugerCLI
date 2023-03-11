#include "ftplistfiles.h"

#ifdef QT_DEBUG
    #include <QDebug>
#endif

// FTPListFiles::FTPListFiles(QObject *parent) : QObject(parent)
FTPListFiles::FTPListFiles(QStringList data) : data(data)
{
    baseServerID = data.at(0).toInt();
    baseIP = data.at(1);
    basePort = data.at(2).toInt();
    baseUser = data.at(3);
    basePass = data.at(4);
    basePath = data.at(5);

    #ifdef QT_DEBUG
        qDebug() << "FTPListFiles (baseServerID): " << baseServerID;
        qDebug() << "FTPListFiles (baseIP): " << baseIP;
        qDebug() << "FTPListFiles (basePort): " << basePort;
        qDebug() << "FTPListFiles (baseUser): " << baseUser;
        qDebug() << "FTPListFiles (basePass): " << basePass;
        qDebug() << "FTPListFiles (basePath): " << basePath;
    #endif
}

FTPListFiles::~FTPListFiles()
{
    if(_abort == false)
    {
        delete ftp;
    }
}

// ftp action
void FTPListFiles::setFTP()
{
    loop = new QEventLoop(this);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(ftpTimeout()));

    ftp = new QFtp(this);
    ftp->setTransferMode(QFtp::Passive);

    connect(ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(doListInfo(QUrlInfo)));
    connect(ftp, SIGNAL(done(bool)), this, SLOT(isDone(bool)));
    connect(ftp, SIGNAL(commandStarted(int)), this, SLOT(ftpCommandStarted(int)));
    connect(ftp, SIGNAL(commandFinished(int,bool)), this, SLOT(ftpCommandFinished(int,bool)));
    connect(ftp, SIGNAL(rawCommandReply(int,QString)), this, SLOT(ftpRawCommandReply(int,QString)));
    connect(ftp, SIGNAL(stateChanged(int)), this, SLOT(ftpstateChanged(int)));
}

void FTPListFiles::ftpTimeout()
{
    if(timer->isActive())
    {
        timer->stop();
    }

    if(loop->isRunning())
    {
        loop->exit(0);
    }

    ftp->abort();
    ftp->close();
    ftp->deleteLater();

    sendLogText(tr("<font color=\"red\">[") + baseSFDL + tr("] FTP Timeout! Keine Verbindung zum Server.</font>"));
    sendWarning(tr("FTP Timeout"), "[" + baseSFDL + "] FTP Timeout! Keine Verbindung zum Server.");

    delete ftp;
    setFTP();
}

void FTPListFiles::ftpstateChanged(int state)
{
    state;

#ifdef QT_DEBUG
    qDebug() << "ftpstateChanged: " << state;
#endif
}

void FTPListFiles::ftpRawCommandReply(int code, const QString & cmd)
{
    code;
    cmd;

#ifdef QT_DEBUG
    qDebug() << "ftpRawCommandReply: " << code << cmd;
#endif
}

void FTPListFiles::ftpCommandStarted(int id)
{
    id;

#ifdef QT_DEBUG
    if(id == 0)
    {
        #ifdef QT_DEBUG
            qDebug() << "ftp commandStarted (Unconnected) id: " << id;
        #endif
    }

    if(id == 1)
    {
        #ifdef QT_DEBUG
            qDebug() << "ftp commandStarted (HostLookup) id: " << id;
        #endif
    }

    if(id == 2)
    {
        #ifdef QT_DEBUG
            qDebug() << "ftp commandStarted (Connecting) id: " << id;
        #endif
    }

    if(id == 3)
    {
        #ifdef QT_DEBUG
            qDebug() << "ftp commandStarted (Connected) id: " << id;
        #endif
    }

    if(id == 4)
    {
        #ifdef QT_DEBUG
            qDebug() << "ftp commandStarted (LoggedIn) id: " << id;
        #endif
    }

    if(id == 5)
    {
        #ifdef QT_DEBUG
            qDebug() << "ftp commandStarted (Closing) id: " << id;
        #endif
    }
#endif
}

void FTPListFiles::ftpCommandFinished(int, bool error)
{
    if(ftp->currentCommand() == QFtp::ConnectToHost)
    {
        if(error)
        {
            #ifdef QT_DEBUG
                qDebug() << "ftp ConnectToHost error: " << error;
            #endif

            sendLogText(tr("<font color=\"red\">[") + baseSFDL + tr("] FTP Fehler: Verbindung zum Server fehlgeschalgen!</font>"));
        }
    }

    if(ftp->currentCommand() == QFtp::Login)
    {
        if(error)
        {
            #ifdef QT_DEBUG
                qDebug() << "ftp Login error: " << error;
            #endif

            sendLogText(tr("<font color=\"red\">[") + baseSFDL + tr("] FTP Fehler: Login fehlgeschalgen!</font>"));
        }
    }

    if(ftp->currentCommand() == QFtp::Get)
    {
        if(error)
        {
            #ifdef QT_DEBUG
                qDebug() << "ftp Get error: " << error;
            #endif

            sendLogText(tr("<font color=\"red\">[") + baseSFDL + tr("] FTP Fehler: Datei GET fehlgeschalgen!</font>"));
        }
    }

    if(ftp->currentCommand() == QFtp::List)
    {
        if(error)
        {
            #ifdef QT_DEBUG
                qDebug() << "ftp List error: " << error;
            #endif

            sendLogText(tr("<font color=\"red\">[") + baseSFDL + tr("] FTP Fehler: Datei Listing fehlgeschalgen!</font>"));
        }
    }
}

/*
QStringList FTPListFiles::ftpList(QString ip, int port, QString user, QString pass, QString path, QString SFDL,
                                  QString proxyHost, QString proxyPort, QString proxyUser, QString proxyPass)
*/
void FTPListFiles::ftpList(QString ip, int port, QString user, QString pass, QString path, QString SFDL)
{
    // set proxy
    /*
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

    if(user.isEmpty())
    {
        user = "anonymous";
    }

    if(pass.isEmpty())
    {
        pass = "anonymous@sfdlsauger.test";
    }

    if(path.isEmpty())
    {
        path = "/";
    }

    baseIP = ip;
    basePort = port;
    baseUser = user;
    basePass = pass;
    basePath = path;
    baseSFDL = SFDL;

    pathList.clear();

    getFTPIndex(baseIP, basePort, baseUser, basePass, basePath);

    // return fileList;
    emit sendFiles(baseServerID, fileList);
    emit finished();
}

void FTPListFiles::getFTPIndex(QString ip, int port, QString user, QString pass, QString path)
{
    basePath = path;

    // sendLogText(baseSFDL + tr(": Lade Inhalt von Pfad: ") + basePath);

    #ifdef QT_DEBUG
        qDebug() << "Get FTP index from path: " << basePath;
    #endif

    // setup ftp connects
    setFTP();

    timer = new QTimer(this);
    timer->setSingleShot(1);
    // connect(timer, SIGNAL(timeout()), this, SLOT(ftpTimeout()));
    timer->start(60);

    loop = new QEventLoop(this);
    loop->connect(ftp, SIGNAL(done(bool)), loop, SLOT(quit()));

    // connect to ftp server
    ftp->connectToHost(ip, port);
    ftp->login(user, pass);
    if(!path.isEmpty())
    {
        ftp->cd(path);
    }

    ftp->list();

    ftp->close();
    loop->exec();

    if(timer->isActive())
    {
        timer->stop();
    }

    for(int i = 0; i < pathList.count(); i++)
    { 
        #ifdef QT_DEBUG
            qDebug() << "pathList: " << pathList.at(i);
        #endif

        QString getPath = pathList.at(i);
        pathList.removeAt(i);

        getFTPIndex(baseIP, basePort, baseUser, basePass, getPath);
    }

}

void FTPListFiles::doListInfo(const QUrlInfo& info)
{
    if(info.isFile())
    {
        #ifdef QT_DEBUG
            qDebug() << "isFile: " << info.name() + "|" + QString::number(info.size());
        #endif

        if(basePath.endsWith("/"))
        {
            fileList.append(basePath + info.name() + "|" + QString::number(info.size()));
            // sendLogText(baseSFDL + ": " + basePath + info.name() + " [" + QString::number(info.size()) + "]");
        }
        else
        {
            fileList.append(basePath + "/" + info.name() + "|" + QString::number(info.size()));
            // sendLogText(baseSFDL + ": " + basePath + "/" + info.name() + " [" + QString::number(info.size()) + "]");
        }
    }

    if(info.isDir() && info.name() != "." && info.name() != "..")
    {
        #ifdef QT_DEBUG
            qDebug() << "isDir: " << info.name();
        #endif

        if(basePath.endsWith("/"))
        {
            pathList.append(basePath + info.name());
            // sendLogText(baseSFDL + ": " + basePath + info.name() + " [Verzeichnis]");
        }
        else
        {
            pathList.append(basePath + "/" + info.name());
            // sendLogText(baseSFDL + ": " + basePath + "/" + info.name() + " [Verzeichnis]");
        }
    }
}

void FTPListFiles::isDone(bool)
{
    if(timer->isActive())
    {
        timer->stop();
    }

    if(ftp->error())
    {
        #ifdef QT_DEBUG
            qDebug() << "FTP Error: " << ftp->error();
            qDebug() << "FTP ErrorString: " << ftp->errorString();
        #endif

        // sendLogText(tr("<font color=\"red\">[") + baseSFDL + tr("] FTP Fehler: ") + ftp->errorString() + "</font>");
        // sendWarning(tr("FTP Fehler"), "[" + baseSFDL + "] " + ftp->errorString());
    }
    else
    {
        #ifdef QT_DEBUG
            qDebug() << "Alle FTP Operationen ohne Fehler beendet!";
        #endif
    }
}

void FTPListFiles::abort()
{
    mutex.lock();
    _abort = true;
    _working = false;
    mutex.unlock();
}

void FTPListFiles::process()
{
    // emit statusUpdateFile(id, _tableRow, tr("Wird geladen ..."), 1);

    mutex.lock();
    _working = true;
    mutex.unlock();

    if(baseUser.isEmpty())
    {
        baseUser = "anonymous";
    }

    if(basePass.isEmpty())
    {
        basePass = "anonymous@sfdlsauger.test";
    }

    if(basePath.isEmpty())
    {
        basePath = "/";
    }

    pathList.clear();

    getFTPIndex(baseIP, basePort, baseUser, basePass, basePath);

    // return fileList;
    emit sendFiles(baseServerID, fileList);
    emit finished();

    // mutex.unlock();
}
