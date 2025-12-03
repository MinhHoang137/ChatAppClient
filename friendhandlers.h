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
    // Property để binding danh sách tin nhắn lên QML
    Q_PROPERTY(QVariantList messages READ messages NOTIFY messagesChanged)

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
     * @brief Gửi tin nhắn tới người dùng đang chat (Target ID).
     * @param content Nội dung tin nhắn.
     */
    Q_INVOKABLE void sendMessage(const QString &content);

    /**
     * @brief Tải lịch sử tin nhắn với một người bạn cụ thể.
     * @param friendID ID của người bạn muốn xem tin nhắn.
     */
    Q_INVOKABLE void loadMessages(int friendID);
    
    // Getter cho property nonFriendUsers
    QVariantList nonFriendUsers() const;
    // Getter cho property messages
    QVariantList messages() const;

signals:
    // Signal phát ra khi danh sách người lạ thay đổi (để QML cập nhật UI)
    void nonFriendUsersChanged();
    // Signal phát ra khi danh sách tin nhắn thay đổi (để QML cập nhật UI)
    void messagesChanged();
    // Signal phát ra khi tin nhắn đã được gửi đi
    void messageSent();
    // Signal nội bộ dùng để nhận tin nhắn từ WinSockClient
    void messageReceived(const QJsonObject &message);

private slots:
    // Slot xử lý khi nhận được danh sách người lạ từ Server
    void onNonFriendUsersReceived(const QJsonObject &data);
    // Slot xử lý chung cho việc nhận tin nhắn (cả tin nhắn mới và lịch sử tin nhắn)
    void onMessageReceived(const QJsonObject &data);
    // Slot cũ (có thể không dùng nữa, giữ lại để tương thích nếu cần)
    void onGetAllMessagesReceived(const QJsonObject &data);

private:
    static FriendHandlers* m_instance;
    QVariantList m_nonFriendUsers; // Lưu trữ danh sách người lạ
    QVariantList m_messages;       // Lưu trữ danh sách tin nhắn hiện tại
};

#endif // FRIENDHANDLERS_H
