#pragma once

#include <QJsonObject>
#include <QString>

#ifdef _WIN32
#include <winsock2.h>
#endif

QJsonObject registerUser(QString username, QString password);
int sendRegisterRequest(SOCKET socket, QString username, QString password);
void handleRegisterResponse(const QJsonObject &response);
int sendLoginRequest(SOCKET socket, QString username, QString password);
QJsonObject loginUser(QString username, QString password);
void handleLoginResponse(const QJsonObject &response);
