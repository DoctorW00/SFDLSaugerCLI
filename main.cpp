#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "sauger.h"
#include "webserver.h"
#include "websocket.h"

#ifdef QT_DEBUG
    #include <QDebug>
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QString(APP_PRODUCT) + " v" + QString(APP_VERSION) + " (GrafSauger)");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption sfdlOption(QStringList() << "s" << "sfdl", QCoreApplication::translate("main", "Source SFDL file."));
    sfdlOption.setValueName("sfdl");
    parser.addOption(sfdlOption);

    QCommandLineOption destinationOption(QStringList() << "d" << "destination", QCoreApplication::translate("main", "Download destination directory."));
    destinationOption.setValueName("destination");
    parser.addOption(destinationOption);

    QCommandLineOption threadsOption(QStringList() << "t" << "threads", QCoreApplication::translate("main", "Set max threads for simultaneous downloads."));
    threadsOption.setValueName("threads");
    parser.addOption(threadsOption);

    QCommandLineOption forceoverwriteOption(QStringList() << "f" << "force", QCoreApplication::translate("main", "Overwrite existing files."));
    forceoverwriteOption.setValueName("forceoverwrite");
    parser.addOption(forceoverwriteOption);

    QCommandLineOption passwordOption(QStringList() << "p" << "password", QCoreApplication::translate("main", "Set password to decrypt SFDL file."));
    passwordOption.setValueName("password");
    parser.addOption(passwordOption);

    // Webserver
    QCommandLineOption wwwServerOption(QStringList() << "w" << "webserver", QCoreApplication::translate("main", "Start webserver (default: false)."));
    wwwServerOption.setValueName("webserver");
    parser.addOption(wwwServerOption);

    QCommandLineOption wwwIPOption(QStringList() << "ip", QCoreApplication::translate("main", "Webserver IP (default: 127.0.0.1"));
    wwwIPOption.setValueName("wwwIPOption");
    parser.addOption(wwwIPOption);

    QCommandLineOption wwwPortOption(QStringList() << "wwwPort", QCoreApplication::translate("main", "Webserver Port (default: 8869."));
    wwwPortOption.setValueName("wwwPort");
    parser.addOption(wwwPortOption);

    // ssl connection
    QCommandLineOption wwwSSLOption(QStringList() << "ssl", QCoreApplication::translate("main", "SSL Connection (default: false)."));
    wwwSSLOption.setValueName("ssl");
    parser.addOption(wwwSSLOption);

    QCommandLineOption wwwSSLCert(QStringList() << "sslCert", QCoreApplication::translate("main", "SSL certificate file location."));
    wwwSSLCert.setValueName("sslCert");
    parser.addOption(wwwSSLCert);

    QCommandLineOption wwwSSLKey(QStringList() << "sslKey", QCoreApplication::translate("main", "SSL key file location."));
    wwwSSLKey.setValueName("sslKey");
    parser.addOption(wwwSSLKey);

    // gui location - webserver will use this location as base path
    QCommandLineOption wwwGUIPath(QStringList() << "wwwGUIpath", QCoreApplication::translate("main", "WebGUI path (default: internal"));
    wwwGUIPath.setValueName("wwwGUIpath");
    parser.addOption(wwwGUIPath);

    // Websocket server
    /*
    QCommandLineOption wsServerOption(QStringList() << "x" << "wsserver", QCoreApplication::translate("main", "Start WSServer (default: true)."));
    wsServerOption.setValueName("wsserver");
    parser.addOption(wsServerOption);
    */

    QCommandLineOption wsPortOption(QStringList() << "wsPort", QCoreApplication::translate("main", "WSServer Port (default: 8870."));
    wsPortOption.setValueName("wsPortOption");
    parser.addOption(wsPortOption);

    // FTP Proxy Server
    QCommandLineOption proxyHostOption(QStringList() << "proxyHost", QCoreApplication::translate("main", "FTP Proxy Host IP or DNS."));
    proxyHostOption.setValueName("proxyHost");
    parser.addOption(proxyHostOption);

    QCommandLineOption proxyPortOption(QStringList() << "proxyPort", QCoreApplication::translate("main", "FTP Proxy Port."));
    proxyPortOption.setValueName("proxyPort");
    parser.addOption(proxyPortOption);

    QCommandLineOption proxyUserOption(QStringList() << "proxyUser", QCoreApplication::translate("main", "FTP Proxy Username."));
    proxyUserOption.setValueName("proxyUsername");
    parser.addOption(proxyUserOption);

    QCommandLineOption proxyPassOption(QStringList() << "proxyPass", QCoreApplication::translate("main", "FTP Proxy Port."));
    proxyPassOption.setValueName("proxyPassword");
    parser.addOption(proxyPassOption);

    parser.process(a);

    #ifdef QT_DEBUG
        qDebug() << "============== DEBUG ==============>";

        qDebug() << "sfdlOption: " << parser.value(sfdlOption);
        qDebug() << "destination: " << parser.value(destinationOption);
        qDebug() << "threadsOption: " << parser.value(threadsOption);
        qDebug() << "forceoverwriteOption: " << parser.value(forceoverwriteOption);
        qDebug() << "passwordOption: " << parser.value(passwordOption);

        qDebug() << "wwwServerOption: " << parser.value(wwwServerOption);
        qDebug() << "wwwIPOption: " << parser.value(wwwIPOption);
        qDebug() << "wwwPortOption: " << parser.value(wwwPortOption).toInt();
        qDebug() << "wwwSSLOption: " << parser.value(wwwSSLOption);
        qDebug() << "wwwSSLCert: " << parser.value(wwwSSLCert);
        qDebug() << "wwwSSLKey: " << parser.value(wwwSSLKey);
        qDebug() << "wwwGUIPath: " << parser.value(wwwGUIPath);

        // qDebug() << "wsServerOption: " << parser.value(wsServerOption);
        qDebug() << "wsPortOption: " << parser.value(wsPortOption).toInt();

        qDebug() << "proxyHostOption: " << parser.value(proxyHostOption);
        qDebug() << "proxyPortOption: " << parser.value(proxyPortOption).toInt();
        qDebug() << "proxyUserOption: " << parser.value(proxyUserOption);
        qDebug() << "proxyPassOption: " << parser.value(proxyPassOption);

        qDebug() << "<============== DEBUG ==============";
    #endif

    // check if sfdl file is set and start download
    if(!parser.value(sfdlOption).isEmpty())
    {
        auto sfdlsaugercli = new sauger();

        bool errorMsg = sfdlsaugercli->setData(parser.value(sfdlOption),
                     parser.value(destinationOption),
                     parser.value(threadsOption).toInt(),
                     QVariant(parser.value(forceoverwriteOption)).toBool(),
                     parser.value(passwordOption));

        if(errorMsg == true)
        {
            parser.showHelp(1);
        }

        sfdlsaugercli->sart();
    }
    else // or start web- and websocket server
    {
        int webserverPort = 8869;
        int websocketPort = 8870;

        if(!parser.value(wwwPortOption).isEmpty() && parser.value(wwwPortOption).toInt() > 0)
        {
            webserverPort = parser.value(wwwPortOption).toInt();
        }

        if(!parser.value(wsPortOption).isEmpty() && parser.value(wsPortOption).toInt() > 0)
        {
            websocketPort = parser.value(wsPortOption).toInt();
        }

        if(webserverPort == websocketPort)
        {
            webserverPort = 8869;
            websocketPort = 8870;
        }

        auto www = new webserver;

        QString hostIP = "127.0.0.1"; // default: 127.0.0.1
        if(!parser.value(wwwIPOption).isEmpty())
        {
            hostIP = parser.value(wwwIPOption);
        }
        www->setHostIP(hostIP);

        if(!parser.value(wwwGUIPath).isEmpty())
        {
            www->setGUIpath(parser.value(wwwGUIPath));
        }

        QString sslOption = parser.value(wwwSSLOption);
        if(sslOption.isEmpty())
        {
            sslOption = "false"; // default: no ssl
        }
        bool sslTrueOrFalse = (sslOption.compare("true", Qt::CaseInsensitive) == 0);
        www->setSSLUsage(sslTrueOrFalse);

        if(!parser.value(wwwSSLCert).isEmpty())
        {
            www->setCerFile(parser.value(wwwSSLCert));
        }

        if(!parser.value(wwwSSLKey).isEmpty())
        {
            www->setKeyFile(parser.value(wwwSSLKey));
        }

        // start webserver
        www->startServer(webserverPort);

        // websocket server
        auto ws = new websocket;

        ws->setSSLUsage(sslTrueOrFalse);

        if(!parser.value(wwwSSLCert).isEmpty())
        {
            ws->setCerFile(parser.value(wwwSSLCert));
        }

        if(!parser.value(wwwSSLKey).isEmpty())
        {
            ws->setKeyFile(parser.value(wwwSSLKey));
        }

        ws->setHostIP(hostIP);

        // start websocket server
        ws->start(websocketPort);

        auto sfdlsaugercli = new sauger();
        sfdlsaugercli->connect(www, SIGNAL(sendSFDLFile(QString)), sfdlsaugercli, SLOT(sart(QString)));
        // sfdlsaugercli->sart();
    }

    return a.exec();
}
