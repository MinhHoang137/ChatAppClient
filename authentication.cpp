#include "authentication.h"
#include "winsockclient.h"
#include <QDebug>
#include <QJsonObject>

AuthenticationHandler::AuthenticationHandler(QObject *parent) : QObject(parent) {}

void AuthenticationHandler::registerUser(QString username, QString password)
{
    QJsonObject request;
    request["action"] = "register";
    request["username"] = username;
    request["password"] = password;
    WinSockClient::getInstance()->sendMessage(request);
}

void AuthenticationHandler::loginUser(QString username, QString password)
{
    QJsonObject request;
    request["action"] = "login";
    request["username"] = username;
    request["password"] = password;
    WinSockClient::getInstance()->sendMessage(request);
}

void handleRegisterResponse(const QJsonObject &response)
{
    bool success = response["success"].toBool();
    QString message = response["message"].toString();
    int userId = response["userId"].toInt();
    if (success) {
        qDebug() << "Registration successful. User ID:" << userId;
        WinSockClient::getInstance()->setUserId(userId);
    } else {
        qDebug() << "Registration failed:" << message;
    }
}

void handleLoginResponse(const QJsonObject &response)
{
    bool success = response["success"].toBool();
    QString message = response["message"].toString();
    int userId = response["userId"].toInt();

    if (success) {
        qDebug() << "Login successful. User ID:" << userId;
        WinSockClient::getInstance()->setUserId(userId);
    } else {
        qDebug() << "Login failed:" << message;
    }
}
