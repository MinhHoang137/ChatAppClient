#include "friendhandlers.h"
#include <QDebug>
#include <QJsonArray>
#include "winsockclient.h"
#include "header.h"

FriendHandlers *FriendHandlers::m_instance = nullptr;

// Triển khai Singleton: Đảm bảo chỉ có 1 instance duy nhất
FriendHandlers *FriendHandlers::getInstance()
{
    if (m_instance == nullptr) {
        m_instance = new FriendHandlers();
    }
    return m_instance;
}

FriendHandlers::FriendHandlers(QObject *parent)
    : QObject(parent)
{
    // Kết nối signal từ WinSockClient tới slot của class này
    connect(WinSockClient::getInstance(),
            &WinSockClient::nonFriendUsersReceived,
            this,
            &FriendHandlers::onNonFriendUsersReceived);

    connect(WinSockClient::getInstance(),
            &WinSockClient::friendRequestsReceived,
            this,
            &FriendHandlers::onFriendRequestsReceived);

    connect(WinSockClient::getInstance(),
            &WinSockClient::friendsListReceived,
            this,
            &FriendHandlers::onFriendsReceived);

    connect(WinSockClient::getInstance(),
            &WinSockClient::friendStatusReceived,
            this,
            &FriendHandlers::onFriendStatusReceived);

    // Kết nối signal nội bộ (được emit từ WinSockClient qua lambda) tới slot xử lý tin nhắn
    connect(this, &FriendHandlers::messageReceived, this, &FriendHandlers::onMessageReceived);

        // Nhận danh sách nhóm
        connect(this, &FriendHandlers::groupsReceived, this, &FriendHandlers::onGroupsReceived);
        // Nhận tin nhắn nhóm
        connect(this, &FriendHandlers::groupMessageReceived, this, &FriendHandlers::onGroupMessageReceived);
        // Nhận danh sách thành viên nhóm
        connect(this, &FriendHandlers::groupMembersReceived, this, &FriendHandlers::onGroupMembersReceived);
}

// Gửi request lấy danh sách người lạ
void FriendHandlers::fetchNonFriendUsers()
{
    int userId = WinSockClient::getInstance()->getUserId();
    QJsonObject request;
    request["action"] = "getNonFriendUsers";
    request["userID"] = userId;
    WinSockClient::getInstance()->sendMessage(request);
}

// Gửi request lấy danh sách yêu cầu kết bạn
void FriendHandlers::fetchFriendRequests()
{
    int userId = WinSockClient::getInstance()->getUserId();
    QJsonObject request;
    request["action"] = "getFriendRequests";
    request["userID"] = userId;
    WinSockClient::getInstance()->sendMessage(request);
}

// Gửi request lấy danh sách bạn bè
void FriendHandlers::fetchFriends()
{
    int userId = WinSockClient::getInstance()->getUserId();
    QJsonObject request;
    request["action"] = "getFriendsList";
    request["userID"] = userId;
    WinSockClient::getInstance()->sendMessage(request);
}

// Gửi tin nhắn
void FriendHandlers::sendMessage(const QString &content)
{
    int senderID = WinSockClient::getInstance()->getUserId();
    int receiverID = WinSockClient::getInstance()->getTargetId();

    // Kiểm tra hợp lệ
    if (receiverID == 0 || content.isEmpty())
        return;

    // Tạo gói tin JSON
    QJsonObject request;
    request["action"] = "sendMessage";
    request["senderID"] = senderID;
    request["receiverID"] = receiverID;
    request["content"] = content;
    WinSockClient::getInstance()->sendMessage(request);

    // Optimistic update: Cập nhật giao diện ngay lập tức trước khi Server phản hồi
    // Giúp trải nghiệm người dùng mượt mà hơn
    QVariantMap map;
    map["senderID"] = senderID;
    map["receiverID"] = receiverID;
    map["content"] = content;
    m_messages.append(map);
    emit messagesChanged();
}

