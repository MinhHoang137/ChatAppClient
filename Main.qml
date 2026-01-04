import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client

Window {
    width: 1200
    height: 800
    visible: true
    title: qsTr("Chat Client")

    AuthenticationHandler {
        id: authHandler
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: authPage
    }

    Component {
        id: homePage
        Home {
            onLogout: stackView.pop()
        }
    }

    Component {
        id: authPage
        Page {
            // Banner cảnh báo mất kết nối
            Rectangle {
                id: disconnectBanner
                visible: !winSockClient.isConnected
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 40
                z: 1000
                color: "#FFF4E5"
                border.color: "#FFD699"
                border.width: 1
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 10
                    Label { text: "Mất kết nối tới server"; color: "#8A6D3B"; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: "Kết nối lại"
                        onClicked: winSockClient.reconnect()
                    }
                }
            }
            Dialog {
                id: messageDialog
                anchors.centerIn: parent
                title: "Thông báo"
                standardButtons: Dialog.Ok
                property string text: ""
                
                contentItem: Label {
                    text: messageDialog.text
                    wrapMode: Text.WordWrap
                }
            }

            Connections {
                target: winSockClient
                
                function onRegisterReceived(data) {
                    console.log("Register received:", JSON.stringify(data))
                    messageDialog.title = data.success ? "Đăng ký thành công" : "Đăng ký thất bại"
                    messageDialog.text = data.message
                    messageDialog.open()
                }

                function onLoginReceived(data) {
                    console.log("Login received:", JSON.stringify(data))
                    if (data.success) {
                        stackView.push(homePage, {username: loginUsername.text})
                    } else {
                        messageDialog.title = "Đăng nhập thất bại"
                        messageDialog.text = data.message
                        messageDialog.open()
                    }
                }

                function onConnectionLost() {
                    messageDialog.title = "Mất kết nối"
                    messageDialog.text = "Kết nối tới server bị mất. Ứng dụng sẽ thử kết nối lại nếu được bật."
                    messageDialog.open()
                }

                function onReconnected() {
                    messageDialog.title = "Đã kết nối lại"
                    messageDialog.text = "Đã kết nối lại thành công với server."
                    messageDialog.open()
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: disconnectBanner.visible ? disconnectBanner.height : 0
                anchors.margins: 20
                spacing: 15

                // --- Server Connection Section ---
                GroupBox {
                    title: "Cấu hình Server"
                    Layout.fillWidth: true
                    
                    ColumnLayout {
                        width: parent.width
                        
                        RowLayout {
                            Layout.fillWidth: true
                            TextField {
                                id: ipField
                                placeholderText: "IP Address"
                                text: "127.0.0.1"
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: portField
                                placeholderText: "Port"
                                text: "8080"
                                Layout.preferredWidth: 80
                            }
                        }

                        Button {
                            text: winSockClient.isConnected ? "Ngắt kết nối" : "Kết nối"
                            Layout.fillWidth: true
                            highlighted: !winSockClient.isConnected
                            onClicked: {
                                if (winSockClient.isConnected) {
                                    winSockClient.disconnectFromServer()
                                } else {
                                    winSockClient.connectToServer(ipField.text, portField.text)
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            CheckBox {
                                id: autoReconnectCheck
                                text: "Tự động kết nối lại"
                                checked: true
                                onToggled: winSockClient.autoReconnect = checked
                            }
                            Label {
                                text: winSockClient.statusMessage
                                color: winSockClient.isConnected ? "green" : "red"
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                // --- Auth Section ---
                TabBar {
                    id: authTabBar
                    Layout.fillWidth: true
                    enabled: winSockClient.isConnected
                    
                    TabButton { text: "Đăng nhập" }
                    TabButton { text: "Đăng ký" }
                }

                StackLayout {
                    currentIndex: authTabBar.currentIndex
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    enabled: winSockClient.isConnected

                    // Login Tab
                    Item {
                        ColumnLayout {
                            anchors.centerIn: parent
                            width: parent.width
                            spacing: 10

                            TextField {
                                id: loginUsername
                                placeholderText: "Username"
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: loginPassword
                                placeholderText: "Password"
                                echoMode: TextInput.Password
                                Layout.fillWidth: true
                            }
                            Button {
                                text: "Đăng nhập"
                                Layout.fillWidth: true
                                highlighted: true
                                onClicked: {
                                    authHandler.loginUser(loginUsername.text, loginPassword.text)
                                }
                            }
                        }
                    }

                    // Register Tab
                    Item {
                        ColumnLayout {
                            anchors.centerIn: parent
                            width: parent.width
                            spacing: 10

                            TextField {
                                id: regUsername
                                placeholderText: "Username"
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: regPassword
                                placeholderText: "Password"
                                echoMode: TextInput.Password
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: regConfirmPassword
                                placeholderText: "Confirm Password"
                                echoMode: TextInput.Password
                                Layout.fillWidth: true
                            }
                            Button {
                                text: "Đăng ký"
                                Layout.fillWidth: true
                                highlighted: true
                                onClicked: {
                                    if (regPassword.text !== regConfirmPassword.text) {
                                        messageDialog.title = "Lỗi"
                                        messageDialog.text = "Mật khẩu xác nhận không khớp!"
                                        messageDialog.open()
                                        return
                                    }
                                    
                                    authHandler.registerUser(regUsername.text, regPassword.text)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
