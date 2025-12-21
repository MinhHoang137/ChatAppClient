#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

/**
 * @brief Lớp quản lý kết nối mạng (Socket Client).
 * Chịu trách nhiệm kết nối tới Server, gửi/nhận dữ liệu JSON, và quản lý luồng gửi/nhận.
 * Thiết kế theo mẫu Singleton.
 */
class WinSockClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(int userId READ getUserId WRITE setUserId NOTIFY userIdChanged)
    Q_PROPERTY(bool autoReconnect READ autoReconnect WRITE setAutoReconnect NOTIFY autoReconnectChanged)
    Q_PROPERTY(int reconnectAttempts READ reconnectAttempts NOTIFY reconnectAttemptsChanged)
    Q_PROPERTY(int groupId READ getGroupId WRITE setGroupId NOTIFY groupIdChanged)
    Q_PROPERTY(int targetId READ getTargetId WRITE setTargetId NOTIFY targetIdChanged)

public:
    /**
     * @brief Lấy instance duy nhất của WinSockClient.
     */
    static WinSockClient *getInstance();
    WinSockClient(const WinSockClient &) = delete;
    WinSockClient &operator=(const WinSockClient &) = delete;

    ~WinSockClient();

    /**
     * @brief Kết nối tới Server.
     * @param ip Địa chỉ IP của Server.
     * @param port Cổng của Server.
     */
    Q_INVOKABLE void connectToServer(const QString &ip, const QString &port);

    /**
     * @brief Gửi một gói tin JSON tới Server.
     * @param message Đối tượng JSON cần gửi.
     * @return Số byte đã gửi (hoặc kích thước hàng đợi).
     */
    Q_INVOKABLE int sendMessage(const QJsonObject &message);

    /**
     * @brief Ngắt kết nối khỏi Server.
     */
    Q_INVOKABLE void disconnectFromServer();

    // Thử kết nối lại tới Server sử dụng cấu hình lần trước
    Q_INVOKABLE void reconnect();

    QString statusMessage() const;
    bool isConnected() const;
    Q_INVOKABLE int getUserId() const;
    Q_INVOKABLE void setUserId(int id);

    Q_INVOKABLE int getGroupId() const;
    Q_INVOKABLE void setGroupId(int newGroupId);

    Q_INVOKABLE int getTargetId() const;
    Q_INVOKABLE void setTargetId(int newTargetId);

    bool autoReconnect() const;
    void setAutoReconnect(bool enable);
    int reconnectAttempts() const;

signals:
    void statusMessageChanged();
    void isConnectedChanged();
    void userIdChanged();
    // Phát ra khi mất kết nối với Server
    void connectionLost();
    // Phát ra khi kết nối lại thành công
    void reconnected();
    // Phát ra khi thử kết nối lại thất bại (đưa số lần thử)
    void reconnectFailed(int attempt);
    // Thay đổi thuộc tính tự động kết nối lại
    void autoReconnectChanged();
    // Thay đổi số lần thử kết nối lại
    void reconnectAttemptsChanged();
    // Signal phát ra khi nhận phản hồi đăng ký
    void registerReceived(const QJsonObject &data);
    // Signal phát ra khi nhận phản hồi đăng nhập
    void loginReceived(const QJsonObject &data);
    // Signal phát ra khi nhận danh sách người lạ
    void nonFriendUsersReceived(const QJsonObject &data);
    // Signal phát ra khi nhận danh sách yêu cầu kết bạn
    void friendRequestsReceived(const QJsonObject &data);
    // Signal phát ra khi nhận danh sách bạn bè
    void friendsListReceived(const QJsonObject &data);
    // Signal phát ra khi nhận trạng thái bạn bè
    void friendStatusReceived(const QJsonObject &data);
    // Signal phát ra khi groupId thay đổi
    void groupIdChanged();
    // Signal phát ra khi targetId thay đổi
    void targetIdChanged();

private:
    using MessageHandler = std::function<void(const QJsonObject &)>;

    // Đăng ký hàm xử lý cho một action cụ thể
    void registerHandler(const QString &action, MessageHandler handler);
    // Xử lý gói tin JSON nhận được, định tuyến tới handler phù hợp
    void processMessage(const QJsonObject &message);

    // Vòng lặp nhận dữ liệu (chạy trên thread riêng)
    void receiveLoop();
    // Vòng lặp gửi dữ liệu (chạy trên thread riêng)
    void sendLoop();
    void cleanup();
    void setStatus(const QString &msg);
    void setConnected(bool connected);
    void startAutoReconnect();

    static WinSockClient *m_instance;
    explicit WinSockClient(QObject *parent = nullptr);

    SOCKET m_socket;
    std::atomic<bool> m_running;
    std::atomic<bool> m_connected;
    std::atomic<bool> m_autoReconnect{true};
    int m_reconnectAttempts = 0;
    QString m_lastIp;
    QString m_lastPort;
    QString m_statusMessage;

    // Map ánh xạ từ tên action sang hàm xử lý (callback)
    std::map<QString, MessageHandler> m_handlers;
    // Map ánh xạ từ tên action sang signal (để emit signal tương ứng)
    std::map<QString, std::function<void(const QJsonObject &)>> m_signalMap;

    // Threading
    std::thread m_recvThread;
    std::thread m_sendThread;
    std::thread m_reconnectThread;

    // Send queue (Hàng đợi gửi tin nhắn để đảm bảo thread-safe)
    std::queue<std::string> m_sendQueue;
    std::mutex m_sendMutex;
    std::condition_variable m_sendCv;

    int userId = 0;
    int groupId = 0;
    int targetId = 0; // ID của người dùng/nhóm đang chat cùng
};
