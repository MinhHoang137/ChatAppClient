#pragma once

#include <QObject>
#include <QString>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <map>
#include <QJsonObject>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

class WinSockClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)

public:
    explicit WinSockClient(QObject *parent = nullptr);
    ~WinSockClient();

    Q_INVOKABLE void connectToServer(const QString &ip, const QString &port);
    Q_INVOKABLE void sendMessage(const QString &message);
    Q_INVOKABLE void disconnectFromServer();

    QString statusMessage() const;
    bool isConnected() const;

signals:
    void statusMessageChanged();
    void isConnectedChanged();
    void messageReceived(const QString &message);

private:
    using MessageHandler = std::function<void(const QJsonObject&)>;
    void registerHandler(const QString &action, MessageHandler handler);
    void processMessage(const QJsonObject &message);

    void receiveLoop();
    void sendLoop();
    void cleanup();
    void setStatus(const QString &msg);
    void setConnected(bool connected);

    SOCKET m_socket;
    std::atomic<bool> m_running;
    std::atomic<bool> m_connected;
    QString m_statusMessage;

    std::map<QString, MessageHandler> m_handlers;

    // Threading
    std::thread m_recvThread;
    std::thread m_sendThread;

    // Send queue
    std::queue<std::string> m_sendQueue;
    std::mutex m_sendMutex;
    std::condition_variable m_sendCv;

    int userId = 0;
    int groupId = 0;
};
