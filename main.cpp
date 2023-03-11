#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "sauger.h"
#include "webserver.h"
#include "websocket.h"

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

    QCommandLineOption wwwPortOption(QStringList() << "wwwPort", QCoreApplication::translate("main", "Webserver Port (default: 8869."));
    wwwPortOption.setValueName("wwwPortOption");
    parser.addOption(wwwPortOption);

    // Websocket server
    QCommandLineOption wsServerOption(QStringList() << "x" << "wsserver", QCoreApplication::translate("main", "Start WSServer (default: false)."));
    wsServerOption.setValueName("wsserver");
    parser.addOption(wsServerOption);

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


    // auto www = new webserver();
    // www->startServer(parser.value(wwwPortOption).toInt());

    bool setWebGUI = false;

    // start webserver
    if(QVariant(parser.value(wwwServerOption)).toBool())
    {
        setWebGUI = true;

        webserver www;
        www.startServer(parser.value(wwwPortOption).toInt());

        if(QVariant(parser.value(wsServerOption)).toBool())
        {
            websocket ws;
            ws.start(parser.value(wsPortOption).toInt());
        }
    }

    if(!setWebGUI)
    {
        if(parser.value(sfdlOption).isEmpty() || parser.value(destinationOption).isEmpty())
        {
            parser.showHelp(1);
        }

        auto sfdlsaugercli = new sauger();

        sfdlsaugercli->setData(parser.value(sfdlOption),
                     parser.value(destinationOption),
                     parser.value(threadsOption).toInt(),
                     QVariant(parser.value(forceoverwriteOption)).toBool(),
                     parser.value(passwordOption));

        sfdlsaugercli->sart();
    }


    /*
    bool setWebGUI = false;

    // start webserver
    if(QVariant(parser.value(wwwServerOption)).toBool())
    {
        setWebGUI = true;

        // auto www = new webserver();
        // www->start(parser.value(wwwPortOption).toInt());

        // webserver www;
        // www.startServer();

        // auto www = new webserver();
        // www->startServer();

        // auto www = new webserver();
        // www->startServer();

        webserver www;
        www.startServer();
    }
    */

    /*
    // start websocket server
    if(QVariant(parser.value(wsServerOption)).toBool())
    {
        setWebGUI = true;

        auto ws = new websocket();
        ws->start(parser.value(wsPortOption).toInt());
    }

    auto muh = new sauger();

    if(!setWebGUI)
    {
        muh->setData(parser.value(sfdlOption),
                     parser.value(destinationOption),
                     parser.value(threadsOption).toInt(),
                     QVariant(parser.value(forceoverwriteOption)).toBool(),
                     parser.value(passwordOption));

        muh->sart();
    }
    */

    /*
    muh->setProxy(parser.value(proxyHostOption),
                  parser.value(proxyPortOption).toInt(),
                  parser.value(proxyUserOption),
                  parser.value(proxyPassOption));
    */

    return a.exec();
}
