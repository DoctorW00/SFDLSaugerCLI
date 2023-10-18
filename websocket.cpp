#include "websocket.h"
#include "sauger.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>

#include <QSslCertificate>
#include <QSslKey>

QMap<QWebSocket *, dConnectionInfo> connections;

websocket::websocket(QObject *parent) : QObject(parent)
{   
    // register metatype socket error
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

    JWT = new JsonWebToken(this);

    updateClients = new QTimer(this);
    connect(updateClients, SIGNAL(timeout()), this, SLOT(sendTextMessage()));
    // connect(updateClients, SIGNAL(timeout()), this, SLOT(sendTest())); // send test data to websocket clients
    updateClients->start(1000);

}

websocket::~websocket()
{
    server->close();
    qDeleteAll(_clients.begin(), _clients.end());
    delete server;
    delete socket;
}

void websocket::setHostIP(QString ip)
{
    hostIP = ip;
}

void websocket::setSSLUsage(bool option)
{
    sslUsage = option;
}

void websocket::setCerFile(QString location)
{
    sslCertFile = location;
}

void websocket::setKeyFile(QString location)
{
    sslKeyFile = location;
}

void websocket::start()
{
    qint16 port = 8870;
    start(port);
}

void websocket::start(qint16 port)
{
    if(!port && port < 65535)
    {
        port = 8870;
    }

    // server = new QWebSocketServer(QStringLiteral(APP_PRODUCT " Websocket Server"), QWebSocketServer::NonSecureMode, this);
    // server = new QWebSocketServer(QStringLiteral(APP_PRODUCT " Websocket Server"), QWebSocketServer::SecureMode, this);

    if(sslUsage == true)
    {
        if(sslCertFile.isEmpty())
        {
            sslCertFile = ":/cert/localhost.crt";
        }

        if(sslKeyFile.isEmpty())
        {
            sslKeyFile = ":/cert/localhost.decrypted.key";
        }

        server = new QWebSocketServer(QStringLiteral(APP_PRODUCT " Websocket Server"), QWebSocketServer::SecureMode, this);

        QSslConfiguration sslConfiguration;

        // const auto certs = QSslCertificate::fromPath(QLatin1String(":/cert/localhost.crt"));
        const auto certs = QSslCertificate::fromPath(QLatin1String(sslCertFile.toLatin1()));

        #ifdef QT_DEBUG
            qDebug() << "WS: Certs: " << certs;
        #endif

        if(certs.length())
        {
            m_sslLocalCertificate = certs.at(0);
        }
        sslConfiguration.setLocalCertificate(m_sslLocalCertificate);

        // QFile keyFile(":/cert/localhost.decrypted.key");
        QFile keyFile(sslKeyFile);
        if(keyFile.open(QIODevice::ReadOnly))
        {
            QString keyFileData = keyFile.readAll();
            QByteArray keydata = keyFileData.toUtf8();

            QSslKey key(keydata, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
            m_sslPrivateKey = key;

            keyFile.close();

            #ifdef QT_DEBUG
                qDebug() << "WS: SSL Key: " << m_sslPrivateKey;
            #endif
        }
        else
        {
            qDebug() << "WS: Can't open SSL KeyFile: " << keyFile.fileName();
        }

        sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
        sslConfiguration.setPrivateKey(m_sslPrivateKey);
        sslConfiguration.setProtocol(QSsl::TlsV1_2);

        server->setSslConfiguration(sslConfiguration);
    }
    else // no ssl
    {
        server = new QWebSocketServer(QStringLiteral(APP_PRODUCT " Websocket Server"), QWebSocketServer::NonSecureMode, this);
    }

    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    connect(server, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(serverError(QAbstractSocket::SocketError)));
    connect(server, SIGNAL(closed()), server, SLOT(deleteLater()));

    // if(server->listen(QHostAddress::Any, port))
    // if(server->listen(QHostAddress("127.0.0.1"), port))
    if(server->listen(QHostAddress(hostIP), port))
    {
        qDebug() << "WS server started " << hostIP << ":" << port;
    }
    else
    {
        qDebug() << "Error: WS server did not start: " << server->errorString();
    }
}

void websocket::newConnection()
{
    socket = server->nextPendingConnection();

    qDebug() << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
    qDebug() << "WS: New client: " << QHostAddress(socket->peerAddress().toIPv4Address()).toString();

    connect(socket, SIGNAL(textMessageReceived(QString)), this, SLOT(textMessageReceived(QString)));
    connect(socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(binaryMessageReceived(QByteArray)));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(serverError(QAbstractSocket::SocketError)));

    _clients << socket;
}

