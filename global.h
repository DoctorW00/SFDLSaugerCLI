#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QVector>

struct dServer
{
    int id;
    QString sfdl;         // sfdl file
    QString name;         // download name
    QString path;         // base path on ftp server
    QString ip;           // ftp server ip
    int port;             // ftp server port
    QString user;         // ftp server username
    QString pass;         // ftp server password
    qint64 progress;      // overall download progress
    qint64 loaded;        // loaded bytes
    qint64 total;         // total download size
    int status;           // download status
    qint64 dateStart;     // timestamp download start
    qint64 dateStop;      // timestamp download stop
    qint64 dateCancel;    // timestamp download canceled
};

struct dFile
{
    int id;
    int dServerID;        // dServer id
    QString fileName;     // name of the file
    QString fullFilePath; // full download path on ftp server
    QString cleanPath;    // clean path without filename
    QString subDirs;      // sub dir structur
    qint64 progress;      // current download progress
    qint64 loaded;        // loaded bytes
    qint64 total;         // total file size
    int status;           // download status
    qint64 dateStart;     // timestamp download start
    qint64 dateStop;      // timestamp download stop
    qint64 dateCancel;    // timestamp download canceled
    bool selected;        // select file for download
};

// webui login info
struct dConnectionInfo
{
    QString username;     // username
    QString token;        // Json Web Token
};

#endif // GLOBAL_H
