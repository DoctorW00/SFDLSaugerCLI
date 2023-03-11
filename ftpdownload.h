#ifndef FTPDOWNLOAD_H
#define FTPDOWNLOAD_H

#include <QObject>
#include <QMutex>
#include <QtFtp/QFtp>
#include <QFile>
#include <QTcpSocket>
#include <QEventLoop>
#include <QNetworkProxy>

#ifdef QT_DEBUG
    #include <QDebug>
#endif

class FTPDownload : public QObject
{
    Q_OBJECT

public:
    FTPDownload(QStringList data);
    ~FTPDownload();
    void abort();
    bool _abort;
    bool _working;
    int _tableRow = -1;
    QString _id;
    QFtp *ftp;

public slots:
    void process();

private slots:
    void isDone(bool);
    void updateProgress(qint64, qint64);
    void finishedDownload();

signals:
    void finished();
    void error(QString err);
    void statusUpdateFile(QString id, int tableRow, QString statusMsg, int status);
    void doProgress(QString id, int tableRow, qint64 read, qint64 total, bool overwriteTime, bool firstUpdate);

private:
    QStringList data;
    QFile *file;
    QEventLoop *ftpLoop;
    QMutex mutex;
    QString id;
    QString host;
    QString port;
    QString user;
    QString pass;
    QString dir;
    QString dlpath;
    QString dlfile;
    int tableRow;
    QNetworkProxy proxy;

};

#endif // FTPDOWNLOAD_H