void websocket::sendTest()
{
    QJsonObject json;

    QJsonArray servers;
    // QJsonArray s = {0, "test download", 1000};

    QJsonObject s;
    s.insert("id", 0);
    // s.insert("sfdl", "Spaceballs.1987.German.DL.DTSD.2160p.UHD.BluRay.x265-GSG9.sfdl");
    s.insert("name", "Spaceballs.1987.German.DL.DTSD.2160p.UHD.BluRay.x265-GSG9");
    // s.insert("path", "/home/user/Spaceballs.1987.German.DL.DTSD.2160p.UHD.BluRay.x265-GSG9");
    // s.insert("ip", "127.0.0.1");
    // s.insert("port", 21);
    // s.insert("user", "ftpUsername");
    // s.insert("pass", "ftpPassword");
    s.insert("progress", 57);
    s.insert("loaded", 17000);
    s.insert("total", 30000);
    s.insert("status", 1);
    // s.insert("dateStart", 0);
    // s.insert("dateStop", 0);
    // s.insert("dateCancel", 0);
    servers.append(s);

    json["servers"] = servers;

    QJsonArray files;
    // QJsonArray f = {0, "test.rar", 1000};

    QJsonObject f1;
    f1.insert("id", 0);
    f1.insert("dServerID", 0);
    f1.insert("fileName", "spaceballs.rar");
    // f1.insert("fullFilePath", "/downloads/movies/2160p/spaceballs.rar");
    // f1.insert("cleanPath", "/downloads/movies/2160p");
    // f1.insert("subDirs", "");
    f1.insert("progress", 30);
    f1.insert("loaded", 3000);
    f1.insert("total", 10000);
    f1.insert("status", 1);
    // f1.insert("dateStart", 0);
    // f1.insert("dateStop", 0);
    // f1.insert("dateCancel", 0);
    f1.insert("selected", true);
    files.append(f1);

    QJsonObject f2;
    f2.insert("id", 1);
    f2.insert("dServerID", 0);
    f2.insert("fileName", "spaceballs.r00");
    // f2.insert("fullFilePath", "/downloads/movies/2160p/spaceballs.r00");
    // f2.insert("cleanPath", "/downloads/movies/2160p");
    // f2.insert("subDirs", "");
    f2.insert("progress", 60);
    f2.insert("loaded", 6000);
    f2.insert("total", 10000);
    f2.insert("status", 1);
    // f2.insert("dateStart", 0);
    // f2.insert("dateStop", 0);
    // f2.insert("dateCancel", 0);
    f2.insert("selected", true);
    files.append(f2);

    QJsonObject f3;
    f3.insert("id", 2);
    f3.insert("dServerID", 0);
    f3.insert("fileName", "spaceballs.r01");
    // f3.insert("fullFilePath", "/downloads/movies/2160p/spaceballs.r01");
    // f3.insert("cleanPath", "/downloads/movies/2160p");
    // f3.insert("subDirs", "");
    f3.insert("progress", 80);
    f3.insert("loaded", 8000);
    f3.insert("total", 10000);
    f3.insert("status", 1);
    // f3.insert("dateStart", 0);
    // f3.insert("dateStop", 0);
    // f3.insert("dateCancel", 0);
    f3.insert("selected", true);
    files.append(f3);

    // 2nd test
    QJsonObject s2;
    s2.insert("id", 1);
    s2.insert("name", "Friedhof.der.Kuscheltiere.Bloodlines.2023.GERMAN.DL.1080p.WEB.H264.iNTERNAL-MGE");
    s2.insert("progress", 30);
    s2.insert("loaded", 3000);
    s2.insert("total", 10000);
    s2.insert("status", 1);
    servers.append(s2);

    json["servers"] = servers;

    QJsonObject f4;
    f4.insert("id", 3);
    f4.insert("dServerID", 1);
    f4.insert("fileName", "kuschel.rar");
    f4.insert("progress", 10);
    f4.insert("loaded", 1000);
    f4.insert("total", 10000);
    f4.insert("status", 1);
    f4.insert("selected", true);
    files.append(f4);

    QJsonObject f5;
    f5.insert("id", 4);
    f5.insert("dServerID", 1);
    f5.insert("fileName", "kuschel.r00");
    f5.insert("progress", 20);
    f5.insert("loaded", 2000);
    f5.insert("total", 10000);
    f5.insert("status", 1);
    f5.insert("selected", true);
    files.append(f5);

    // files.append(f);
    json["files"] = files;

    QJsonDocument doc(json);

    #ifdef QT_DEBUG
        QString jsonString(doc.toJson(QJsonDocument::Indented)); // send human readable output on debug builds
    #else
        QString jsonString(doc.toJson(QJsonDocument::Compact));
    #endif

    for(int i = 0; i < _clients.count(); i++)
    {
        if(_clients.at(i)->state() == QAbstractSocket::ConnectedState)
        {
            qDebug() << "WS: Sending to client: " << i << jsonString;
            _clients.at(i)->sendTextMessage(jsonString);
        }
    }
}

