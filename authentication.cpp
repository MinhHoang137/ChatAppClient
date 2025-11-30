#include "authentication.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <winsock2.h>

QJsonObject registerUser(QString username, QString password) {
    // Implementation of register function
    return {{"action", "register"},
            {"username", username},
            {"password", password}};
}

int sendRegisterRequest(SOCKET socket, QString username, QString password) {
    QJsonObject request = registerUser(username, password);
    QJsonDocument doc(request);
    QByteArray byteArray = doc.toJson(QJsonDocument::Compact);
    std::string jsonString = byteArray.toStdString();
    return send(socket, jsonString.c_str(), (int)jsonString.length(), 0);
}

void handleRegisterResponse(const QJsonObject &response) {
    bool success = response["success"].toBool();
    QString message = response["message"].toString();
    int userId = response["userId"].toInt();

    if (success) {
        qDebug() << "Registration successful. User ID:" << userId;
    } else {
        qDebug() << "Registration failed:" << message;
    }
}

