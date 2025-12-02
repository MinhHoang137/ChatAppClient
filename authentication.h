#pragma once

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QQmlEngine>

void handleRegisterResponse(const QJsonObject &response);
void handleLoginResponse(const QJsonObject &response);

class AuthenticationHandler : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit AuthenticationHandler(QObject *parent = nullptr);

    Q_INVOKABLE void registerUser(QString username, QString password);
    Q_INVOKABLE void loginUser(QString username, QString password);
};