void websocket::sendTextMessage()
{  
    qDebug() << "WS: Sending text message to all clients ...";

    QJsonObject json;

    QJsonArray servers;
    foreach(dServer var, Servers)
    {
        // QJsonArray s = {var.id, var.name, var.total};

        QJsonObject s;
        s.insert("id", var.id);
        s.insert("sfdl", var.sfdl);
        s.insert("name", var.name);
        s.insert("path", var.path);
        s.insert("ip", var.ip);
        s.insert("port", var.port);
        s.insert("user", var.user);
        s.insert("pass", var.pass);
        s.insert("progress", var.progress);
        s.insert("loaded", var.loaded);
        s.insert("total", var.total);
        s.insert("status", var.status);
        s.insert("dateStart", var.dateStart);
        s.insert("dateStop", var.dateStop);
        s.insert("dateCancel", var.dateCancel);
        servers.append(s);
    }
    json["servers"] = servers;

    QJsonArray files;
    foreach(dFile var, Files)
    {
        // QJsonArray f = {var.id, var.dServerID, var.fileName, var.total};

        QJsonObject f;
        f.insert("id", var.id);
        f.insert("dServerID", var.dServerID);
        f.insert("fileName", var.fileName);
        f.insert("fullFilePath", var.fullFilePath);
        f.insert("cleanPath", var.cleanPath);
        f.insert("subDirs", var.subDirs);
        f.insert("progress", var.progress);
        f.insert("loaded", var.loaded);
        f.insert("total", var.total);
        f.insert("status", var.status);
        f.insert("dateStart", var.dateStart);
        f.insert("dateStop", var.dateStop);
        f.insert("dateCancel", var.dateCancel);
        f.insert("selected", var.selected);
        files.append(f);
    }
    json["files"] = files;

    QJsonDocument doc(json);

    #ifdef QT_DEBUG
        QString jsonString(doc.toJson(QJsonDocument::Indented)); // send human readable output on debug builds
    #else
        QString jsonString(doc.toJson(QJsonDocument::Compact));
    #endif

    for(int i = 0; i < _clients.count(); i++)
    {
        if(_clients.at(i)->state() == QAbstractSocket::ConnectedState)
        {
            qDebug() << "WS: Sending to client: " << i << jsonString;
            _clients.at(i)->sendTextMessage(jsonString);
        }
    }
}

