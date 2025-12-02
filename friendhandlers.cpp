#include "friendhandlers.h"
#include "winsockclient.h"
#include <QDebug>
#include <QJsonArray>

FriendHandlers::FriendHandlers(QObject *parent) : QObject(parent)
{
    connect(WinSockClient::getInstance(), &WinSockClient::nonFriendUsersReceived,
            this, &FriendHandlers::onNonFriendUsersReceived);
}

void FriendHandlers::fetchNonFriendUsers()
{
    int userId = WinSockClient::getInstance()->getUserId();
    QJsonObject request;
    request["action"] = "getNonFriendUsers";
    request["userID"] = userId;
    WinSockClient::getInstance()->sendMessage(request);
}

QVariantList FriendHandlers::nonFriendUsers() const
{
    return m_nonFriendUsers;
}

void FriendHandlers::onNonFriendUsersReceived(const QJsonObject &data)
{
    if (data["success"].toBool()) {
        m_nonFriendUsers.clear();
        QJsonArray users = data["users"].toArray();
        for (const auto &val : users) {
            QJsonObject user = val.toObject();
            QVariantMap userMap;
            userMap["userID"] = user["userID"].toInt();
            userMap["username"] = user["username"].toString();
            userMap["status"] = user["status"].toInt(); // 1 online, 0 offline
            m_nonFriendUsers.append(userMap);
        }
        emit nonFriendUsersChanged();
    } else {
        qDebug() << "Failed to get non-friend users:" << data["message"].toString();
    }
}