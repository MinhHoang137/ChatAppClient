#include "friendhandlers.h"
#include "winsockclient.h"
#include <QDebug>
#include <QJsonArray>

// Hàm xử lý phản hồi (callback) cũ, hiện tại có thể không dùng đến vì đã chuyển sang dùng Signal/Slot
void handleGetNonFriendUsers(const QJsonObject &response)
{
    qDebug() << "Non-friend users received:" << response;
}

FriendHandlers* FriendHandlers::m_instance = nullptr;

// Triển khai Singleton: Đảm bảo chỉ có 1 instance duy nhất
FriendHandlers* FriendHandlers::getInstance()
{
    if (m_instance == nullptr) {
        m_instance = new FriendHandlers();
    }
    return m_instance;
}

FriendHandlers::FriendHandlers(QObject *parent) : QObject(parent)
{
    // Kết nối signal từ WinSockClient tới slot của class này
    connect(WinSockClient::getInstance(), &WinSockClient::nonFriendUsersReceived,
            this, &FriendHandlers::onNonFriendUsersReceived);
    // Kết nối signal nội bộ (được emit từ WinSockClient qua lambda) tới slot xử lý tin nhắn
    connect(this, &FriendHandlers::messageReceived, this, &FriendHandlers::onMessageReceived);
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

// Gửi tin nhắn
void FriendHandlers::sendMessage(const QString &content)
{
    int senderID = WinSockClient::getInstance()->getUserId();
    int receiverID = WinSockClient::getInstance()->getTargetId();
    
    // Kiểm tra hợp lệ
    if (receiverID == 0 || content.isEmpty()) return;

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

QVariantList FriendHandlers::messages() const
{
    return m_messages;
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
            userMap["status"] = user["status"].toInt(); // 1 online, 0 offline
            m_nonFriendUsers.append(userMap);
        }
        emit nonFriendUsersChanged();
    } else {
        qDebug() << "Failed to get non-friend users:" << data["message"].toString();
    }
}

// Xử lý khi nhận tin nhắn (lịch sử hoặc tin nhắn mới)
void FriendHandlers::onMessageReceived(const QJsonObject &data)
{
    QString action = data["action"].toString();
    
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
            emit messagesChanged();
        }
    } 
    // Trường hợp 2: Nhận một tin nhắn mới từ người khác
    else if (action == "receiveMessage") {
        int currentTarget = WinSockClient::getInstance()->getTargetId();
        int senderID = data["senderID"].toInt();
        
        // Chỉ hiển thị nếu tin nhắn đến từ người mình đang chat cùng
        if (senderID == currentTarget) {
             QVariantMap map;
             map["senderID"] = senderID;
             map["receiverID"] = WinSockClient::getInstance()->getUserId();
             map["content"] = data["content"].toString();
             m_messages.append(map);
             emit messagesChanged();
        }
    }
}

void FriendHandlers::onGetAllMessagesReceived(const QJsonObject &data)
{
    // Handled in onMessageReceived
}


