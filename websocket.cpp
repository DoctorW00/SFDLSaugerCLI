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

    /*
    updateClients = new QTimer(this);
    connect(updateClients, SIGNAL(timeout()), this, SLOT(sendTextMessage()));
    updateClients->start(1000);
    */
}

websocket::~websocket()
{
    server->close();
    qDeleteAll(_clients.begin(), _clients.end());
    delete server;
    delete socket;
    delete client;
}

void websocket::start()
{
    qint16 port = 8870;
    start(port);
}

void websocket::start(qint16 port)
{
    if(!port)
    {
        port = 8870;
    }

    server = new QWebSocketServer(QStringLiteral(APP_PRODUCT " Websocket Server"), QWebSocketServer::NonSecureMode, this);

    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    connect(server, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(serverError(QAbstractSocket::SocketError)));
    connect(server, SIGNAL(closed()), server, SLOT(deleteLater()));

    if(server->listen(QHostAddress::Any, port))
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

    connect(socket, SIGNAL(textMessageReceived(QString)), this, SLOT(textMessageReceived(QString)));
    connect(socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(binaryMessageReceived(QByteArray)));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(serverError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

    _clients << socket;
}

// void websocket::sendTextMessage(QString jsonString)
void websocket::sendTextMessage()
{
    qDebug() << "WS: Sending text message to all clients ...";

    // QString dt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    /*
    QJsonObject json;
    json["main"] = "date & time: " + dt;
    QJsonDocument doc(json);
    QString jsonString(doc.toJson(QJsonDocument::Compact));
    */

    /*
    auto s = new sauger;
    QString jsonString = s->createJson();
    */

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

