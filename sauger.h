#ifndef SAUGER_H
#define SAUGER_H

#include <QObject>
#include <QTextStream>
#include <QPair>
#include <QTimer>
#include "ftpdownload.h"

class sauger : public QObject
{
    Q_OBJECT

public:
    explicit sauger(QObject *parent = nullptr);

public slots:
    void sart();
    void sart(QString sfdlFileName);
    bool setData(QString sfdlFile, QString downloadPath, int maxThreads, bool forceOverwrite, QString password);
    void setProxy(QString proxyHost, int proxyPort, QString proxyUser, QString proxyPass);
    void createJson();
    void startDownload();
    void stopDownload();

private slots:
    void printGraf();
    void printBanner();
    void clearConsole();
    void printError(QString text);
    void printStausText(QString text1, QString text2, QString text3);
    QString fitMyString(QString text);
    void printConsol(QString text);
    void printPlain(QString text, QString color);
    void printFileProgress(QString fileName, qint64 loaded, qint64 total, int percent);
    void getSFDLData(QStringList data, QStringList files);
    void receiveFTPFileIndex(int id, QStringList files);
    QStringList dirFromFilePath(QString filePath, QString name);
    qint64 getTimestamp();
    QPair<int, int> returnConsolSize();
    QStringList dirFromFilePath(QString filePath);
    void downloadError(QString error);
    void updateDownloadFileStatus(int fID, int status);
    void updateDownloadProgress(int serverID, int fileID, qint64 read, qint64 total, bool overwriteTime = false, bool firstUpdate = false);
    void printDownloadProgress();
    int getServerIndexByID(int serverID);
    int getFileIndexByID(int fileID);
    QString seconds_to_DHMS(int duration);
    QString bytes2Human(float filesize);
    int returnPercent(qint64 read, qint64 total);
    QString returnProgressBar(int progress, int len);

private:
    QString _SFDLFile;
    QString _DownloadDestination;
    int _MaxDownloadThreads = 3;
    bool _ForceOverwirteFiles;
    QString _SFDLPassword;
    QString _proxyHost;
    int _proxyPort;
    QString _proxyUser;
    QString _proxyPass;
    int _runningDownloads = 0;
    QList<QThread*> g_Threads;
    QList<FTPDownload*> g_Worker;
    QStringList _data;
    QStringList _files;
    int ServerID = 0;
    int FileID = 0;
    int _consolCOL = 80;
    int _consolROW = 24;
    QTimer *pProgress;
    int cProgressbarLen;

};

#endif // SAUGER_H
