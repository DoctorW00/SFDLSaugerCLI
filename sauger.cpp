#include "sauger.h"
#include "sfdl.h"
#include "webserver.h"
#include "websocket.h"
#include "ftplistfiles.h"
#include "global.h"
#include <QFile>
#include <QFileInfo>
#include <QThread>

#ifdef Q_OS_WIN
    #include <windows.h>
#endif

#ifdef Q_OS_UNIX
    #include <sys/ioctl.h>
    #include <stdio.h>
    #include <unistd.h>
#endif

#ifdef QT_DEBUG
    #include <QDebug>
#endif

QVector<dServer> Servers;
QVector<dFile> Files;

sauger::sauger(QObject *parent) : QObject(parent)
{

    // set consol size
    QPair<int, int> consolsize = returnConsolSize();
    int cCol = consolsize.first;
    int cRow = consolsize.second;
    if(cCol && cRow)
    {
        _consolCOL = cCol;
        _consolROW = cRow;
    }
}

void sauger::printConsol(QString text)
{
    QTextStream cout(stdout);
    cout << "| " + text << endl;
    cout.flush();
}

void sauger::printPlain(QString text)
{
    QTextStream cout(stdout);
    cout << text << endl;
    cout.flush();
}

QPair<int, int> sauger::returnConsolSize()
{
    int cCol;
    int cRow;

#ifdef Q_OS_WIN
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    cCol = columns;
    cRow = rows;
#endif

#ifdef Q_OS_UNIX
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

    cCol = size.ws_col;
    cRow = size.ws_row;
#endif

    QPair<int, int> consol;
    consol.first = cCol;
    consol.second = cRow;

    return consol;
}

void sauger::sart()
{
    printConsol("-> " + _SFDLFile);

    auto *sfdlFile = new sfdl();

    connect(sfdlFile, SIGNAL(sendSFDLData(QStringList,QStringList)), this, SLOT(getSFDLData(QStringList,QStringList)));
    connect(sfdlFile, SIGNAL(sendLogText(QString)), this, SLOT(printConsol(QString)));

    sfdlFile->setSFDL(_SFDLFile, QStringList() << _SFDLPassword);
    sfdlFile->readSFDL();
}

void sauger::receiveFTPFileIndex(int id, QStringList files)
{
    #ifdef QT_DEBUG
        qDebug() << "receive ftp file index ...";
        qDebug() << "id: " << id;
        qDebug() << "files count: " << files.count();
        qDebug() << "files: " << files;
    #endif

    qint64 totalDownloadSize = 0;

    for(const auto& f : files)
    {
        FileID = FileID + 1;

        QStringList line = QString(f).split("|");
        if(line.count())
        {
            // get download name
            QString name;
            for(int i = 0; i < Servers.count(); i++)
            {
                if(Servers[i].id == id)
                {
                    name = Servers[i].name;
                    break;
                }
            }

            QStringList cleanFile = dirFromFilePath(line.at(0), name);
            QString cleanpath = cleanFile.at(0);
            QString filename = cleanFile.at(1);
            QString subDirs = cleanFile.at(2);

            QString filepath = line.at(0);
            qint64 fileSize = line.at(1).toLongLong();

            totalDownloadSize = totalDownloadSize + fileSize;

            Files.append({FileID, id, filename, filepath, cleanpath, subDirs, 0, fileSize, 0, 0, 0, 0});
        }
    }

    #ifdef QT_DEBUG
        qDebug() << "totalDownloadSize: " << totalDownloadSize;
    #endif

    // set total download size for this download
    for(int i = 0; i < Servers.count(); i++)
    {
        if(Servers[i].id == id)
        {
            Servers[i].total = totalDownloadSize;
            break;
        }
    }

    #ifdef QT_DEBUG
        foreach(dServer var, Servers)
        {
            qDebug() << var.sfdl;
            qDebug() << var.name;
            qDebug() << var.user;
            qDebug() << var.pass;
            qDebug() << "----------------------------";
        }

        foreach(dFile var, Files)
        {
            qDebug() << var.id;
            qDebug() << var.fileName;
            qDebug() << var.fullFilePath;
            qDebug() << var.cleanPath;
            qDebug() << var.subDirs;
            qDebug() << var.total;
            qDebug() << "----------------------------";
        }
    #endif

    createJson();
    startDownload();
}

