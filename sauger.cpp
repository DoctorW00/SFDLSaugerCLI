#include "sauger.h"
#include "sfdl.h"
#include "webserver.h"
#include "websocket.h"
#include "ftplistfiles.h"
#include "global.h"
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <iostream>

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
    cProgressbarLen = 10; // length (chars) of the progressbar (console output)

    // activate ansi support for windows 10 console
    #ifdef Q_OS_WIN
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hConsole, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hConsole, dwMode);

        // set unicode font
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_FONT_INFOEX fontInfo;
        fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
        GetCurrentConsoleFontEx(consoleHandle, FALSE, &fontInfo);
        wcscpy_s(fontInfo.FaceName, L"Consolas");
        SetCurrentConsoleFontEx(consoleHandle, FALSE, &fontInfo);
        SetConsoleOutputCP(CP_UTF8);
    #endif

    // set consol size
    QPair<int, int> consolsize = returnConsolSize();
    int cCol = consolsize.first;
    int cRow = consolsize.second;
    if(cCol && cRow)
    {
        _consolCOL = cCol;
        _consolROW = cRow;
    }

    pProgress = new QTimer(this);
    connect(pProgress, SIGNAL(timeout()), this, SLOT(printDownloadProgress()));
    // pProgress->start(1000);

}

void sauger::printGraf()
{
    QTextStream cout(stdout);
    cout.setCodec("UTF-8");

    QString asciiGraf = QString::fromUtf8(
        "\033[47m\033[30m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▒▒▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓███████████████████▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓███████▓▓▓▓███▓█▓▓███▓▓▓███▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓████████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓█▓█████████▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓████▓▓█▓▓▓▓▓▓▓▓▓▓█████▓▓▒▓▓▓██▓▒▒▒▒▓███▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒████▓▓▓▓▓▓▓▒▒▒▒░░░░░░▒▒▓▓▓▒▒▒▓▓▓░░░░░░░▒▓██▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓████▓▓▓▓▓▒░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░▒▓█▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓█▓▓▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓██▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░▒▒▒░▒▒▒▒▒▒▒▒▓▓█▓▓█▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒█▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░▒▒░▒▒▒░▒▓▓█▓▓▓▓▓▓▓▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▓█▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░░░░░░▒▓▓██▓▓▓▓▓▓▒▒░░░░░░░░▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▓█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░░░░░░▓▓████▓▓▓▓▒▒▒░░░░░░▒▒▒▒▒▒░░░░░▒░░░░░░░░░░░░░░▒▒▒▒▒▓▓▓▓▓▓▓▓▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░░░░░▒███▓▓▓███▓▓▒▒░░░░░▒▒▓▒▒▒▒▓▒▒▒▒▓▒▒▒▒▒▒░░░░░░░░░░▒▒▒▒▒▒▒▒▓▓▓▒▒░▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░░░░░▓████▓▓▓▓▓▓▒▒▒░░░░░▓▓▓▒▒▓▓▒▒▒▒▒░░▒▒▒▒▒▒░░░░░░░░▒▒▒▒░░░░░░▒▓▒▒░▒░░▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░░▒▒▓▓▓▓▓▓▒▓▓▓▒▒▒▒▒▒▒▒░▒▓▓▒▒▒▒░▒░░░░░░▒▒▒▒▒▒▒▒░░░░░▒▒▒▒▒▒▒▒▒▒░░▒▒▒▒▒▒░░▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░░░▒▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒░░▓▓▒▒░░░░░░░▒▒▒▒▒▓▒▓▓▓▒░░░░░▓▓▓▒▒▒▒▓▓▓▒▒▒▓▒▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░░▒▒▒▓█▒▒▓▒▓▒░▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░▒▒▓▓▓▓▓▒▓▓▓▓▓▓▓▒░░░░▓▓▒██▓▒░▒▒░░▒▒▓▒▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░▒▒░▒▒▒▒▓▒▒░▒▒▒░▒▒▒▒▒▒▒▒▒▒▒░░░▒▒▓▓▓▒░░▒▒▒▒▒▓▒▒▒░░░░░▒▒▓▓▓▒░░░░░░▒▓▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░▒▒░▒▒░░▒▓▒▒░▒░▒▒▒▒▒▒▒▒▒▒░░░░░▒▒▓▓▓▓▒░░░░▒░▒▒▒▒░░░░░░░░░░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░░░░░▒▒▒▒░▒░░▒░▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░▒░░░░░░░░░░░░░░░░░░░░▒▒▒█████▓▓▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░░░░▒▓▒░▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▓█████████▓▓▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░░░▒▓▓▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░▒▒░░░░░░░░░░░░░░░░░░▓█████████████▓▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░░░▓██▓▒▓▓▒▒░▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░▒░░▒░░░░░░░░░░░░░░░░░░▓███████████████\033[0m\n"
        "\033[47m\033[30m░░▒▒▓▓███████▓█▓▓▒▒░▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒▒░░░░░░░░▓▒░░░░░░░▓███████████████\033[0m\n"
        "\033[47m\033[30m▒▓███████████▓███▓▓▓▒▒▒▒▒░▒░▒▒▒░░░░░░░░░░░░░░░░░░░░▓█████▓▓▒▒▒▒▒░░▓▒░░░░░░▒███████████████\033[0m\n"
        "\033[47m\033[30m███████████████▓▓▓▓▓▓▒▒▒▒▒▒▒░▒▒░░░░░░░░░░░░░░░░░░░░░▒▒▒▓█████▓▒░░░▒▓▒░░░▒░▓███████████████\033[0m\n"
        "\033[47m\033[30m███████████████▓▓▓██▓▓▓▓▓▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░▒▓███▒░░░░░▒▒░░░░░▓███████████████\033[0m\n"
        "\033[47m\033[30m███████████████████▓███▓▓▓▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒▒░░▒▒░░░░░▓████████▓▓▓▓▓▓▓\033[0m\n"
        "\033[47m\033[30m█████████████████████████▓▓▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░▒▒▓▓▓▓███████▒▒▒░░░░░██████▓▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m███████████████████▓██████▓▓▓▒▒▒▒▒▒░░░░░░░░░░░░▒▒▓▓███████████████▓▒▒░░░░▒▓▓█▓▓▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m██████████████████████████▓▓▓▓▓▒▒▒▒▒░░░░░░░░░▒▒▓███████████████▓▓▓▒▒░░░░░▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m████████████████████████████▓▓▓▓▓▒▒▒░░░░░░░░░▒████▓▓▓▒▒▒▒▒▒▒▒▓█▓▓▓░▒░░░░▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m███████████████████████████████▓▓▓▒▒▒░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒░▒░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m█████████████████████████████████▓▓▒▒▒▒▒░░░░░░░▒▒▒▒▒▒▒░░░▒▒▒▒▒▒▓█▒░░▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m███████████████████████████████████▓▓▒▒▒▒▒░░░░░░░▒▓▓▒▓▓▒▒▒▒▓▓▓█▓▒░░░░▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m▓▒▒▒▒▒▒▒▒▒▒▒▓███████████████████████▓▓▒▒▒▒▒░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m▒▒░░░░░▒░░▒░░████████▓▓██████████████▓▓▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░░░░▒▒░▒▒▓████████▓▓▓████████████▓▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░░▒▒▒▒▒▒▒▒▓██████████▓▓▓███▓▓██████▓▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░▓███▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░░░░▒▒▒▒▒▒▓███████████████▓▓▓▓█▓▓▓▓█████▓▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░▒█████████▓▓▒▒▒▒▒▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░▒▒▒▒▒▒▓█████████████████████▓▓▓▓▓▓▓▓█████▓▓▒▒▒▓▓▒░░░░░░░░░░░░░░░░▒███████████████▓▓▒▒▒▒▒\033[0m\n"
        "\033[47m\033[30m░░▒▒▓▓██████████████████████████▓▓▓▒▒▓████████▓▒▒▓█▓▓▒▒░░░░▒▒▒▒▒▒▓██████████████████████▓▓\033[0m\n"
        "\033[47m\033[30m▒▓████████████████████████████████▓▓▒░▒▓▓▓▓██████▓▓█████▓▓▓▓██████████████████████████████\033[0m\n"
        "\033[47m\033[30m████████████████████████████████████▓▓▒░▒▓▓▒▓█████████████████████████████████████████████\033[0m\n"
        "\033[47m\033[30m███████████████████████████████████████▓▒▒▓▓▒░░▒▓█████████████████████████████████████████\033[0m\n"
    );

    // cout << "\033[47m\033[30m" << asciiGraf << "\033[0m" << endl;
    cout << asciiGraf << endl;
    cout.flush();
}

