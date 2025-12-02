#include "authentication.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <winsock2.h>
#include "winsockclient.h"

QJsonObject registerUser(QString username, QString password)
{
    // Implementation of register function
    return {{"action", "register"}, {"username", username}, {"password", password}};
}

int sendRegisterRequest(SOCKET socket, QString username, QString password)
{
    return WinSockClient::getInstance()->sendMessage(registerUser(username, password));
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

QJsonObject loginUser(QString username, QString password)
{
    // Implementation of login function
    return {{"action", "login"}, {"username", username}, {"password", password}};
}

int sendLoginRequest(SOCKET socket, QString username, QString password)
{
    return WinSockClient::getInstance()->sendMessage(loginUser(username, password));
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