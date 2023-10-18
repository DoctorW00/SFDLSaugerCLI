#ifndef FTPLISTFILES_H
#define FTPLISTFILES_H

#include <QObject>
#include <QTimer>
#include <QEventLoop>
#include <QtFtp/QFtp>
#include <QNetworkProxy>
#include <QMutex>

#ifdef QT_DEBUG
    #include <QDebug>
#endif

class FTPListFiles : public QObject
{
    Q_OBJECT

public:
    FTPListFiles(QStringList data);
    ~FTPListFiles();
    void abort();
    QFtp *ftp;
    bool _abort;
    bool _working;

public slots:
    void process();
    void ftpList(QString ip, int port, QString user, QString pass, QString path, QString SFDL);
    /*
    QStringList ftpList(QString ip, int port, QString user, QString pass, QString path, QString SFDL,
                        QString proxyHost, QString proxyPort, QString proxyUser, QString proxyPass);
                        */

private slots:
    void setFTP();
    void ftpTimeout();
    void ftpRawCommandReply(int code, const QString & cmd);
    void ftpCommandStarted(int id);
    void ftpCommandFinished(int, bool error);
    void ftpstateChanged(int state);
    void doListInfo(const QUrlInfo& info);
    void isDone(bool);
    void getFTPIndex(QString ip, int port, QString user, QString pass, QString path);

signals:
    void sendWarning(QString label, QString text);
    void sendLogText(QString text1, QString text2, QString text3);
    void sendErrorMsg(QString text);
    void sendFiles(int id, QStringList files);
    void finished();

private:
    QTimer *timer;
    QEventLoop *loop;
    int baseServerID;
    QString baseIP;
    int basePort;
    QString baseUser;
    QString basePass;
    QString basePath;
    QStringList pathList;
    QStringList fileList;
    QString baseSFDL;
    QNetworkProxy proxy;
    QMutex mutex;
    QStringList data;

};

#endif // FTPLISTFILES_H
