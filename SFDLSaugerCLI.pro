QT -= gui
QT += core network ftp xml websockets

CONFIG += c++14 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        sfdl.cpp \
        qaesencryption.cpp \
        ftpdownload.cpp \
        ftplistfiles.cpp \
        sauger.cpp \
        webserver.cpp \
        websocket.cpp \
        webserverthread.cpp

HEADERS += \
        sfdl.h \
        qaesencryption.h \
        ftpdownload.h \
        ftplistfiles.h \
        sauger.h \
        webserver.h \
        websocket.h \
        global.h \
        webserverthread.h

RESOURCES += \
        web.qrc

VERSION = 0.1.3.9

QMAKE_TARGET_COMPANY = "GrafSauger"
QMAKE_TARGET_PRODUCT = "SFDLSaugerCLI"
QMAKE_TARGET_DESCRIPTION = "SFDLSaugerCLI - SFDL Downloader [4] MLCBoard.com"
QMAKE_TARGET_COPYRIGHT = "2023 by GrafSauger"

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += APP_PRODUCT=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\"
DEFINES += APP_DESCRIPTION=\"\\\"$$QMAKE_TARGET_DESCRIPTION\\\"\"
DEFINES += APP_COPYRIGHT=\"\\\"$$QMAKE_TARGET_COPYRIGHT\\\"\"


