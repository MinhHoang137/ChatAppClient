#include "winsockclient.h"
#include "authentication.h"
#include "filesharinghandlers.h"
#include "friendhandlers.h"
#include "grouphandlers.h"
#include "header.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

WinSockClient *WinSockClient::m_instance = nullptr;

// Singleton implementation
WinSockClient *WinSockClient::getInstance() {
  if (m_instance == nullptr) {
    m_instance = new WinSockClient();
  }
  return m_instance;
}

WinSockClient::WinSockClient(QObject *parent)
    : QObject(parent), m_socket(INVALID_SOCKET), m_running(false),
      m_connected(false) {
  // Khởi tạo thư viện WinSock
  WSADATA wsaData;
  int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) {
    setStatus("WSAStartup failed: " + QString::number(iResult));
  }

  // Đăng ký các handler xử lý phản hồi từ server (Legacy callbacks)
  registerHandler("registerResponse", handleRegisterResponse);
  registerHandler("loginResponse", handleLoginResponse);

  // Khởi tạo signal map: Ánh xạ từ action string sang signal emit
  // Khi nhận được gói tin có action tương ứng, signal sẽ được phát ra
  m_signalMap["registerResponse"] = [this](const QJsonObject &data) {
    emit registerReceived(data);
  };
  m_signalMap["loginResponse"] = [this](const QJsonObject &data) {
    emit loginReceived(data);
  };
  m_signalMap["getNonFriendUsers"] = [this](const QJsonObject &data) {
    emit nonFriendUsersReceived(data);
  };
  m_signalMap["getFriendRequests"] = [this](const QJsonObject &data) {
    emit friendRequestsReceived(data);
  };
  m_signalMap["getFriendsList"] = [this](const QJsonObject &data) {
    emit friendsListReceived(data);
  };
  m_signalMap["queryFriendStatus"] = [this](const QJsonObject &data) {
    emit friendStatusReceived(data);
  };

  // Các action liên quan đến tin nhắn đều được chuyển về FriendHandlers xử lý
  m_signalMap["receiveMessage"] = [](const QJsonObject &data) {
    emit FriendHandlers::getInstance()->messageReceived(data);
  };
  m_signalMap["getAllMessages"] = [](const QJsonObject &data) {
    emit FriendHandlers::getInstance()->messageReceived(data);
  };
  m_signalMap["sendMessage"] = [](const QJsonObject &data) {
    emit FriendHandlers::getInstance()->messageReceived(data);
  };

  // Group-related signals
  m_signalMap["getUserGroups"] = [](const QJsonObject &data) {
    emit GroupHandlers::getInstance()->groupsReceived(data);
  };
  m_signalMap["getGroupMessages"] = [](const QJsonObject &data) {
    emit GroupHandlers::getInstance()->groupMessageReceived(data);
  };
  m_signalMap["receiveGroupMessage"] = [](const QJsonObject &data) {
    emit GroupHandlers::getInstance()->groupMessageReceived(data);
  };
  m_signalMap["sendGroupMessage"] = [](const QJsonObject &data) {
    emit GroupHandlers::getInstance()->groupMessageReceived(data);
  };
  m_signalMap["getGroupMembers"] = [](const QJsonObject &data) {
    emit GroupHandlers::getInstance()->groupMembersReceived(data);
  };
  m_signalMap["exploreGroups"] = [](const QJsonObject &data) {
    emit GroupHandlers::getInstance()->exploreGroupsReceived(data);
  };
  m_signalMap["requestJoinGroup"] = [](const QJsonObject &data) {
    emit GroupHandlers::getInstance()->requestJoinReceived(data);
  };
  m_signalMap["getGroupRequests"] = [](const QJsonObject &data) {
    emit GroupHandlers::getInstance()->groupRequestsReceived(data);
  };
  m_signalMap["respondToRequest"] = [](const QJsonObject &data) {
    emit GroupHandlers::getInstance()->respondToRequestReceived(data);
  };

  // File Sharing signals
  auto fsHandler = FileSharingHandlers::getInstance();
  m_signalMap["listFiles"] = [fsHandler](const QJsonObject &data) {
    emit fsHandler->listFilesReceived(data);
  };
  m_signalMap["uploadChunk"] = [fsHandler](const QJsonObject &data) {
    emit fsHandler->uploadChunkAck(data);
  };
  m_signalMap["downloadChunk"] = [fsHandler](const QJsonObject &data) {
    emit fsHandler->downloadChunkReceived(data);
  };
  m_signalMap["createFolder"] = [fsHandler](const QJsonObject &data) {
    emit fsHandler->operationResultReceived(data);
  };
  m_signalMap["deleteItem"] = [fsHandler](const QJsonObject &data) {
    emit fsHandler->operationResultReceived(data);
  };
  m_signalMap["renameItem"] = [fsHandler](const QJsonObject &data) {
    emit fsHandler->operationResultReceived(data);
  };
  m_signalMap["browseDirectories"] = [fsHandler](const QJsonObject &data) {
    emit fsHandler->browseDirectoriesReceived(data);
  };
}

