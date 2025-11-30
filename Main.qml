import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 400
    height: 500
    visible: true
    title: qsTr("Chat Client Login")

    Connections {
        target: winSockClient
        function onMessageReceived(message) {
            try {
                var response = JSON.parse(message)
                if (response.action === "registerResponse") {
                    if (response.success) {
                        console.log("Đăng ký thành công! User ID: " + response.userId)
                        // Có thể hiển thị thông báo hoặc tự động điền username/pass
                    } else {
                        console.log("Đăng ký thất bại: " + response.message)
                    }
                }
            } catch (e) {
                console.log("Lỗi parse JSON: " + e)
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.8
        spacing: 10

        Label {
            text: "Đăng nhập Chat Server"
            font.pixelSize: 20
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        TextField {
            id: ipField
            placeholderText: "Địa chỉ IP Server"
            text: "127.0.0.1"
            Layout.fillWidth: true
        }

        TextField {
            id: portField
            placeholderText: "Cổng (Port)"
            text: "8080"
            Layout.fillWidth: true
        }

        Button {
            text: "Kết nối Server"
            Layout.fillWidth: true
            visible: !winSockClient.isConnected
            onClicked: {
                winSockClient.connectToServer(ipField.text, portField.text)
            }
            background: Rectangle {
                color: parent.down ? "#1976D2" : "#2196F3"
                radius: 4
            }
            contentItem: Text {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font: parent.font
            }
        }

        TextField {
            id: usernameField
            placeholderText: "Username"
            Layout.fillWidth: true
        }

        TextField {
            id: passwordField
            placeholderText: "Password"
            echoMode: TextInput.Password
            Layout.fillWidth: true
        }

        Button {
            text: "Đăng nhập"
            Layout.fillWidth: true
            enabled: winSockClient.isConnected
            onClicked: {
                var loginJson = {
                    "action": "login",
                    "username": usernameField.text,
                    "password": passwordField.text
                }
                winSockClient.sendMessage(JSON.stringify(loginJson))
            }
            background: Rectangle {
                color: !parent.enabled ? "#bdc3c7" : (parent.down ? "#388E3C" : "#4CAF50")
                radius: 4
            }
            contentItem: Text {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font: parent.font
            }
        }

        Button {
            text: "Đăng ký"
            Layout.fillWidth: true
            enabled: winSockClient.isConnected
            onClicked: {
                var registerJson = {
                    "action": "register",
                    "username": usernameField.text,
                    "password": passwordField.text
                }
                winSockClient.sendMessage(JSON.stringify(registerJson))
            }
            background: Rectangle {
                color: !parent.enabled ? "#bdc3c7" : (parent.down ? "#F57C00" : "#FF9800")
                radius: 4
            }
            contentItem: Text {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font: parent.font
            }
        }

        Button {
            text: "Ngắt kết nối"
            Layout.fillWidth: true
            enabled: winSockClient.isConnected
            onClicked: {
                winSockClient.disconnectFromServer()
            }
            background: Rectangle {
                color: !parent.enabled ? "#bdc3c7" : (parent.down ? "#D32F2F" : "#F44336")
                radius: 4
            }
            contentItem: Text {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font: parent.font
            }
        }

        Label {
            text: winSockClient.statusMessage
            color: winSockClient.isConnected ? "green" : "red"
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
