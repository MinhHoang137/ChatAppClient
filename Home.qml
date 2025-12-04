import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client

Page {
    property string username: ""
    property string selectedUserName: "Chưa chọn"
    property int selectedUserStatus: -1 // -1: none, 0: offline, 1: online

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
                    TabButton { text: "Yêu cầu"; width: implicitWidth }
                    TabButton { text: "Nhóm"; width: implicitWidth }

                    Component.onCompleted: {
                        if (currentIndex === 0) {
                            friendHandlers.fetchFriends()
                        }
                    }

                    onCurrentIndexChanged: {
                        if (currentIndex === 0) {
                            friendHandlers.fetchFriends()
                        } else if (currentIndex === 1) {
                            friendHandlers.fetchNonFriendUsers()
                        } else if (currentIndex === 2) {
                            friendHandlers.fetchFriendRequests()
                        }
                    }
                }

                StackLayout {
                    currentIndex: contactTabBar.currentIndex
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    // Tab 1: Bạn bè
                    ListView {
                        model: friendHandlers.friends
                        clip: true
                        delegate: ItemDelegate {
                            width: parent.width
                            height: 50
                            highlighted: ListView.isCurrentItem
                            onClicked: {
                                ListView.view.currentIndex = index
                                winSockClient.setTargetId(modelData.userID)
                                selectedUserName = modelData.username
                                selectedUserStatus = modelData.status
                                friendHandlers.loadMessages(modelData.userID)
                                friendHandlers.sendQueryFriendStatus(modelData.userID)
                                console.log("Target ID set to:", modelData.userID)
                            }
                            
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

                    // Tab 2: Người lạ
                    ListView {
                        model: friendHandlers.nonFriendUsers
                        clip: true
                        delegate: ItemDelegate {
                            width: parent.width
                            height: 50
                            highlighted: ListView.isCurrentItem
                            onClicked: {
                                ListView.view.currentIndex = index
                                winSockClient.setTargetId(modelData.userID)
                                selectedUserName = modelData.username
                                selectedUserStatus = modelData.status
                                friendHandlers.loadMessages(modelData.userID)
                                friendHandlers.sendQueryFriendStatus(modelData.userID)
                                console.log("Target ID set to:", modelData.userID)
                            }

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

                    // Tab 3: Yêu cầu kết bạn
                    ListView {
                        model: friendHandlers.friendRequests
                        clip: true
                        delegate: ItemDelegate {
                            width: parent.width
                            height: 50
                            highlighted: ListView.isCurrentItem
                            
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
                                        text: "Yêu cầu kết bạn"
                                        font.pixelSize: 12
                                        color: "gray"
                                    }
                                }
                                Button {
                                    text: "Chấp nhận"
                                    onClicked: {
                                        friendHandlers.sendAcceptFriendRequest(modelData.userID)
                                        console.log("Accepted friend request from:", modelData.userID)
                                    }
                                }
                            }
                        }
                        ScrollIndicator.vertical: ScrollIndicator { }
                    }

                    // Tab 4: Nhóm
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
                            color: selectedUserStatus === 1 ? "#4CAF50" : "#cccccc"
                            Text { 
                                anchors.centerIn: parent
                                text: selectedUserName && selectedUserName !== "Chưa chọn" ? selectedUserName.charAt(0).toUpperCase() : "?"
                                color: "white"
                                font.bold: true
                            }
                        }

                        ColumnLayout {
                            spacing: 0
                            Label {
                                text: selectedUserName
                                font.bold: true
                                font.pixelSize: 16
                            }
                            Label {
                                text: selectedUserStatus === 1 ? "Trực tuyến" : (selectedUserStatus === 0 ? "Ngoại tuyến" : "")
                                font.pixelSize: 12
                                color: "gray"
                                visible: contactTabBar.currentIndex !== 2 && selectedUserStatus !== -1
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
                        id: messageListView
                        anchors.fill: parent
                        anchors.margins: 10
                        clip: true
                        model: friendHandlers.messages
                        spacing: 10
                        
                        delegate: Item {
                            width: parent.width
                            height: msgRect.height

                            Rectangle {
                                id: msgRect
                                color: modelData.senderID === winSockClient.userId ? "#0078d4" : "white"
                                radius: 10
                                width: Math.min(msgText.implicitWidth + 24, parent.width * 0.7)
                                height: msgText.implicitHeight + 20
                                anchors.right: modelData.senderID === winSockClient.userId ? parent.right : undefined
                                anchors.left: modelData.senderID !== winSockClient.userId ? parent.left : undefined
                                border.color: "#e1dfdd"
                                border.width: modelData.senderID !== winSockClient.userId ? 1 : 0

                                Text {
                                    id: msgText
                                    anchors.centerIn: parent
                                    text: modelData.content
                                    color: modelData.senderID === winSockClient.userId ? "white" : "black"
                                    width: parent.width - 24
                                    wrapMode: Text.Wrap
                                }
                            }
                        }

                        onCountChanged: positionViewAtEnd()
                        
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
                            id: messageInput
                            placeholderText: "Nhập tin nhắn..."
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            selectByMouse: true
                            background: Rectangle {
                                color: "#f3f2f1"
                                radius: 20
                                border.color: parent.activeFocus ? "#0078d4" : "transparent"
                            }
                            onAccepted: sendBtn.clicked()
                        }
                        Button {
                            id: sendBtn
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
                            onClicked: {
                                if (messageInput.text.trim() !== "") {
                                    friendHandlers.sendMessage(messageInput.text)
                                    messageInput.text = ""
                                }
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
            visible: contactTabBar.currentIndex !== 2 // Hide in Requests tab
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
                    color: selectedUserStatus === 1 ? "#4CAF50" : "#cccccc"
                    Text {
                        anchors.centerIn: parent
                        text: selectedUserName && selectedUserName !== "Chưa chọn" ? selectedUserName.charAt(0).toUpperCase() : "?"
                        color: "white"
                        font.pixelSize: 40
                        font.bold: true
                    }
                }

                Label {
                    text: selectedUserName
                    font.bold: true
                    font.pixelSize: 16
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: selectedUserStatus === 1 ? "Trực tuyến" : (selectedUserStatus === 0 ? "Ngoại tuyến" : "")
                    color: "gray"
                    Layout.alignment: Qt.AlignHCenter
                    visible: selectedUserStatus !== -1
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
                    onClicked: {
                        friendHandlers.sendUnfriendRequest(winSockClient.getTargetId())
                        // Reset selection
                        selectedUserName = "Chưa chọn"
                        selectedUserStatus = -1
                        winSockClient.setTargetId(0)
                    }
                }

                Button {
                    text: friendHandlers.currentFriendStatus === 0 ? "Đã gửi yêu cầu" : 
                          (friendHandlers.currentFriendStatus === 1 ? "Đã là bạn" : 
                          (friendHandlers.currentFriendStatus === 2 ? "Đã nhận yêu cầu" : "Kết bạn"))
                    Layout.fillWidth: true
                    visible: contactTabBar.currentIndex === 1 // Tab Người lạ
                    enabled: friendHandlers.currentFriendStatus === -1
                    highlighted: true
                    onClicked: {
                        friendHandlers.sendFriendRequest(winSockClient.getTargetId())
                    }
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