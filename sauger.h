#ifndef SAUGER_H
#define SAUGER_H

#include <QObject>
#include <QTextStream>
#include <QPair>
#include "ftpdownload.h"

class sauger : public QObject
{
    Q_OBJECT

public:
    explicit sauger(QObject *parent = nullptr);

public slots:
    void sart();
    void setData(QString sfdlFile, QString downloadPath, int maxThreads, bool forceOverwrite, QString password);
    void setProxy(QString proxyHost, int proxyPort, QString proxyUser, QString proxyPass);
    void createJson();

private slots:
    void printConsol(QString text);
    void printPlain(QString text);
    void getSFDLData(QStringList data, QStringList files);
    void receiveFTPFileIndex(int id, QStringList files);
    void startDownload();
    QStringList dirFromFilePath(QString filePath, QString name);
    qint64 getTimestamp();
    QPair<int, int> returnConsolSize();

private:
    QString _SFDLFile;
    QString _DownloadDestination;
    int _MaxDownloadThreads;
    bool _ForceOverwirteFiles;
    QString _SFDLPassword;
    QString _proxyHost;
    int _proxyPort;
    QString _proxyUser;
    QString _proxyPass;
    int _runningDownloads;
    QList<QThread*> g_Threads;
    QList<FTPDownload*> g_Worker;
    QStringList _data;
    QStringList _files;
    int ServerID = 0;
    int FileID = 0;
    int _consolCOL = 80;
    int _consolROW = 24;

};

#endif // SAUGER_H