// Tải lịch sử tin nhắn
void FriendHandlers::loadMessages(int friendID)
{
    int userID = WinSockClient::getInstance()->getUserId();
    QJsonObject request;
    request["action"] = "getAllMessages";
    request["userID"] = userID;
    request["friendID"] = friendID;
    WinSockClient::getInstance()->sendMessage(request);
}

QVariantList FriendHandlers::nonFriendUsers() const
{
    return m_nonFriendUsers;
}

QVariantList FriendHandlers::friendRequests() const
{
    return m_friendRequests;
}

QVariantList FriendHandlers::friends() const
{
    return m_friends;
}

QVariantList FriendHandlers::messages() const
{
    return m_messages;
}

QVariantList FriendHandlers::groups() const
{
    return m_groups;
}

QVariantList FriendHandlers::groupMessages() const
{
    return m_groupMessages;
}

QVariantList FriendHandlers::groupMembers() const
{
    return m_groupMembers;
}

void FriendHandlers::sendFriendRequest(int toUserID)
{
    int fromUserID = WinSockClient::getInstance()->getUserId();
    QJsonObject request;
    request["action"] = "friendRequest";
    request["fromUserID"] = fromUserID;
    request["toUserID"] = toUserID;
    WinSockClient::getInstance()->sendMessage(request);

    // Optimistic update: Update local list to show "Sent"
    for (int i = 0; i < m_nonFriendUsers.size(); ++i) {
        QVariantMap map = m_nonFriendUsers[i].toMap();
        if (map["userID"].toInt() == toUserID) {
            map["sentRequest"] = true;
            m_nonFriendUsers[i] = map;
            emit nonFriendUsersChanged();
            break;
        }
    }

    // Optimistic update for currentFriendStatus
    if (WinSockClient::getInstance()->getTargetId() == toUserID) {
        m_currentFriendStatus = 0; // 0: Pending
        emit currentFriendStatusChanged();
    }
}

void FriendHandlers::sendQueryFriendStatus(int toUserID)
{
    int fromUserID = WinSockClient::getInstance()->getUserId();
    QJsonObject request;
    request["action"] = "queryFriendStatus";
    request["fromUserID"] = fromUserID;
    request["toUserID"] = toUserID;
    WinSockClient::getInstance()->sendMessage(request);
}

void FriendHandlers::sendAcceptFriendRequest(int fromUserID)
{
    int toUserID = WinSockClient::getInstance()->getUserId();
    QJsonObject request;
    request["action"] = "acceptFriendRequest";
    request["fromUserID"] = fromUserID;
    request["toUserID"] = toUserID;
    WinSockClient::getInstance()->sendMessage(request);

    // Remove from friend requests list locally
    for (int i = 0; i < m_friendRequests.size(); ++i) {
        QVariantMap map = m_friendRequests[i].toMap();
        if (map["userID"].toInt() == fromUserID) {
            m_friendRequests.removeAt(i);
            emit friendRequestsChanged();
            break;
        }
    }

    // Optimistic update for currentFriendStatus
    if (WinSockClient::getInstance()->getTargetId() == fromUserID) {
        m_currentFriendStatus = 1; // 1: Friend
        emit currentFriendStatusChanged();
    }
}

void FriendHandlers::sendGetFriendListRequest(int userID)
{
    QJsonObject request;
    request["action"] = "getFriendList";
    request["userID"] = userID;
    WinSockClient::getInstance()->sendMessage(request);
}

// Xử lý khi nhận danh sách người lạ
void FriendHandlers::onNonFriendUsersReceived(const QJsonObject &data)
{
    if (data["success"].toBool()) {
        m_nonFriendUsers.clear();
        QJsonArray users = data["users"].toArray();
        // Chuyển đổi JSON Array sang QVariantList để QML hiểu
        for (const auto &val : users) {
            QJsonObject user = val.toObject();
            QVariantMap userMap;
            userMap["userID"] = user["userID"].toInt();
            userMap["username"] = user["username"].toString();
            userMap["status"] = user["status"].toInt();            // 1 online, 0 offline
            userMap["sentRequest"] = user["sentRequest"].toBool(); // Check if request sent
            m_nonFriendUsers.append(userMap);
        }
        emit nonFriendUsersChanged();
    } else {
        qDebug() << "Failed to get non-friend users:" << data["message"].toString();
    }
}

