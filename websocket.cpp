#include "websocket.h"
#include "sauger.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>

#include <QSslCertificate>
#include <QSslKey>

#include "global.h"

extern QVector<dServer> Servers;
extern QVector<dFile> Files;

websocket::websocket(QObject *parent) : QObject(parent)
{
    // register metatype socket error
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

    updateClients = new QTimer(this);
    // connect(updateClients, SIGNAL(timeout()), this, SLOT(sendTextMessage()));
    connect(updateClients, SIGNAL(timeout()), this, SLOT(sendTest())); // send test data to websocket clients
    updateClients->start(1000);

}

websocket::~websocket()
{
    server->close();
    qDeleteAll(_clients.begin(), _clients.end());
    delete server;
    delete socket;
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
    server = new QWebSocketServer(QStringLiteral(APP_PRODUCT " Websocket Server"), QWebSocketServer::SecureMode, this);

    QSslConfiguration sslConfiguration;

    const auto certs = QSslCertificate::fromPath(QLatin1String(":/cert/localhost.crt"));

    #ifdef QT_DEBUG
        qDebug() << "WS: Certs: " << certs;
    #endif

    if(certs.length())
    {
        m_sslLocalCertificate = certs.at(0);
    }
    sslConfiguration.setLocalCertificate(m_sslLocalCertificate);

    QFile keyFile(":/cert/localhost.decrypted.key");
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

    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    connect(server, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(serverError(QAbstractSocket::SocketError)));
    connect(server, SIGNAL(closed()), server, SLOT(deleteLater()));

    // if(server->listen(QHostAddress::Any, port))
    if(server->listen(QHostAddress("127.0.0.1"), port))
    {
        qDebug() << "WS server started @ port: " << port;
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
    s.insert("sfdl", "Spaceballs.1987.German.DL.DTSD.2160p.UHD.BluRay.x265-GSG9.sfdl");
    s.insert("name", "Spaceballs.1987.German.DL.DTSD.2160p.UHD.BluRay.x265-GSG9");
    s.insert("path", "/home/user/Spaceballs.1987.German.DL.DTSD.2160p.UHD.BluRay.x265-GSG9");
    s.insert("ip", "127.0.0.1");
    s.insert("port", 21);
    s.insert("user", "ftpUsername");
    s.insert("pass", "ftpPassword");
    s.insert("progress", 0);
    s.insert("total", 30000);
    s.insert("status", 0);
    s.insert("dateStart", 0);
    s.insert("dateStop", 0);
    s.insert("dateCancel", 0);
    servers.append(s);

    json["servers"] = servers;

    QJsonArray files;
    // QJsonArray f = {0, "test.rar", 1000};

    QJsonObject f1;
    f1.insert("id", 0);
    f1.insert("dServerID", 0);
    f1.insert("fileName", "spaceballs.rar");
    f1.insert("fullFilePath", "/downloads/movies/2160p/spaceballs.rar");
    f1.insert("cleanPath", "/downloads/movies/2160p");
    f1.insert("subDirs", "");
    f1.insert("progress", 0);
    f1.insert("total", 10000);
    f1.insert("status", 0);
    f1.insert("dateStart", 0);
    f1.insert("dateStop", 0);
    f1.insert("dateCancel", 0);
    files.append(f1);

    QJsonObject f2;
    f2.insert("id", 1);
    f2.insert("dServerID", 0);
    f2.insert("fileName", "spaceballs.r00");
    f2.insert("fullFilePath", "/downloads/movies/2160p/spaceballs.r00");
    f2.insert("cleanPath", "/downloads/movies/2160p");
    f2.insert("subDirs", "");
    f2.insert("progress", 0);
    f2.insert("total", 10000);
    f2.insert("status", 0);
    f2.insert("dateStart", 0);
    f2.insert("dateStop", 0);
    f2.insert("dateCancel", 0);
    files.append(f2);

    QJsonObject f3;
    f3.insert("id", 2);
    f3.insert("dServerID", 0);
    f3.insert("fileName", "spaceballs.r01");
    f3.insert("fullFilePath", "/downloads/movies/2160p/spaceballs.r01");
    f3.insert("cleanPath", "/downloads/movies/2160p");
    f3.insert("subDirs", "");
    f3.insert("progress", 0);
    f3.insert("total", 10000);
    f3.insert("status", 0);
    f3.insert("dateStart", 0);
    f3.insert("dateStop", 0);
    f3.insert("dateCancel", 0);
    files.append(f3);

    // files.append(f);
    json["files"] = files;

    QJsonDocument doc(json);
    QString jsonString(doc.toJson(QJsonDocument::Compact));

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
        f.insert("total", var.total);
        f.insert("status", var.status);
        f.insert("dateStart", var.dateStart);
        f.insert("dateStop", var.dateStop);
        f.insert("dateCancel", var.dateCancel);
        files.append(f);
    }
    json["files"] = files;

    QJsonDocument doc(json);
    QString jsonString(doc.toJson(QJsonDocument::Compact));

    for(int i = 0; i < _clients.count(); i++)
    {
        if(_clients.at(i)->state() == QAbstractSocket::ConnectedState)
        {
            qDebug() << "WS: Sending to client: " << i << jsonString;
            _clients.at(i)->sendTextMessage(jsonString);
        }
    }
}

void websocket::textMessageReceived(QString message)
{
    qDebug() << "WS: Text message received: " << message;

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

    client = qobject_cast<QWebSocket *>(sender());

    QString returnMessage = client->peerAddress().toString() + " > " + QString(message);

    client->sendTextMessage(returnMessage);
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

