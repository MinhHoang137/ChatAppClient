import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client

Page {
    property string username: ""

    FriendHandlers {
        id: friendHandlers
    }

    header: ToolBar {
        background: Rectangle {
            color: "#0078d4"
        }
        RowLayout {
            anchors.fill: parent
            Label {
                text: "Chat App - " + username
                color: "white"
                font.pixelSize: 20
                font.bold: true
                Layout.leftMargin: 20
            }
            Item { Layout.fillWidth: true }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // --- LEFT SIDEBAR (Danh sách) ---
        Rectangle {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            color: "#f3f2f1"
            border.color: "#e1dfdd"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                TabBar {
                    id: contactTabBar
                    Layout.fillWidth: true
                    background: Rectangle { color: "#f3f2f1" }
                    
                    TabButton { text: "Bạn bè"; width: implicitWidth }
                    TabButton { text: "Người lạ"; width: implicitWidth }
                    TabButton { text: "Nhóm"; width: implicitWidth }

                    onCurrentIndexChanged: {
                        if (currentIndex === 1) {
                            friendHandlers.fetchNonFriendUsers()
                        }
                    }
                }

                StackLayout {
                    currentIndex: contactTabBar.currentIndex
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    // Tab 1: Bạn bè
                    ListView {
                        model: 10
                        clip: true
                        delegate: ItemDelegate {
                            width: parent.width
                            height: 50
                            highlighted: ListView.isCurrentItem
                            onClicked: parent.ListView.view.currentIndex = index
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 10
                                Rectangle {
                                    width: 32; height: 32; radius: 16
                                    color: "#cccccc"
                                    Text { anchors.centerIn: parent; text: "B" }
                                }
                                Label { 
                                    text: "Bạn bè " + (index + 1)
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }
                            }
                        }
                        ScrollIndicator.vertical: ScrollIndicator { }
                    }

                    // Tab 2: Người lạ
                    ListView {
                        model: friendHandlers.nonFriendUsers
                        clip: true
                        delegate: ItemDelegate {
                            width: parent.width
                            height: 50
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 10
                                Rectangle {
                                    width: 32; height: 32; radius: 16
                                    color: modelData.status === 1 ? "#4CAF50" : "#cccccc"
                                    Text { 
                                        anchors.centerIn: parent 
                                        text: modelData.username ? modelData.username.charAt(0).toUpperCase() : "?" 
                                        color: "white"
                                    }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 0
                                    Label { 
                                        text: modelData.username
                                        font.bold: true
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                    Label {
                                        text: modelData.status === 1 ? "Trực tuyến" : "Ngoại tuyến"
                                        font.pixelSize: 12
                                        color: "gray"
                                    }
                                }
                            }
                        }
                        ScrollIndicator.vertical: ScrollIndicator { }
                    }

                    // Tab 3: Nhóm
                    ListView {
                        model: 3
                        clip: true
                        delegate: ItemDelegate {
                            width: parent.width
                            height: 50
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 10
                                Rectangle {
                                    width: 32; height: 32; radius: 4
                                    color: "#cccccc"
                                    Text { anchors.centerIn: parent; text: "G" }
                                }
                                Label { 
                                    text: "Nhóm " + (index + 1)
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }
                            }
                        }
                        ScrollIndicator.vertical: ScrollIndicator { }
                    }
                }
            }
        }

        // --- CENTER CHAT AREA (Khung chat) ---
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Chat Header
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    color: "white"
                    border.color: "#e1dfdd"
                    border.width: 1
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10
                        
                        Rectangle {
                            width: 40; height: 40; radius: 20
                            color: "#0078d4"
                            Text { 
                                anchors.centerIn: parent
                                text: "A"
                                color: "white"
                                font.bold: true
                            }
                        }

                        ColumnLayout {
                            spacing: 0
                            Label {
                                text: "Tên người hoặc nhóm"
                                font.bold: true
                                font.pixelSize: 16
                            }
                            Label {
                                text: "Trạng thái: Online"
                                font.pixelSize: 12
                                color: "gray"
                                visible: contactTabBar.currentIndex !== 2
                            }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }

                // Message History
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#f3f2f1"
                    
                    ListView {
                        anchors.fill: parent
                        anchors.margins: 10
                        clip: true
                        model: 0
                        spacing: 10
                        
                        Label {
                            anchors.centerIn: parent
                            text: "Chưa có tin nhắn nào"
                            color: "gray"
                            visible: parent.count === 0
                            font.pixelSize: 16
                        }
                    }
                }

                // Chat Footer (Input)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    color: "white"
                    border.color: "#e1dfdd"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        TextField {
                            placeholderText: "Nhập tin nhắn..."
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            selectByMouse: true
                            background: Rectangle {
                                color: "#f3f2f1"
                                radius: 20
                                border.color: parent.activeFocus ? "#0078d4" : "transparent"
                            }
                        }
                        Button {
                            text: "Gửi"
                            highlighted: true
                            Layout.preferredHeight: 40
                            background: Rectangle {
                                color: parent.down ? "#005a9e" : "#0078d4"
                                radius: 20
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }
            }
        }

        // --- RIGHT SIDEBAR (Thao tác) ---
        Rectangle {
            Layout.preferredWidth: 250
            Layout.fillHeight: true
            color: "white"
            border.color: "#e1dfdd"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                Label {
                    text: "Thông tin"
                    font.bold: true
                    font.pixelSize: 18
                    Layout.alignment: Qt.AlignHCenter
                }

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 100; height: 100; radius: 50
                    color: "#cccccc"
                    Text {
                        anchors.centerIn: parent
                        text: "Avatar"
                        color: "#666666"
                    }
                }

                Label {
                    text: "Tên hiển thị"
                    font.bold: true
                    font.pixelSize: 16
                    Layout.alignment: Qt.AlignHCenter
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#e1dfdd"
                }

                Label {
                    text: "Thao tác"
                    font.bold: true
                    font.pixelSize: 14
                }

                Button {
                    text: "Hủy kết bạn"
                    Layout.fillWidth: true
                    visible: contactTabBar.currentIndex === 0
                    flat: true
                    contentItem: Text {
                        text: parent.text
                        color: "red"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        border.color: "red"
                        border.width: 1
                        radius: 4
                        color: parent.down ? "#ffe6e6" : "transparent"
                    }
                }

                Button {
                    text: "Thêm bạn"
                    Layout.fillWidth: true
                    visible: contactTabBar.currentIndex === 1
                    highlighted: true
                }

                Button {
                    text: "Thêm bạn vào nhóm"
                    Layout.fillWidth: true
                    visible: contactTabBar.currentIndex === 2
                    highlighted: true
                }
                
                Item { Layout.fillHeight: true }
            }
        }
    }
}