int WinSockClient::getTargetId() const { return targetId; }

void WinSockClient::setTargetId(int newTargetId) {
  if (targetId != newTargetId) {
    targetId = newTargetId;
    emit targetIdChanged();
  }
  // Chỉ reset groupId khi đang set targetId khác 0 (tức là chọn chat với user)
  if (newTargetId != 0 && groupId != 0) {
    groupId = 0;
    emit groupIdChanged();
  }
}

int WinSockClient::getGroupId() const { return groupId; }

void WinSockClient::setGroupId(int newGroupId) {
  if (groupId != newGroupId) {
    groupId = newGroupId;
    emit groupIdChanged();
  }
  // Chỉ reset targetId khi đang set groupId khác 0 (tức là chọn chat nhóm)
  if (newGroupId != 0 && targetId != 0) {
    targetId = 0;
    emit targetIdChanged();
  }
}

WinSockClient::~WinSockClient() {
  disconnectFromServer();
  WSACleanup();
}

void WinSockClient::connectToServer(const QString &ip, const QString &port) {
  if (m_connected) {
    disconnectFromServer();
  }

  setStatus("Đang kết nối...");
  m_lastIp = ip;
  m_lastPort = port;
  m_reconnectAttempts = 0;

  struct addrinfo *result = NULL, *ptr = NULL, hints;

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  // Phân giải địa chỉ server
  int iResult = getaddrinfo(ip.toStdString().c_str(),
                            port.toStdString().c_str(), &hints, &result);
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
  iResult = ::connect(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
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

int WinSockClient::sendMessage(const QJsonObject &json) {
  if (!m_connected)
    return 0;

  QJsonDocument doc(json);
  QByteArray bytes = doc.toJson(QJsonDocument::Compact);
  std::string msg = bytes.toStdString();
  // Append newline as a simple frame delimiter for TCP stream parsing on server
  msg.push_back('\n');

  // Log enqueue of outgoing message for traceability
  clientLogMessage(std::string("[QUEUE] ") + msg);

  // Đẩy tin nhắn vào hàng đợi gửi (Thread-safe)
  {
    std::lock_guard<std::mutex> lock(m_sendMutex);
    m_sendQueue.push(msg);
  }
  m_sendCv.notify_one(); // Đánh thức luồng gửi
  return (int)msg.size();
}

void WinSockClient::disconnectFromServer() {
  m_running = false;
  m_sendCv.notify_all(); // Đánh thức luồng gửi để nó thoát

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

void WinSockClient::receiveLoop() {
  // LUỒNG NGHE (RECEIVE THREAD)
  // Chuyên trách việc nhận dữ liệu từ server
  char recvbuf[8192];
  int recvbuflen = 8192;

  std::string buffer;
  while (m_running) {
    int iResult = recv(m_socket, recvbuf, recvbuflen, 0);
    if (iResult > 0) {
      // Append received data to buffer
      buffer.append(recvbuf, iResult);

      // Process all complete messages in the buffer
      size_t pos;
      while ((pos = buffer.find('\n')) != std::string::npos) {
        std::string msg = buffer.substr(0, pos);
        buffer.erase(0, pos + 1);

        // Ghi log dữ liệu nhận được từ Server
        // clientLogMessage(std::string("[RX] ") + msg);

        // Parse JSON
        // Use QByteArray from raw string bytes to avoid encoding issues
        QByteArray jsonBytes(msg.data(), msg.size());
        QJsonDocument doc = QJsonDocument::fromJson(jsonBytes);

        if (!doc.isNull() && doc.isObject()) {
          QJsonObject obj = doc.object();
          // Chuyển việc xử lý logic về Main Thread
          QMetaObject::invokeMethod(
              this, [this, obj]() { processMessage(obj); },
              Qt::QueuedConnection);
        } else {
          // Optional: Log invalid JSON or keepalives
          // qDebug() << "Invalid JSON or partial:" <<
          // QString::fromStdString(msg);
        }
      }

    } else if (iResult == 0) {
      // Connection closed
      if (m_running) {
        // Server đóng kết nối
        m_running = false;
        clientLogMessage("[INFO] Server closed the connection.");
      }
    } else {
      // Error
      if (m_running) {
        // Lỗi nhận
        m_running = false;
        clientLogMessage("[ERROR] recv failed, connection lost.");
      }
    }
  }
  // Khi thoát vòng lặp, đảm bảo trạng thái cập nhật
  if (m_connected) {
    QMetaObject::invokeMethod(this, [this]() {
      setConnected(false);
      setStatus("Mất kết nối với server.");
      emit connectionLost();
      if (m_autoReconnect) {
        startAutoReconnect();
      }
    });
  }
}

void WinSockClient::sendLoop() {
  // LUỒNG GỬI (SEND THREAD)
  // Chuyên trách việc lấy dữ liệu từ hàng đợi và gửi đi
  while (m_running) {
    std::unique_lock<std::mutex> lock(m_sendMutex);
    // Chờ cho đến khi có tin nhắn trong hàng đợi hoặc bị yêu cầu dừng
    m_sendCv.wait(lock, [this] { return !m_sendQueue.empty() || !m_running; });

    if (!m_running)
      break;

    while (!m_sendQueue.empty()) {
      std::string msg = m_sendQueue.front();
      m_sendQueue.pop();
      lock.unlock(); // Unlock để gửi, tránh giữ lock lâu gây block luồng khác

      // Log the actual transmit attempt
      clientLogMessage(std::string("[TX] ") + msg);
      int iResult = send(m_socket, msg.c_str(), (int)msg.length(), 0);
      if (iResult == SOCKET_ERROR) {
        // Xử lý lỗi gửi
        m_running = false;
        setStatus("Lỗi gửi dữ liệu. Mất kết nối.");
        clientLogMessage("[ERROR] send failed, connection lost.");
      }

      lock.lock(); // Lock lại để check queue tiếp
    }
  }
}

QString WinSockClient::statusMessage() const { return m_statusMessage; }

bool WinSockClient::isConnected() const { return m_connected; }

void WinSockClient::setStatus(const QString &msg) {
  if (m_statusMessage != msg) {
    m_statusMessage = msg;
    emit statusMessageChanged();
  }
}

void WinSockClient::setConnected(bool connected) {
  if (m_connected != connected) {
    m_connected = connected;
    emit isConnectedChanged();
  }
}

void WinSockClient::registerHandler(const QString &action,
                                    MessageHandler handler) {
  m_handlers[action] = handler;
}

void WinSockClient::processMessage(const QJsonObject &message) {
  QString action = message["action"].toString();

  // Gọi handler callback (nếu có)
  if (m_handlers.find(action) != m_handlers.end()) {
    m_handlers[action](message);
  } else {
    // qDebug() << "No handler for action:" << action;
  }

  // Emit signal tương ứng (nếu có)
  if (m_signalMap.find(action) != m_signalMap.end()) {
    m_signalMap[action](message);
  }
}

int WinSockClient::getUserId() const { return userId; }

void WinSockClient::setUserId(int id) {
  if (userId != id) {
    userId = id;
    emit userIdChanged();
  }
}

bool WinSockClient::autoReconnect() const { return m_autoReconnect.load(); }

void WinSockClient::setAutoReconnect(bool enable) {
  if (m_autoReconnect.load() != enable) {
    m_autoReconnect.store(enable);
    emit autoReconnectChanged();
  }
}

int WinSockClient::reconnectAttempts() const { return m_reconnectAttempts; }

void WinSockClient::reconnect() {
  // Thử kết nối lại ngay lập tức sử dụng cấu hình trước đó
  if (!m_lastIp.isEmpty() && !m_lastPort.isEmpty()) {
    connectToServer(m_lastIp, m_lastPort);
  }
}

void WinSockClient::startAutoReconnect() {
  // Nếu đã có luồng reconnect đang chạy, không khởi tạo thêm
  if (m_reconnectThread.joinable()) {
    return;
  }

  m_reconnectThread = std::thread([this]() {
    const int maxAttempts = 5;
    int delayMs = 1000; // 1s ban đầu
    for (m_reconnectAttempts = 1; m_reconnectAttempts <= maxAttempts;
         ++m_reconnectAttempts) {
      // Nếu người dùng đã tự kết nối lại thành công ở nơi khác
      if (m_connected) {
        break;
      }
      // Nếu bị tắt auto reconnect thì dừng
      if (!m_autoReconnect.load()) {
        break;
      }

      // Thử kết nối
      QMetaObject::invokeMethod(
          this,
          [this]() {
            setStatus(QString("Đang thử kết nối lại (lần %1)...")
                          .arg(m_reconnectAttempts));
          },
          Qt::QueuedConnection);

      if (!m_lastIp.isEmpty() && !m_lastPort.isEmpty()) {
        connectToServer(m_lastIp, m_lastPort);
      }

      // Kiểm tra kết quả
      if (m_connected) {
        QMetaObject::invokeMethod(
            this,
            [this]() {
              emit reconnected();
              setStatus("Kết nối lại thành công!");
            },
            Qt::QueuedConnection);
        break;
      } else {
        QMetaObject::invokeMethod(
            this, [this]() { emit reconnectFailed(m_reconnectAttempts); },
            Qt::QueuedConnection);
      }

      // Backoff
      std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
      delayMs = std::min(delayMs * 2, 8000); // Tối đa 8s
    }
  });
}