bool websocket::isValidJson(const QString& jsonString)
{
    QJsonParseError error;
    QJsonDocument::fromJson(jsonString.toUtf8(), &error);

    return (error.error == QJsonParseError::NoError);
}

QString websocket::doLogin(QString username)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    qint64 unixTimestamp = currentDateTime.toSecsSinceEpoch();

    // create JWT
    QByteArray secretKey = "1234567890";

    QJsonObject payload;
    payload.insert("sub", username);
    payload.insert("iat", unixTimestamp);
    payload.insert("exp", unixTimestamp + 3600);
    QString jwToken = JWT->createJWT(secretKey, payload);

    QJsonObject payload2;
    payload2.insert("sub", username);
    payload2.insert("iat", unixTimestamp);
    payload2.insert("exp", unixTimestamp + 31536000);
    QString refrehToken = JWT->createRefreshToken(secretKey, payload2);

    QJsonObject json;
    QJsonArray files;
    QJsonObject f;
    f.insert("authorized", true);
    f.insert("username", username);
    f.insert("token", jwToken);
    f.insert("refrehToken", refrehToken);
    files.append(f);
    json["login"] = files;
    QJsonDocument doc(json);
    QString jsonString(doc.toJson(QJsonDocument::Compact));

    return jsonString;
}

void websocket::doLogOut(QString token)
{
    for(auto it = connections.begin(); it != connections.end(); it++)
    {
        if(it.value().token == token)
        {
            connections.erase(it);
        }
    }
}

