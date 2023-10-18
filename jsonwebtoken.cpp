#include "jsonwebtoken.h"

JsonWebToken::JsonWebToken(QObject *parent) : QObject(parent)
{

}

QByteArray JsonWebToken::base64UrlEncode(const QByteArray &input)
{
    QByteArray base64 = input.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    return base64;
}

QString JsonWebToken::createJWT(const QByteArray &secretKey, const QJsonObject &payload)
{
    // header
    QJsonObject header;
    header.insert("alg", "HS256");
    header.insert("typ", "JWT");

    QByteArray headerBytes = QJsonDocument(header).toJson(QJsonDocument::Compact);
    QByteArray encodedHeader = base64UrlEncode(headerBytes);

    // payload
    QByteArray payloadBytes = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QByteArray encodedPayload = base64UrlEncode(payloadBytes);

    // signatur
    QByteArray unsignedToken = encodedHeader + "." + encodedPayload;
    QByteArray signature = QMessageAuthenticationCode::hash(unsignedToken, secretKey, QCryptographicHash::Sha256).toBase64(QByteArray::Base64UrlEncoding);

    // JWT
    QString jwt = QString(encodedHeader) + "." + QString(encodedPayload) + "." + QString(signature);

    return jwt;
}

QString JsonWebToken::usernameFromToken(const QString &token)
{
    QStringList tokenParts = token.split('.');
    if(tokenParts.size() != 3)
    {
        return QString();
    }

    QByteArray payloadBase64 = QByteArray::fromBase64(tokenParts.at(1).toUtf8());
    QJsonDocument payloadJson = QJsonDocument::fromJson(payloadBase64);
    QJsonObject payload = payloadJson.object();

    if (!payload.contains("sub"))
    {
        return QString();
    }

    QString username = payload.value("sub").toString();

    return username;
}

bool JsonWebToken::isTokenExpired(const QString &token)
{
    QByteArray tokenData = QByteArray::fromBase64(token.toUtf8().split('.')[1]);
    QJsonDocument payloadJson = QJsonDocument::fromJson(tokenData);
    QJsonObject payload = payloadJson.object();

    if(payload.contains("exp"))
    {
        qint64 expirationTime = payload["exp"].toVariant().toLongLong();
        qint64 currentUnixTimestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
        return expirationTime <= currentUnixTimestamp;
    }

    return true;
}

QString JsonWebToken::createRefreshToken(const QByteArray &secretKey, const QJsonObject &payload)
{
    // header
    QJsonObject header;
    header.insert("alg", "HS256");
    header.insert("typ", "JWT");

    QByteArray headerBytes = QJsonDocument(header).toJson(QJsonDocument::Compact);
    QByteArray encodedHeader = base64UrlEncode(headerBytes);

    // payload
    QByteArray payloadBytes = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QByteArray encodedPayload = base64UrlEncode(payloadBytes);

    // signatur
    QByteArray unsignedToken = encodedHeader + "." + encodedPayload;
    QByteArray signature = QMessageAuthenticationCode::hash(unsignedToken, secretKey, QCryptographicHash::Sha256).toBase64(QByteArray::Base64UrlEncoding);

    // refresh token
    QString refreshToken = QString(encodedHeader) + "." + QString(encodedPayload) + "." + QString(signature);

    return refreshToken;
}

bool JsonWebToken::isRefreshTokenValid(const QString &refreshToken, const QByteArray &secretKey)
{
    return false;
}

QString JsonWebToken::renewAccessToken(const QString &refreshToken, const QByteArray &secretKey)
{
    if(isRefreshTokenValid(refreshToken, secretKey))
    {
    }

    return QString();
}

