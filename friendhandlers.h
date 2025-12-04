#ifndef FRIENDHANDLERS_H
#define FRIENDHANDLERS_H

#include <QObject>
#include <QJsonObject>
#include <QVariantList>
#include <QQmlEngine>

/**
 * @brief Lớp xử lý các logic liên quan đến bạn bè và tin nhắn.
 * Kế thừa từ QObject để có thể sử dụng Signals & Slots và tích hợp với QML.
 * Thiết kế theo mẫu Singleton để dễ dàng truy cập từ mọi nơi.
 */
class FriendHandlers : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    // Property để binding danh sách người lạ lên QML
    Q_PROPERTY(QVariantList nonFriendUsers READ nonFriendUsers NOTIFY nonFriendUsersChanged)
    // Property để binding danh sách yêu cầu kết bạn lên QML
    Q_PROPERTY(QVariantList friendRequests READ friendRequests NOTIFY friendRequestsChanged)
    // Property để binding danh sách bạn bè lên QML
    Q_PROPERTY(QVariantList friends READ friends NOTIFY friendsChanged)
    // Property để binding danh sách tin nhắn lên QML
    Q_PROPERTY(QVariantList messages READ messages NOTIFY messagesChanged)
    // Property trạng thái bạn bè với người đang chọn (-1: chưa, 0: chờ, 1: bạn)
    Q_PROPERTY(int currentFriendStatus READ currentFriendStatus NOTIFY currentFriendStatusChanged)

public:
    /**
     * @brief Lấy instance duy nhất của FriendHandlers (Singleton Pattern).
     * @return Con trỏ tới instance của FriendHandlers.
     */
    static FriendHandlers* getInstance();
    explicit FriendHandlers(QObject *parent = nullptr);

    /**
     * @brief Gửi yêu cầu lấy danh sách người lạ (chưa kết bạn) lên Server.
     */
    Q_INVOKABLE void fetchNonFriendUsers();

    /**
     * @brief Gửi yêu cầu lấy danh sách lời mời kết bạn (Incoming) lên Server.
     */
    Q_INVOKABLE void fetchFriendRequests();

    /**
     * @brief Gửi yêu cầu lấy danh sách bạn bè lên Server.
     */
    Q_INVOKABLE void fetchFriends();

    /**
     * @brief Gửi tin nhắn tới người dùng đang chat (Target ID).
     * @param content Nội dung tin nhắn.
     */
    Q_INVOKABLE void sendMessage(const QString &content);

    /**
     * @brief Tải lịch sử tin nhắn với một người bạn cụ thể.
     * @param friendID ID của người bạn muốn xem tin nhắn.
     */
    Q_INVOKABLE void loadMessages(int friendID);

    /**
     * @brief Gửi yêu cầu kết bạn tới một người dùng khác.
     * @param toUserID ID của người dùng mà ta muốn gửi lời mời kết bạn.
     */
    Q_INVOKABLE void sendFriendRequest(int toUserID);
    
    /**
     * @brief Gửi yêu cầu truy vấn trạng thái kết bạn với một người dùng khác.
     * @param toUserID ID của người dùng mà ta muốn truy vấn trạng thái kết bạn.
     */
    Q_INVOKABLE void sendQueryFriendStatus(int toUserID);

    /**
     * @brief Gửi yêu cầu chấp nhận lời mời kết bạn từ một người dùng khác.
     * @param fromUserID ID của người dùng đã gửi lời mời kết bạn.
     */
    Q_INVOKABLE void sendAcceptFriendRequest(int fromUserID);

    /**
     * @brief Gửi yêu cầu lấy danh sách bạn bè của người dùng.
     * @param userID ID của người dùng mà ta muốn lấy danh sách bạn bè.
     */
    Q_INVOKABLE void sendGetFriendListRequest(int userID);
    // Getter cho property nonFriendUsers
    QVariantList nonFriendUsers() const;
    // Getter cho property friendRequests
    QVariantList friendRequests() const;
    // Getter cho property friends
    QVariantList friends() const;
    // Getter cho property messages
    QVariantList messages() const;
    // Getter cho property currentFriendStatus
    int currentFriendStatus() const;

signals:
    // Signal phát ra khi danh sách người lạ thay đổi (để QML cập nhật UI)
    void nonFriendUsersChanged();
    // Signal phát ra khi danh sách yêu cầu kết bạn thay đổi
    void friendRequestsChanged();
    // Signal phát ra khi danh sách bạn bè thay đổi
    void friendsChanged();
    // Signal phát ra khi danh sách tin nhắn thay đổi (để QML cập nhật UI)
    void messagesChanged();
    // Signal phát ra khi trạng thái bạn bè thay đổi
    void currentFriendStatusChanged();
    // Signal phát ra khi tin nhắn đã được gửi đi
    void messageSent();
    // Signal nội bộ dùng để nhận tin nhắn từ WinSockClient
    void messageReceived(const QJsonObject &message);

private slots:
    // Slot xử lý khi nhận được danh sách người lạ từ Server
    void onNonFriendUsersReceived(const QJsonObject &data);
    // Slot xử lý khi nhận được danh sách yêu cầu kết bạn từ Server
    void onFriendRequestsReceived(const QJsonObject &data);
    // Slot xử lý khi nhận được danh sách bạn bè từ Server
    void onFriendsReceived(const QJsonObject &data);
    // Slot xử lý khi nhận được trạng thái bạn bè
    void onFriendStatusReceived(const QJsonObject &data);
    // Slot xử lý chung cho việc nhận tin nhắn (cả tin nhắn mới và lịch sử tin nhắn)
    void onMessageReceived(const QJsonObject &data);
    // Slot cũ (có thể không dùng nữa, giữ lại để tương thích nếu cần)
    void onGetAllMessagesReceived(const QJsonObject &data);

private:
    static FriendHandlers* m_instance;
    QVariantList m_nonFriendUsers; // Lưu trữ danh sách người lạ
    QVariantList m_friendRequests; // Lưu trữ danh sách yêu cầu kết bạn
    QVariantList m_friends;        // Lưu trữ danh sách bạn bè
    QVariantList m_messages;       // Lưu trữ danh sách tin nhắn hiện tại
    int m_currentFriendStatus = -1;
};

#endif // FRIENDHANDLERS_H
