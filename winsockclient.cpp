#include "winsockclient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include "authentication.h"

WinSockClient* WinSockClient::m_instance = nullptr;

WinSockClient* WinSockClient::getInstance()
{
    if (m_instance == nullptr) {
        m_instance = new WinSockClient();
    }
    return m_instance;
}

WinSockClient::WinSockClient(QObject *parent)
    : QObject(parent)
    , m_socket(INVALID_SOCKET)
    , m_running(false)
    , m_connected(false)
{
    // Initialize WinSock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        setStatus("WSAStartup failed: " + QString::number(iResult));
    }

    // Register handlers
    registerHandler("registerResponse", handleRegisterResponse);
    registerHandler("loginResponse", handleLoginResponse);

    // Initialize signal map
    m_signalMap["registerResponse"] = [this](const QJsonObject &data){ emit registerReceived(data); };
    m_signalMap["loginResponse"] = [this](const QJsonObject &data){ emit loginReceived(data); };
}

WinSockClient::~WinSockClient()
{
    disconnectFromServer();
    WSACleanup();
}

void WinSockClient::connectToServer(const QString &ip, const QString &port)
{
    if (m_connected) {
        disconnectFromServer();
    }

    setStatus("Đang kết nối...");

    struct addrinfo *result = NULL, *ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    int iResult = getaddrinfo(ip.toStdString().c_str(), port.toStdString().c_str(), &hints, &result);
    if (iResult != 0) {
        setStatus("getaddrinfo failed: " + QString::number(iResult));
        return;
    }

    // ---------------------------------------------------------
    // GIAI ĐOẠN 1: TẠO SOCKET
    // ---------------------------------------------------------
    ptr = result;
    m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (m_socket == INVALID_SOCKET) {
        setStatus("Error at socket(): " + QString::number(WSAGetLastError()));
        freeaddrinfo(result);
        return;
    }

    // ---------------------------------------------------------
    // GIAI ĐOẠN 2: KẾT NỐI (CONNECT)
    // ---------------------------------------------------------
    iResult = ::connect(m_socket, ptr->ai_addr, (int) ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        setStatus("Không thể kết nối tới server.");
        freeaddrinfo(result);
        return;
    }

    freeaddrinfo(result);

    // Kết nối thành công
    m_running = true;
    setConnected(true);
    setStatus("Kết nối thành công!");

    // ---------------------------------------------------------
    // GIAI ĐOẠN 3: TÁCH LUỒNG GỬI VÀ LUỒNG NGHE RIÊNG
    // ---------------------------------------------------------

    // Khởi tạo luồng gửi (Send Thread)
    m_sendThread = std::thread(&WinSockClient::sendLoop, this);

    // Khởi tạo luồng nghe (Receive Thread)
    m_recvThread = std::thread(&WinSockClient::receiveLoop, this);
}

int WinSockClient::sendMessage(const QJsonObject &json)
{
    if (!m_connected)
        return 0;

    QJsonDocument doc(json);
    QByteArray bytes = doc.toJson(QJsonDocument::Compact);
    std::string msg = bytes.toStdString();

    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        m_sendQueue.push(msg);
    }
    m_sendCv.notify_one();
    return (int)msg.size();
}

void WinSockClient::disconnectFromServer()
{
    m_running = false;
    m_sendCv.notify_all(); // Wake up send thread

    if (m_socket != INVALID_SOCKET) {
        shutdown(m_socket, SD_BOTH);
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    if (m_recvThread.joinable()) {
        m_recvThread.join();
    }
    if (m_sendThread.joinable()) {
        m_sendThread.join();
    }

    setConnected(false);
    setStatus("Đã ngắt kết nối.");
}

void WinSockClient::receiveLoop()
{
    // LUỒNG NGHE (RECEIVE THREAD)
    // Chuyên trách việc nhận dữ liệu từ server
    char recvbuf[512];
    int recvbuflen = 512;

    while (m_running) {
        int iResult = recv(m_socket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            // Nhận được dữ liệu
            std::string msg(recvbuf, iResult);
            QString qMsg = QString::fromStdString(msg);

            // Emit signal (Qt handles thread safety)
            emit messageReceived(qMsg);

            // Parse JSON in receive thread
            QJsonDocument doc = QJsonDocument::fromJson(qMsg.toUtf8());
            if (!doc.isNull() && doc.isObject()) {
                QJsonObject obj = doc.object();
                // Run processMessage in Main Thread (UI Thread) to handle UI updates
                QMetaObject::invokeMethod(
                    this, [this, obj]() { processMessage(obj); }, Qt::QueuedConnection);
            }

        } else if (iResult == 0) {
            // Connection closed
            if (m_running) {
                // Server đóng kết nối
                m_running = false;
            }
        } else {
            // Error
            if (m_running) {
                // Lỗi nhận
                m_running = false;
            }
        }
    }
    // Khi thoát vòng lặp, đảm bảo trạng thái cập nhật
    if (m_connected) {
        QMetaObject::invokeMethod(this, [this]() {
            setConnected(false);
            setStatus("Mất kết nối với server.");
        });
    }
}

void WinSockClient::sendLoop()
{
    // LUỒNG GỬI (SEND THREAD)
    // Chuyên trách việc lấy dữ liệu từ hàng đợi và gửi đi
    while (m_running) {
        std::unique_lock<std::mutex> lock(m_sendMutex);
        m_sendCv.wait(lock, [this] { return !m_sendQueue.empty() || !m_running; });

        if (!m_running)
            break;

        while (!m_sendQueue.empty()) {
            std::string msg = m_sendQueue.front();
            m_sendQueue.pop();
            lock.unlock(); // Unlock để gửi, tránh giữ lock lâu

            int iResult = send(m_socket, msg.c_str(), (int) msg.length(), 0);
            if (iResult == SOCKET_ERROR) {
                // Xử lý lỗi gửi
                m_running = false;
            }

            lock.lock(); // Lock lại để check queue tiếp
        }
    }
}

QString WinSockClient::statusMessage() const
{
    return m_statusMessage;
}

bool WinSockClient::isConnected() const
{
    return m_connected;
}

void WinSockClient::setStatus(const QString &msg)
{
    if (m_statusMessage != msg) {
        m_statusMessage = msg;
        emit statusMessageChanged();
    }
}

void WinSockClient::setConnected(bool connected)
{
    if (m_connected != connected) {
        m_connected = connected;
        emit isConnectedChanged();
    }
}

void WinSockClient::registerHandler(const QString &action, MessageHandler handler)
{
    m_handlers[action] = handler;
}

void WinSockClient::processMessage(const QJsonObject &message)
{
    QString action = message["action"].toString();
    if (m_handlers.find(action) != m_handlers.end()) {
        m_handlers[action](message);
    } else {
        qDebug() << "No handler for action:" << action;
    }

    if (m_signalMap.find(action) != m_signalMap.end()) {
        m_signalMap[action](message);
    }
}

int WinSockClient::getUserId(){
    return userId;
}

void WinSockClient::setUserId(int id){
    userId = id;
}