void websocket::textMessageReceived(QString message)
{
    client = qobject_cast<QWebSocket *>(sender());

    qDebug() << "WS: Text message received: " << message << client->peerAddress().toString() << client->peerPort();

    if(isValidJson(message))
    {
        QJsonDocument parsedDocument = QJsonDocument::fromJson(message.toUtf8());
        QJsonObject parsedObject = parsedDocument.object();

        QString token = parsedObject["token"].toString();

        // check if user is (still) logged in
        if(!token.isEmpty())
        {
            QString token = parsedObject["token"].toString();
            QString refreshToken = parsedObject["refreshToken"].toString();

            if(!token.isEmpty(), !refreshToken.isEmpty())
            {
                if(JWT->isTokenExpired(token))
                {
                    // not (yet) very secure
                    if(JWT->isTokenExpired(refreshToken))
                    {
                        // log out
                        doLogOut(token);
                    }
                    else
                    {
                        // refresh token still good
                        // do login
                        QString username = JWT->usernameFromToken(token);
                        QString jsonString = doLogin(username);

                        if(!username.isEmpty())
                        {
                            dConnectionInfo info;
                            info.username = username;
                            info.token = token;
                            connections.insert(socket, info);
                            socket->sendTextMessage("Authenticated as: " + username);
                        }
                        else
                        {
                            socket->sendTextMessage("Authentication failed!");
                            socket->close();
                        }

                        if(!jsonString.isEmpty())
                        {
                            client->sendTextMessage(jsonString);

                            if(socket->flush())
                            {
                                return;
                            }
                        }
                    }
                }
                else
                {
                    /*
                    QString username = JWT->usernameFromToken(token);

                    if(!username.isEmpty())
                    {
                        dConnectionInfo info;
                        info.username = username;
                        info.token = token;
                        connections.insert(socket, info);
                        socket->sendTextMessage("Authenticated as: " + username);
                    }
                    else
                    {
                        socket->sendTextMessage("Authentication failed");
                        socket->close();
                    }
                    */
                }
            }
        }


        QString action = parsedObject["action"].toString();

        if(action == "login")
        {
            qDebug() << "WS: Login ... " << client->peerAddress().toString();

            QString username = parsedObject["username"].toString();
            QString password = parsedObject["password"].toString();

            if(!username.isEmpty(), !password.isEmpty())
            {
                QString jsonString = doLogin(username);

                if(!username.isEmpty())
                {
                    dConnectionInfo info;
                    info.username = username;
                    info.token = token;
                    connections.insert(socket, info);
                    socket->sendTextMessage("Authenticated as: " + username);
                }
                else
                {
                    socket->sendTextMessage("Authentication failed!");
                    socket->close();
                }

                if(!jsonString.isEmpty())
                {
                    client->sendTextMessage(jsonString);

                    if(socket->flush())
                    {
                        return;
                    }
                }
            }
        }

        // write sfdl file to server location
        if(action == "send-sfdl")
        {
            QByteArray sfdlData = parsedObject["xml"].toString().toUtf8();

            if(!sfdlData.isEmpty() && !parsedObject["filename"].toString().isEmpty())
            {
                QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
                if(!tempPath.endsWith("/"))
                {
                    tempPath.append("/");
                }

                QString sfdlPath = tempPath + parsedObject["filename"].toString();
                QFile file(sfdlPath);
                if(file.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    file.write(sfdlData);
                    file.close();

                    qDebug() << "OK! SFDL stored @ " << sfdlPath;

                    auto s = new sauger;
                    s->setData(sfdlPath, "/", 3, true, "mlcboard.com");
                    s->sart();
                }
                else
                {
                    qDebug() << "Error writing SFDL: " << file.errorString();
                }
            }
        }

        // (un)select file(s) for download
        if(action == "check")
        {
            QJsonDocument parsedDocument = QJsonDocument::fromJson(message.toUtf8());
            QJsonObject parsedObject = parsedDocument.object();

            int fileID = parsedObject["id"].toInt();
            for(int f = 0; f < Files.count(); f++)
            {
                if(Files[f].id == fileID)
                {
                    Files[f].selected = true;
                }
            }
        }

        if(action == "uncheck")
        {
            QJsonDocument parsedDocument = QJsonDocument::fromJson(message.toUtf8());
            QJsonObject parsedObject = parsedDocument.object();

            int fileID = parsedObject["id"].toInt();
            for(int f = 0; f < Files.count(); f++)
            {
                if(Files[f].id == fileID)
                {
                    Files[f].selected = false;
                }
            }
        }

        if(action == "stop")
        {
            qDebug() << "Stopping downloads!";

            auto s = new sauger;
            s->stopDownload();
        }

        if(action == "start")
        {
            qDebug() << "Starting downloads!";

            auto s = new sauger;
            s->startDownload();
        }


    }

    if(message == "!status")
    {
        sendTextMessage();
    }

    if(message.startsWith("!sfdl"))
    {
        QString sfdl = message.split("!sfdl ").at(1);

        auto s = new sauger;
        connect(s, SIGNAL(createJson()), this, SLOT(sendTextMessage(QString)));

        s->setData(sfdl, "/", 3, true, "mlcboard.com");
        s->sart();
    }

    // echo server
    QString returnMessage = client->peerAddress().toString() + " > " + QString(message);
    client->sendTextMessage(returnMessage);

    // close app
    if(message == "shutdown")
    {
        client->sendTextMessage("Bye bye!");

        if(socket->flush())
        {
            qApp->exit(0);
        }
    }
}

void websocket::binaryMessageReceived(QByteArray message)
{
    qDebug() << "WS: binary message received: " << message;

    client = qobject_cast<QWebSocket *>(sender());
    client->sendBinaryMessage(message);
}

void websocket::bytesWritten(qint64 bytes)
{
    qDebug() << "WS: " << bytes << " bytes written!";
}

void websocket::serverError(QAbstractSocket::SocketError error)
{
    qDebug() << "WS socket error: " << error;
}

void websocket::disconnected()
{
    qDebug() << "WS: disconnected!";
    qDebug() << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";

    _clients.removeAll(client);
}

