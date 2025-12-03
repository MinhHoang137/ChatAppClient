#pragma once

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QQmlEngine>

// Các hàm callback xử lý phản hồi từ server (Legacy)
void handleRegisterResponse(const QJsonObject &response);
void handleLoginResponse(const QJsonObject &response);

/**
 * @brief Lớp xử lý xác thực người dùng (Đăng ký, Đăng nhập).
 * Được expose cho QML để gọi từ giao diện.
 */
class AuthenticationHandler : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit AuthenticationHandler(QObject *parent = nullptr);

    /**
     * @brief Gửi yêu cầu đăng ký tài khoản mới.
     * @param username Tên đăng nhập.
     * @param password Mật khẩu.
     */
    Q_INVOKABLE void registerUser(QString username, QString password);

    /**
     * @brief Gửi yêu cầu đăng nhập.
     * @param username Tên đăng nhập.
     * @param password Mật khẩu.
     */
    Q_INVOKABLE void loginUser(QString username, QString password);
};