void FriendHandlers::sendUnfriendRequest(int toUserID)
{
    int fromUserID = WinSockClient::getInstance()->getUserId();
    QJsonObject request;
    request["action"] = "unfriend";
    request["fromUserID"] = fromUserID;
    request["toUserID"] = toUserID;
    WinSockClient::getInstance()->sendMessage(request);

    // Optimistic update: Remove from friends list locally
    for (int i = 0; i < m_friends.size(); ++i) {
        QVariantMap map = m_friends[i].toMap();
        if (map["userID"].toInt() == toUserID) {
            m_friends.removeAt(i);
            emit friendsChanged();
            break;
        }
    }

    // Optimistic update for currentFriendStatus
    if (WinSockClient::getInstance()->getTargetId() == toUserID) {
        m_currentFriendStatus = -1; // -1: Not friends
        emit currentFriendStatusChanged();
    }
}

// --- Groups ---
void FriendHandlers::fetchGroups()
{
    int userId = WinSockClient::getInstance()->getUserId();
    QJsonObject request;
    request["action"] = "getUserGroups";
    request["userID"] = userId;
    WinSockClient::getInstance()->sendMessage(request);
}

void FriendHandlers::createGroup(const QString &groupName)
{
    int ownerId = WinSockClient::getInstance()->getUserId();
    QJsonObject request;
    request["action"] = "createGroup";
    request["groupName"] = groupName;
    request["ownerID"] = ownerId;
    WinSockClient::getInstance()->sendMessage(request);
}

void FriendHandlers::addUserToGroup(int groupID, int userID)
{
    QJsonObject request;
    request["action"] = "addUserToGroup";
    request["groupID"] = groupID;
    request["userID"] = userID;
    WinSockClient::getInstance()->sendMessage(request);
    // Refresh lists after change
    fetchGroups();
    if (WinSockClient::getInstance()->getGroupId() == groupID) {
        loadGroupMessages(groupID);
        loadGroupMembers(groupID);
    }
}

void FriendHandlers::removeUserFromGroup(int groupID, int userID)
{
    QJsonObject request;
    request["action"] = "removeUserFromGroup";
    request["groupID"] = groupID;
    request["userID"] = userID;
    WinSockClient::getInstance()->sendMessage(request);
    // Refresh lists after change
    fetchGroups();
    if (WinSockClient::getInstance()->getGroupId() == groupID) {
        loadGroupMessages(groupID);
        loadGroupMembers(groupID);
    }
}

void FriendHandlers::leaveGroup(int groupID)
{
    int userID = WinSockClient::getInstance()->getUserId();
    QJsonObject request;
    request["action"] = "leaveGroup";
    request["groupID"] = groupID;
    request["userID"] = userID;
    WinSockClient::getInstance()->sendMessage(request);
}

void FriendHandlers::sendGroupMessage(const QString &content)
{
    int senderID = WinSockClient::getInstance()->getUserId();
    int groupID = WinSockClient::getInstance()->getGroupId();
    if (senderID == 0 || groupID == 0 || content.isEmpty()) {
        qWarning() << "sendGroupMessage aborted: groupID=" << groupID << ", content empty=" << content.isEmpty();
        clientLogMessage(std::string("[GROUP_SEND_ABORT] userID=") + std::to_string(senderID) + ", groupID=" + std::to_string(groupID) + (content.isEmpty() ? " empty" : " ok"));
        return;
    }

    QJsonObject request;
    request["action"] = "sendGroupMessage";
    request["groupID"] = groupID;
    request["senderID"] = senderID;
    request["content"] = content;
    WinSockClient::getInstance()->sendMessage(request);

    // Trace outgoing group send
    clientLogMessage(std::string("[GROUP_SEND] groupID=") + std::to_string(groupID) + ", senderID=" + std::to_string(senderID) + ", content=" + content.toStdString());

    // Optimistic update
    QVariantMap map;
    map["senderID"] = senderID;
    map["groupID"] = groupID;
    map["content"] = content;
    m_groupMessages.append(map);
    emit groupMessagesChanged();
}