void sauger::printBanner()
{
    QTextStream cout(stdout);

    cout << "\x1B[1;33m SFDLSaugerCLI \x1B[1;35m" << QString(APP_VERSION) << "\x1B[1;33m (GrafSauger)\x1B[1;32m" << R"(
   _____ ______ _____  _       _____                              _____ _      _____
  / ____|  ____|  __ \| |     / ____|                            / ____| |    |_   _|
 | (___ | |__  | |  | | |    | (___   __ _ _   _  __ _  ___ _ __| |    | |      | |
  \___ \|  __| | |  | | |     \___ \ / _` | | | |/ _` |/ _ \ '__| |    | |      | |
  ____) | |    | |__| | |____ ____) | (_| | |_| | (_| |  __/ |  | |____| |____ _| |_
 |_____/|_|    |_____/|______|_____/ \__,_|\__,_|\__, |\___|_|   \_____|______|_____|
                                                  __/ |
                                                 |___/
 )" << "\x1B[0m" << endl;


    cout.flush();
}

void sauger::printConsol(QString text)
{
    QTextStream cout(stdout);
    cout << "| " + text << endl;
    cout.flush();
}

void sauger::printError(QString text)
{
    QTextStream cout(stdout);
    cout << "\x1B[1;31m[ERROR] " + text + "\x1B[0m" << endl;
    cout.flush();
}

void sauger::printStausText(QString text1, QString text2, QString text3)
{
    QTextStream cout(stdout);
    cout << "\x1B[1;32m[" + text1 + "] \x1B[1;36m" + text2 + " \x1B[1;35m " + text3 + "\x1B[0m" << endl;
    cout.flush();
}

void sauger::printPlain(QString text, QString color)
{
    QTextStream cout(stdout);

    if(color == "red")
    {
        cout << "\x1B[1;31m" + text + "\x1B[0m" << endl;
    }
    else if(color == "yellow")
    {
        cout << "\x1B[1;33m" + text + "\x1B[0m" << endl;
    }
    else if(color == "green ")
    {
        cout << "\x1B[1;32m" + text + "\x1B[0m" << endl;
    }
    else if(color == "blue ")
    {
        cout << "\x1B[1;34m" + text + "\x1B[0m" << endl;
    }
    else if(color == "white")
    {
        cout << "\x1B[1;37m" + text + "\x1B[0m" << endl;
    }
    else if(color == "magenta")
    {
        cout << "\x1B[1;35m" + text + "\x1B[0m" << endl;
    }
    else if(color == "cyan")
    {
        cout << "\x1B[1;36m" + text + "\x1B[0m" << endl;
    }
    else if(color == "purple")
    {
        cout << "\x1B[1;35m" + text + "\x1B[0m" << endl;
    }
    else if(color == "brown")
    {
        cout << "\x1B[1;33m" + text + "\x1B[0m" << endl;
    }
    else if(color == "gray")
    {
        cout << "\x1B[1;30m" + text + "\x1B[0m" << endl;
    }
    else if(color == "lightgrey")
    {
        cout << "\x1B[1;37m" + text + "\x1B[0m" << endl;
    }
    else
    {
        cout << " " << text << endl;
    }

    cout.flush();
}

void sauger::printFileProgress(QString fileName, qint64 loaded, qint64 total, int percent)
{
    QTextStream cout(stdout);
    cout.setCodec("UTF-8");

    cout << "\x1B[1;33m" + fitMyString(fileName) + " \x1B[0m" + returnProgressBar(percent, cProgressbarLen) + " \x1B[1;31m(\x1B[1;36m" + bytes2Human(loaded) + "\x1B[1;31m/\x1B[1;36m" + bytes2Human(total) + "\x1B[1;31m) \x1B[1;32m" +  QString::number(percent) + "%\x1B[0m " << endl;
    cout.flush();
}

// shorten string to fit console size
QString sauger::fitMyString(QString text)
{
    int maxLen = _consolCOL - 35;

    if (text.length() <= maxLen)
    {
        return text;
    }

    int remainingChars = maxLen - 3;
    int leftHalfLength = remainingChars / 2;
    int rightHalfLength = remainingChars - leftHalfLength;

    QString leftHalf = text.left(leftHalfLength);
    QString rightHalf = text.right(rightHalfLength);

    return leftHalf + "..." + rightHalf;
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

    auto sfdlFile = new sfdl();
    auto thread = new QThread;

    connect(sfdlFile, SIGNAL(sendSFDLData(QStringList,QStringList)), this, SLOT(getSFDLData(QStringList,QStringList)));
    connect(sfdlFile, SIGNAL(sendLogText(QString)), this, SLOT(printConsol(QString)));

    sfdlFile->setSFDL(_SFDLFile, QStringList() << _SFDLPassword);
    sfdlFile->readSFDL();
    sfdlFile->moveToThread(thread);

    thread->start();
}

void sauger::sart(QString sfdlFileName)
{
    printConsol("-> " + sfdlFileName);

    auto sfdlFile = new sfdl();
    auto thread = new QThread;

    connect(sfdlFile, SIGNAL(sendSFDLData(QStringList,QStringList)), this, SLOT(getSFDLData(QStringList,QStringList)));
    connect(sfdlFile, SIGNAL(sendLogText(QString)), this, SLOT(printConsol(QString)));

    sfdlFile->setSFDL(sfdlFileName, QStringList() << _SFDLPassword);
    sfdlFile->readSFDL();
    sfdlFile->moveToThread(thread);

    thread->start();
}

QString sauger::seconds_to_DHMS(int duration)
{
    QString res;
    int seconds = (int) (duration % 60);
    duration /= 60;
    int minutes = (int) (duration % 60);
    duration /= 60;
    int hours = (int) (duration % 24);
    int days = (int) (duration / 24);
    if((hours == 0)&&(days == 0)&&(minutes == 0))
    {
        return res.sprintf("%02ds", seconds);
    }
    else if((hours == 0)&&(days == 0))
    {
        return res.sprintf("%02dm %02ds", minutes, seconds);
    }
    else if (days == 0)
    {
        return res.sprintf("%02dh %02dm %02ds", hours, minutes, seconds);
    }
    else
    {
        return res.sprintf("%ddt %02dh %02dm %02ds", days, hours, minutes, seconds);
    }
}

QString sauger::bytes2Human(float filesize)
{
    float num = filesize;

    QStringList list;
    list << "KiB" << "MiB" << "GiB" << "TiB";

    QStringListIterator i(list);
    QString unit("bytes");

    while(num >= 1024.0 && i.hasNext())
    {
        unit = i.next();
        num /= 1024.0;
    }
    return QString().setNum(num,'f',2) + " " + unit;
}

void sauger::clearConsole()
{
    std::cout << "\033[2J";
    std::cout << "\033[H";
    std::cout.flush();
}

QString sauger::returnProgressBar(int progress, int len)
{
    QString progressBar = "";
    int pBarLen = 0;
    int pBarPat = 0;

    if(progress > 0)
    {
        pBarLen = (progress * len) / 100;

        if(pBarLen > len)
        {
            pBarLen = len;
        }

        pBarPat = len - pBarLen;

        for(int i = 0; i < pBarLen; i++)
        {
            progressBar.append(QString::fromUtf8("█"));
        }

        if(pBarPat >= 1)
        {
            for(int j = 0; j < pBarPat; j++)
            {
                progressBar.append(QString::fromUtf8("░"));
            }
        }
    }
    else
    {
        for(int i = 0; i < len; i++)
        {
            progressBar.append(QString::fromUtf8("░"));
        }
    }

    return progressBar;
}

int sauger::returnPercent(qint64 read, qint64 total)
{
    int progress;

    if(total < 100)
    {
       progress = 100;
    }
    else
    {
        progress = read/(total/100);
    }

    if(progress > 100)
    {
        progress = 100;
    }

    return progress;
}

void sauger::printDownloadProgress()
{
    clearConsole();
    printBanner();
    // printGraf();

    for(auto i = Servers.begin(); i != Servers.end(); ++i)
    {
        if(i->status == 1)
        {
            printFileProgress(i->name, i->loaded, i->total, returnPercent(i->loaded, i->total));

            for(auto j = Files.begin(); j != Files.end(); j++)
            {
                if(j->status == 1 && j->dServerID == i->id)
                {
                    printFileProgress(j->fileName, j->loaded, j->total, returnPercent(j->loaded, j->total));
                }
            }
        }
    }
}

// get server index by id
int sauger::getServerIndexByID(int serverID)
{
    int c = 0;
    for(auto i = Servers.begin(); i != Servers.end(); i++)
    {
        if(i->id == serverID)
        {
            break;
        }
        c++;
    }

    return c;
}

// get file index by id
int sauger::getFileIndexByID(int fileID)
{
    int c = 0;
    for(auto i = Files.begin(); i != Files.end(); i++)
    {
        if(i->id == fileID)
        {
            break;
        }
        c++;
    }

    return c;
}

// return clean path and file from file full path
QStringList sauger::dirFromFilePath(QString filePath)
{
    QRegularExpression regex("[/\\\\]");
    QStringList pathParts = filePath.split(regex);
    QString fileName = pathParts.last();
    QString cleanPath = filePath.left(filePath.length() - fileName.length());

    QStringList result;
    result.append(cleanPath);
    result.append(fileName);

    return result;
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

            Files.append({FileID, id, filename, filepath, cleanpath, subDirs, 0, fileSize, 0, 0, 0, 0, true});
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
            qDebug() << var.selected;
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
        int port = data.at(8).split("|").at(1).toInt();
        QString user = data.at(9).split("|").at(1);
        QString pass = data.at(10).split("|").at(1);

        #ifdef QT_DEBUG
            qDebug() << "ServerID: " << ServerID;
            qDebug() << "sfdl: " << sfdl;
            qDebug() << "name: " << name;
            qDebug() << "path: " << path;
            qDebug() << "ip: " << ip;
            qDebug() << "port: " << port;
            qDebug() << "user: " << user;
            qDebug() << "pass: " << pass;
        #endif

        Servers.append({ServerID, sfdl, name, path, ip, port, user, pass, 0, 0, 0, 0, 0, 0});
    }

    if(!files.count())
    {
        // printConsol(tr("Loading file index from FTP server ..."));
        printStausText("SFDL", tr("Loading file index from FTP"), data.at(1).split("|").at(1));

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

        connect(listFTP, SIGNAL(sendLogText(QString,QString,QString)), this, SLOT(printStausText(QString,QString,QString)));
        connect(listFTP, SIGNAL(sendErrorMsg(QString)), this, SLOT(printError(QString)));

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
}

void sauger::startDownload()
{
    #ifdef QT_DEBUG
        qDebug() << "startDownload ... (" << _runningDownloads << "|" << _MaxDownloadThreads << ")";
    #endif

    if(_runningDownloads < _MaxDownloadThreads)
    {
        // get files to download
        for(auto i = Servers.begin(); i != Servers.end(); ++i)
        {
           if(i->status != 10) // 10 = all done
           {
               for(auto j = Files.begin(); j != Files.end(); ++j)
               {
                   if(j->dServerID == i->id)
                   {
                       if(j->status != 10 && j->status != 9 && j->status != 2)
                       {
                           if(_runningDownloads >= _MaxDownloadThreads)
                           {
                                #ifdef QT_DEBUG
                                    qDebug() << "break: " << _runningDownloads << _MaxDownloadThreads;
                                #endif

                               break;
                           }

                           QStringList downloadList;
                           downloadList.append(QString::number(i->id));   // server id
                           downloadList.append(QString::number(j->id));   // file id
                           downloadList.append(i->ip);                    // host
                           downloadList.append(QString::number(i->port)); // port
                           downloadList.append(i->user);                  // user
                           downloadList.append(i->pass);                  // pass

                           downloadList.append(dirFromFilePath(j->fullFilePath).at(0));  // ftp dir

                           // set local download path
                           QString fullDownloadPath = _DownloadDestination + i->name;
                           if(!fullDownloadPath.endsWith("/"))
                           {
                               fullDownloadPath = _DownloadDestination + i->name + "/";
                           }

                           QString fileSubPath = j->subDirs;

                           if(!fileSubPath.isEmpty())
                           {
                               if(fileSubPath.endsWith("/"))
                               {
                                  fullDownloadPath = fullDownloadPath + fileSubPath;
                               }
                               else
                               {
                                   fullDownloadPath = fullDownloadPath + fileSubPath + "/";
                               }
                           }

                           // create local download path (if needed)
                           QDir dir;
                           if(!dir.mkpath(fullDownloadPath))
                           {
                                #ifdef QT_DEBUG
                                    qDebug() << "error: can't create download path: " << fullDownloadPath;
                                #endif

                               break;
                           }

                           downloadList.append(fullDownloadPath);                        // download path
                           downloadList.append(dirFromFilePath(j->fullFilePath).at(1));  // file

                           auto thread = new QThread;
                           auto worker = new FTPDownload(downloadList);
                           worker->moveToThread(thread);
                           g_Worker.append(worker);

                           connect(worker, SIGNAL(doProgress(int,int,qint64,qint64,bool,bool)), this, SLOT(updateDownloadProgress(int,int,qint64,qint64,bool,bool)));
                           connect(worker, SIGNAL(statusUpdateFile(int,int)), this, SLOT(updateDownloadFileStatus(int,int)));

                           connect(worker, SIGNAL(error(QString)), this, SLOT(downloadError(QString)));

                           connect(thread, SIGNAL(started()), worker, SLOT(process()));
                           connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
                           connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
                           connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

                           thread->start();

                           Servers[getServerIndexByID(i->id)].status = 1;
                           Files[getFileIndexByID(j->id)].status = 1;

                           _runningDownloads++;

                           #ifdef QT_DEBUG
                               qDebug() << "new download thread for: " << j->fileName << _runningDownloads << _MaxDownloadThreads;
                           #endif

                           // start progress output timer
                           pProgress->start(1000);

                        }
                    }
                }
            }
        }
    }
}

// stop all downloads
void sauger::stopDownload()
{
    // stop progress output timer
    pProgress->stop();

    foreach(FTPDownload* w, g_Worker)
    {
        if(w->_working && w->thread()->isRunning())
        {
            updateDownloadFileStatus(w->fileID, 9); // fileID

            int serverIndex = getServerIndexByID(w->serverID);
            Servers[serverIndex].status = 9; // set server status

            w->ftp->abort();
            w->ftp->thread()->quit();
            w->ftp->thread()->wait();
            w->ftp->deleteLater();
            w->abort();
            w->thread()->quit();
            w->thread()->wait();
            w->deleteLater();

            _runningDownloads--;
        }
    }
}

void sauger::downloadError(QString error)
{
    printError("FTP: " + error);
}


// update progress on downloads
void sauger::updateDownloadProgress(int serverID, int fileID, qint64 read, qint64 total, bool overwriteTime, bool firstUpdate)
{
    overwriteTime;
    firstUpdate;

    // set file download progress
    int fileIndex = getFileIndexByID(fileID);
    // qint64 lastFileProgress = Files.at(fileIndex).progress;
    qint64 lastFileProgress = Files.at(fileIndex).loaded;
    qint64 fileProgressNew = read - lastFileProgress;

    // set precetage for progressbars
    Files[fileIndex].progress = returnPercent(read, total);

    if(fileProgressNew > 0)
    {
        // Files[fileIndex].progress += fileProgressNew;
        Files[fileIndex].loaded += fileProgressNew;
    }

    // set server download progress
    if(fileProgressNew > 0)
    {
        int serverIndex = getServerIndexByID(serverID);
        // qint64 serverProgress = Servers.at(serverIndex).progress;
        qint64 serverProgress = Servers.at(serverIndex).loaded;
        qint64 serverProgressNew = 0;

        serverProgressNew = serverProgress + fileProgressNew;
        // Servers[serverIndex].progress = serverProgressNew;
        Servers[serverIndex].loaded = serverProgressNew;

        // set precetage for progressbars
        Servers[serverIndex].progress = returnPercent(Servers[serverIndex].loaded, Servers[serverIndex].total);
    }
}

// update file status
void sauger::updateDownloadFileStatus(int fileID, int status)
{
    int fileIndex = getFileIndexByID(fileID);
    Files[fileIndex].status = status;

    #ifdef QT_DEBUG
        qDebug() << "updateDownloadFileStatus fileID: " << fileID;
        qDebug() << "updateDownloadFileStatus status: " << status;
        qDebug() << "updateDownloadFileStatus fileName: " << Files.at(fileIndex).fileName;
        qDebug() << "updateDownloadFileStatus _runningDownloads: " << _runningDownloads;
    #endif

    // if(status == 9 || status == 10)
    if(status > 1)
    {
        _runningDownloads--;

        #ifdef QT_DEBUG
            qDebug() << "updateDownloadFileStatus _runningDownloads-2: " << _runningDownloads;
        #endif

        startDownload();
    }
}

bool sauger::setData(QString sfdlFile, QString downloadPath, int maxThreads, bool forceOverwrite, QString password)
{
    int errors = 0;
    _SFDLFile = sfdlFile;

    if(!downloadPath.endsWith("/"))
    {
        downloadPath = downloadPath + "/";
    }

    _DownloadDestination = downloadPath;
    _MaxDownloadThreads = maxThreads;
    _runningDownloads = 0;
    _ForceOverwirteFiles = forceOverwrite;
    _SFDLPassword = password;

    if(_SFDLFile.isEmpty())
    {
        printError(tr("No SFDL file set!"));
        errors++;
    }
    else
    {
        QFile file(_SFDLFile);
        if(!file.exists())
        {
            printError(tr("SFDL file <") + _SFDLFile + tr("> does not exist!"));
            errors++;
        }
    }

    if(!_DownloadDestination.isEmpty())
    {
        QFileInfo dPath(_DownloadDestination);
        if(!dPath.exists())
        {
            printError(tr("Destination directory <") + dPath.absoluteFilePath() + tr("> does not exist!"));
            errors++;
        }

        if(!dPath.isDir())
        {
            printError(tr("Destination <") + dPath.absoluteFilePath() + tr("> is not a directory!"));
            errors++;
        }

        if(!dPath.isWritable())
        {
            printError(tr("Destination directory <") + dPath.absoluteFilePath() + tr("> is not writeable!"));
            errors++;
        }
    }
    else
    {
        printError(tr("Destination directory is not set!"));
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
        // exit(errors);
        return true;
    }

    return false;
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
