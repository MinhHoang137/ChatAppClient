#include "authentication.h"
#include <QDebug>
#include <QJsonObject>
#include "winsockclient.h"

AuthenticationHandler::AuthenticationHandler(QObject *parent)
    : QObject(parent)
{}

// Gửi yêu cầu đăng ký
void AuthenticationHandler::registerUser(QString username, QString password)
{
    QJsonObject request;
    request["action"] = "register";
    request["username"] = username;
    request["password"] = password;
    WinSockClient::getInstance()->sendMessage(request);
}

// Gửi yêu cầu đăng nhập
void AuthenticationHandler::loginUser(QString username, QString password)
{
    QJsonObject request;
    request["action"] = "login";
    request["username"] = username;
    request["password"] = password;
    WinSockClient::getInstance()->sendMessage(request);
}

// Xử lý phản hồi đăng ký từ Server
void handleRegisterResponse(const QJsonObject &response)
{
    bool success = response["success"].toBool();
    QString message = response["message"].toString();
    int userId = response["userId"].toInt();
    if (success) {
        qDebug() << "Registration successful. User ID:" << userId;
        // Lưu User ID vào WinSockClient để sử dụng sau này
        WinSockClient::getInstance()->setUserId(userId);
    } else {
        qDebug() << "Registration failed:" << message;
    }
}

// Xử lý phản hồi đăng nhập từ Server
void handleLoginResponse(const QJsonObject &response)
{
    bool success = response["success"].toBool();
    QString message = response["message"].toString();
    int userId = response["userId"].toInt();

    if (success) {
        qDebug() << "Login successful. User ID:" << userId;
        // Lưu User ID vào WinSockClient để sử dụng sau này
        WinSockClient::getInstance()->setUserId(userId);
    } else {
        qDebug() << "Login failed:" << message;
    }
}