void FriendHandlers::loadGroupMessages(int groupID)
{
    QJsonObject request;
    request["action"] = "getGroupMessages";
    request["groupID"] = groupID;
    WinSockClient::getInstance()->sendMessage(request);
}

void FriendHandlers::loadGroupMembers(int groupID)
{
    QJsonObject request;
    request["action"] = "getGroupMembers";
    request["groupID"] = groupID;
    WinSockClient::getInstance()->sendMessage(request);
}

// Xử lý khi nhận danh sách yêu cầu kết bạn
void FriendHandlers::onFriendRequestsReceived(const QJsonObject &data)
{
    if (data["success"].toBool()) {
        m_friendRequests.clear();
        QJsonArray users = data["requests"].toArray();
        for (const auto &val : users) {
            QJsonObject user = val.toObject();
            QVariantMap userMap;
            userMap["userID"] = user["userID"].toInt();
            userMap["username"] = user["username"].toString();
            userMap["status"] = user["status"].toInt();
            m_friendRequests.append(userMap);
        }
        emit friendRequestsChanged();
    } else {
        qDebug() << "Failed to get friend requests:" << data["message"].toString();
    }
}

// Xử lý khi nhận danh sách bạn bè
void FriendHandlers::onFriendsReceived(const QJsonObject &data)
{
    if (data["success"].toBool()) {
        m_friends.clear();
        QJsonArray users = data["friends"].toArray();
        for (const auto &val : users) {
            QJsonObject user = val.toObject();
            QVariantMap userMap;
            userMap["userID"] = user["userID"].toInt();
            userMap["username"] = user["username"].toString();
            userMap["status"] = user["status"].toInt();
            m_friends.append(userMap);
        }
        emit friendsChanged();
    } else {
        qDebug() << "Failed to get friends list:" << data["message"].toString();
    }
}

// Xử lý khi nhận tin nhắn (lịch sử hoặc tin nhắn mới)
void FriendHandlers::onMessageReceived(const QJsonObject &data)
{
    QString action = data["action"].toString();
    qDebug() << "[FriendHandlers] onMessageReceived - action:" << action;

    // Trường hợp 1: Nhận toàn bộ lịch sử tin nhắn
    if (action == "getAllMessages") {
        if (data["success"].toBool()) {
            m_messages.clear();
            QJsonArray msgs = data["messages"].toArray();
            for (const auto &val : msgs) {
                QJsonObject obj = val.toObject();
                QVariantMap map;
                map["senderID"] = obj["senderID"].toInt();
                map["receiverID"] = obj["receiverID"].toInt();
                map["content"] = obj["content"].toString();
                map["sentAt"] = obj["sentAt"].toString();
                m_messages.append(map);
            }
            qDebug() << "[FriendHandlers] Loaded" << m_messages.size() << "direct messages";
            emit messagesChanged();
        }
    }
    // Trường hợp 2: Nhận một tin nhắn mới từ người khác
    else if (action == "receiveMessage") {
        int currentTarget = WinSockClient::getInstance()->getTargetId();
        int senderID = data["senderID"].toInt();
        qDebug() << "[FriendHandlers] receiveMessage - senderID:" << senderID << "currentTarget:" << currentTarget;

        // Chỉ hiển thị nếu tin nhắn đến từ người mình đang chat cùng
        if (senderID == currentTarget) {
            QVariantMap map;
            map["senderID"] = senderID;
            map["receiverID"] = WinSockClient::getInstance()->getUserId();
            map["content"] = data["content"].toString();
            m_messages.append(map);
            qDebug() << "[FriendHandlers] Appended direct message, total:" << m_messages.size();
            emit messagesChanged();
        }
    }
    // Trường hợp 3: Tin nhắn mình gửi thành công (echo back)
    else if (action == "sendMessage") {
        if (data["success"].toBool()) {
            qDebug() << "[FriendHandlers] sendMessage success";
        } else {
            qDebug() << "[FriendHandlers] sendMessage failed:" << data["message"].toString();
        }
    }
}