void sauger::getSFDLData(QStringList data, QStringList files)
{
    #ifdef QT_DEBUG
        qDebug() << "data list: " << data;
        qDebug() << "files list: " << files;
    #endif

    if(data.count())
    {
        ServerID = ServerID + 1;

        QString sfdl = data.at(0).split("|").at(1);
        QString name = data.at(1).split("|").at(1);
        QString path = data.at(21).split("|").at(1);
        QString ip = data.at(7).split("|").at(1);
        qint16 port = data.at(8).split("|").at(1).toShort();
        QString user = data.at(9).split("|").at(1);
        QString pass = data.at(10).split("|").at(1);

        #ifdef QT_DEBUG
            qDebug() << "ServerID: " << ServerID;
        #endif

        Servers.append({ServerID, sfdl, name, path, ip, port, user, pass, 0, 0, 0, 0, 0, 0});
    }

    if(!files.count())
    {
        printConsol(tr("Loading file index from FTP server ..."));

        QStringList downloadList;
        downloadList.append(QString::number(ServerID));    // server id
        downloadList.append(data.at(7).split("|").at(1));  // host
        downloadList.append(data.at(8).split("|").at(1));  // port
        downloadList.append(data.at(9).split("|").at(1));  // user
        downloadList.append(data.at(10).split("|").at(1)); // pass
        downloadList.append(data.at(21).split("|").at(1)); // path

        auto thread = new QThread;
        auto listFTP = new FTPListFiles(downloadList);

        listFTP->moveToThread(thread);

        connect(listFTP, SIGNAL(finished()), thread, SLOT(quit()));
        connect(listFTP, SIGNAL(finished()), listFTP, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(thread, SIGNAL(started()), listFTP, SLOT(process()));

        connect(listFTP, SIGNAL(sendLogText(QString)), this, SLOT(printConsol(QString)));
        connect(listFTP, SIGNAL(sendFiles(int, QStringList)), this, SLOT(receiveFTPFileIndex(int, QStringList)));

        thread->start();

    }
    else
    {
        if(ServerID)
        {
            receiveFTPFileIndex(ServerID, files);
        }
    }

    /*
    #ifdef QT_DEBUG
        qDebug() << "files list: " << files;
    #endif

    if(!files.count())
    {
        printConsol("Error: There are no files to download!");
        exit(1);
    }

    _data = data;
    _files = files;

    startDownload();
    */

}

void sauger::startDownload()
{

    qDebug() << "startDownload!";

    /*
    if(_runningDownloads < _MaxDownloadThreads)
    {
        for(int i = 0; i < _files.count(); i++)
        {
            if(_runningDownloads >= _MaxDownloadThreads)
            {
                break;
            }

            QStringList downloadList;
            downloadList.append(_data.at(7).split("|").at(1));
            downloadList.append(_data.at(8).split("|").at(1).toInt());
            downloadList.append(_data.at(9).split("|").at(1));
            downloadList.append(_data.at(10).split("|").at(1));
            downloadList.append(_data.at(21).split("|").at(1));

            auto thread = new QThread;
            auto worker = new FTPDownload(downloadList);
            worker->moveToThread(thread);
            g_Worker.append(worker);

            connect(worker, SIGNAL(doProgress(QString,int,qint64,qint64,bool,bool)), this, SLOT(updateDownloadProgress(QString,int,qint64,qint64,bool,bool)));
            connect(worker, SIGNAL(statusUpdateFile(QString,int,QString,int)), this, SLOT(updateDownloadFileStatus(QString,int,QString,int)));
            connect(worker, SIGNAL(error(QString)), this, SLOT(downloadError(QString)));
            connect(thread, SIGNAL(started()), worker, SLOT(process()));
            connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
            connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
            connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

            thread->start();

            _runningDownloads++;
        }
    }
    */
}

void sauger::setData(QString sfdlFile, QString downloadPath, int maxThreads, bool forceOverwrite, QString password)
{
    int errors = 0;
    _SFDLFile = sfdlFile;
    _DownloadDestination = downloadPath;
    _MaxDownloadThreads = maxThreads;
    _ForceOverwirteFiles = forceOverwrite;
    _SFDLPassword = password;

    if(_SFDLFile.isEmpty())
    {
        printConsol(tr("Error: No SFDL file set!"));
        errors++;
    }
    else
    {
        QFile file(_SFDLFile);
        if(!file.exists())
        {
            printConsol(tr("Error: SFDL file <") + _SFDLFile + tr("> does not exist!"));
            errors++;
        }
    }

    if(!_DownloadDestination.isEmpty())
    {
        QFileInfo dPath(_DownloadDestination);
        if(!dPath.exists())
        {
            printConsol(tr("Error: Destination directory <") + dPath.absoluteFilePath() + tr("> does not exist!"));
            errors++;
        }
        else if(!dPath.isDir())
        {
            printConsol(tr("Error: Destination <") + dPath.absoluteFilePath() + tr("> is not a directory!"));
            errors++;
        }
        else if(!dPath.isWritable())
        {
            printConsol(tr("Error: Destination directory <") + dPath.absoluteFilePath() + tr("> is not writeable!"));
            errors++;
        }
    }
    else
    {
        printConsol(tr("Error: Destination directory is not set!"));
        errors++;
    }

    if(!_MaxDownloadThreads)
    {
        _MaxDownloadThreads = 3;
    }

    if(_SFDLPassword.isEmpty())
    {
        _SFDLPassword = "mlcboard.com";
    }

    if(errors)
    {
        exit(errors);
    }
}

void sauger::setProxy(QString proxyHost, int proxyPort, QString proxyUser, QString proxyPass)
{
    _proxyHost = proxyHost;
    _proxyPort = proxyPort;
    _proxyUser = proxyUser;
    _proxyPass = proxyPass;
}

// clean path and file from file full path
QStringList sauger::dirFromFilePath(QString filePath, QString name)
{
    QString fileName = filePath.split(QRegExp("[/\\\\]")).last();
    QString cleanPath = filePath.split(QRegExp(fileName)).at(0);
    QString subDir = filePath.split(name).at(1);
    subDir = subDir.split(fileName).at(0);

    if(subDir.startsWith("/"))
    {
        subDir.remove(0, 1);
    }

    QStringList result;

    result.append(cleanPath);
    result.append(fileName);
    result.append(subDir);

    return result;
}

void sauger::createJson()
{
    QJsonObject json;

    QJsonArray servers;
    foreach(dServer var, Servers)
    {
        QJsonArray s = {var.id, var.name, var.total};
        servers.append(s);
    }
    json["servers"] = servers;

    QJsonArray files;
    foreach(dFile var, Files)
    {
        QJsonArray f = {var.id, var.dServerID, var.fileName, var.total};
        files.append(f);
    }
    json["files"] = files;

    QJsonDocument doc(json);
    QString jsonString(doc.toJson(QJsonDocument::Compact));

    #ifdef QT_DEBUG
        qDebug() << "sauger.cpp - createJson(): " << jsonString;
    #endif
}

qint64 sauger::getTimestamp()
{
    return QDateTime::currentSecsSinceEpoch();
}
