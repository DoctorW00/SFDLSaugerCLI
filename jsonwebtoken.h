#ifndef JSONWEBTOKEN_H
#define JSONWEBTOKEN_H

#include <QObject>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QVariant>
#include <QDateTime>

class JsonWebToken : public QObject
{
    Q_OBJECT

public:
    explicit JsonWebToken(QObject *parent = nullptr);

public slots:
    QString createJWT(const QByteArray &secretKey, const QJsonObject &payload);
    QString usernameFromToken(const QString &token);
    bool isTokenExpired(const QString &token);
    QString createRefreshToken(const QByteArray &secretKey, const QJsonObject &payload);
    QString renewAccessToken(const QString &refreshToken, const QByteArray &secretKey);

private slots:
    QByteArray base64UrlEncode(const QByteArray &input);
    bool isRefreshTokenValid(const QString &refreshToken, const QByteArray &secretKey);

};

#endif // JSONWEBTOKEN_H