int FriendHandlers::currentFriendStatus() const
{
    return m_currentFriendStatus;
}

void FriendHandlers::onFriendStatusReceived(const QJsonObject &data)
{
    if (data["success"].toBool()) {
        int status = data["status"].toInt();
        if (m_currentFriendStatus != status) {
            m_currentFriendStatus = status;
            emit currentFriendStatusChanged();
        }
    } else {
        qDebug() << "Failed to query friend status:" << data["message"].toString();
    }
}

void FriendHandlers::onGroupsReceived(const QJsonObject &data)
{
    if (data["success"].toBool()) {
        m_groups.clear();
        QJsonArray groups = data["groups"].toArray();
        for (const auto &val : groups) {
            QJsonObject g = val.toObject();
            QVariantMap gm;
            gm["groupID"] = g["groupID"].toInt();
            gm["groupName"] = g["groupName"].toString();
            m_groups.append(gm);
        }
        emit groupsChanged();
    }
}

void FriendHandlers::onGroupMessageReceived(const QJsonObject &data)
{
    QString action = data["action"].toString();
    qDebug() << "[FriendHandlers] onGroupMessageReceived - action:" << action;
    
    // Group history
    if (action == "getGroupMessages") {
        if (data["success"].toBool()) {
            m_groupMessages.clear();
            QJsonArray msgs = data["messages"].toArray();
            for (const auto &val : msgs) {
                QJsonObject obj = val.toObject();
                QVariantMap map;
                map["senderID"] = obj["senderID"].toInt();
                map["senderName"] = obj["senderName"].toString();
                map["content"] = obj["content"].toString();
                map["sentAt"] = obj["sentAt"].toString();
                m_groupMessages.append(map);
            }
            qDebug() << "[FriendHandlers] Loaded" << m_groupMessages.size() << "group messages";
            emit groupMessagesChanged();
        }
    }
    // Own message sent successfully - already handled by optimistic update, just log
    else if (action == "sendGroupMessage") {
        if (data["success"].toBool()) {
            qDebug() << "[FriendHandlers] sendGroupMessage success (already optimistically updated)";
        } else {
            qDebug() << "[FriendHandlers] sendGroupMessage failed:" << data["message"].toString();
            // TODO: Could remove the optimistic update here on failure
        }
    }
    // Group incoming message from another user (real-time)
    else if (action == "receiveGroupMessage") {
        int currentGroup = WinSockClient::getInstance()->getGroupId();
        int groupID = data["groupID"].toInt();
        int senderID = data["senderID"].toInt();
        int myUserID = WinSockClient::getInstance()->getUserId();
        
        qDebug() << "[FriendHandlers] receiveGroupMessage - currentGroup:" << currentGroup 
                 << "msgGroupID:" << groupID << "senderID:" << senderID << "myUserID:" << myUserID;
        
        // Only show if it's the current group AND message is from someone else
        // (our own message is already shown via optimistic update)
        if (groupID == currentGroup && senderID != myUserID) {
            QVariantMap map;
            map["senderID"] = senderID;
            map["senderName"] = data["senderName"].toString();
            map["content"] = data["content"].toString();
            map["sentAt"] = data["sentAt"].toString();
            m_groupMessages.append(map);
            qDebug() << "[FriendHandlers] Appended group message from other user, total:" << m_groupMessages.size();
            emit groupMessagesChanged();
        }
    }
}

void FriendHandlers::onGroupMembersReceived(const QJsonObject &data)
{
    if (data["success"].toBool()) {
        m_groupMembers.clear();
        QJsonArray members = data["members"].toArray();
        for (const auto &val : members) {
            QJsonObject m = val.toObject();
            QVariantMap mm;
            mm["userID"] = m["userID"].toInt();
            mm["username"] = m["username"].toString();
            mm["status"] = m["status"].toInt();
            mm["joinedAt"] = m["joinedAt"].toString();
            m_groupMembers.append(mm);
        }
        emit groupMembersChanged();
    } else {
        qDebug() << "Failed to get group members:" << data["message"].toString();
    }
}
