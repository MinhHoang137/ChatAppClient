#pragma once

#include <QString>
#include <QJsonObject>

#ifdef _WIN32
#include <winsock2.h>
#endif

QJsonObject registerUser(QString username, QString password);
int sendRegisterRequest(SOCKET socket, QString username, QString password);
void handleRegisterResponse(const QJsonObject &response);
