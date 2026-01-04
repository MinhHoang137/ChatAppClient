import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs as QD
import Client

Page {
    property string username: ""
    property string selectedUserName: "Chưa chọn"
    property int selectedUserStatus: -1 // -1: none, 0: offline, 1: online
    
    signal logout()

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

            Button {
                text: "Đăng xuất"
                flat: true
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.down ? "#005a9e" : (parent.hovered ? "#006cc1" : "transparent")
                }
                onClicked: {
                    winSockClient.setUserId(0)
                    winSockClient.setGroupId(0)
                    winSockClient.setTargetId(0)
                    logout()
                }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Overlay cảnh báo khi mất kết nối
        Rectangle {
            visible: !winSockClient.isConnected
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 36
            z: 10
            color: "#FFF4E5"
            border.color: "#FFD699"
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 10
                Label { text: "Mất kết nối tới server"; color: "#8A6D3B" }
                Item { Layout.fillWidth: true }
                Button { text: "Kết nối lại"; onClicked: winSockClient.reconnect() }
            }
        }

        // --- LEFT SIDEBAR (Danh sách) ---
        Rectangle {
            Layout.preferredWidth: 360
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
                    TabButton { text: "Chia sẻ file"; width: implicitWidth }

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
                        } else if (currentIndex === 3) {
                            groupHandlers.fetchGroups()
                            // Prefetch lists used by Manage Members dialog
                            friendHandlers.fetchFriends()
                            friendHandlers.fetchNonFriendUsers()
                        } else if (currentIndex === 4) {
                            groupHandlers.fetchGroups()
                            if (winSockClient.groupId !== 0) {
                                fileSharingHandlers.listFiles(winSockClient.getGroupId(), "")
                            }
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
                        enabled: winSockClient.isConnected
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
                        enabled: winSockClient.isConnected
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
                        enabled: winSockClient.isConnected
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
                                    }
                                }
                            }
                        }
                        ScrollIndicator.vertical: ScrollIndicator { }
                    }

                    // Tab 4: Nhóm
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            TextField {
                                id: newGroupName
                                placeholderText: "Tên nhóm mới"
                                Layout.fillWidth: true
                            }
                            Button {
                                text: "Tạo nhóm"
                                highlighted: true
                                onClicked: {
                                    if (newGroupName.text.trim() !== "") {
                                        groupHandlers.createGroup(newGroupName.text)
                                        newGroupName.text = ""
                                        groupHandlers.fetchGroups()
                                    }
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            Button {
                                text: "Quản lý thành viên"
                                enabled: winSockClient.isConnected && winSockClient.groupId !== 0
                                onClicked: {
                                    friendHandlers.fetchFriends()
                                    friendHandlers.fetchNonFriendUsers()
                                    groupHandlers.loadGroupMembers(winSockClient.getGroupId())
                                    // Also fetch requests if leader? let's fetch anyway
                                    groupHandlers.getGroupRequests(winSockClient.getGroupId())
                                    groupMemberDialog.open() // wait, groupMemberDialog is not defined?
                                    // Ah, I see "Right content" is managed by "groupMemberDialog"? 
                                    // No, the code I saw earlier was just "Right Content". 
                                    // Maybe line 318 `groupMemberDialog.open()` implies there is a dialog.
                                    // But earlier view showed "Right Content" as part of the main layout?
                                    // Let me re-read line 1003. "=== RIGHT CONTENT: Quản lý thành viên ==="
                                    // It seems the "Right Content" IS the member management view.
                                    // So the button at 318 might be opening a Popup or it's controlling visibility?
                                    // If line 1003 is a ColumnLayout in a SplitView (not shown), then it's always visible?
                                    // Or maybe "groupMemberDialog" is the ID of the right drawer/dialog?
                                    // Let's assume for now I just add the button "Tìm nhóm".
                                }
                            }
                            Button {
                                text: "Tìm nhóm"
                                onClicked: {
                                    groupHandlers.exploreGroups()
                                    exploreGroupsDialog.open()
                                }
                            }
                            Item { Layout.fillWidth: true }
                            Button {
                                text: "Làm mới"
                                onClicked: groupHandlers.fetchGroups()
                            }
                        }

                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            model: groupHandlers.groups
                            clip: true
                            delegate: ItemDelegate {
                                width: parent.width
                                height: 50
                                highlighted: ListView.isCurrentItem
                                onClicked: {
                                    ListView.view.currentIndex = index
                                    winSockClient.setGroupId(modelData.groupID)
                                    winSockClient.setTargetId(0)
                                    selectedUserName = modelData.groupName
                                    selectedUserStatus = -1
                                    groupHandlers.loadGroupMessages(modelData.groupID)
                                    groupHandlers.loadGroupMembers(modelData.groupID)
                                    fileSharingHandlers.listFiles(modelData.groupID, "")
                                }
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
                                        text: modelData.groupName
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }
                                    Button {
                                        text: "Rời nhóm"
                                        onClicked: {
                                            groupHandlers.leaveGroup(modelData.groupID)
                                            groupHandlers.fetchGroups()
                                        }
                                    }
                                }
                            }
                            ScrollIndicator.vertical: ScrollIndicator { }
                        }
                    }

                    // Tab 5: Chia sẻ file
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 8
                        visible: contactTabBar.currentIndex === 4

                        // Breadcrumb / Path
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            Label {
                                text: "Đường dẫn:"
                                font.bold: true
                            }
                            Label {
                                text: fileSharingHandlers.currentPath === "" ? "/" : "/" + fileSharingHandlers.currentPath
                                Layout.fillWidth: true
                                elide: Text.ElideMiddle
                            }
                            Button {
                                text: "Lên"
                                enabled: fileSharingHandlers.currentPath !== ""
                                onClicked: {
                                    // Calculate parent path
                                    var path = fileSharingHandlers.currentPath
                                    var lastSlash = path.lastIndexOf('/')
                                    if (lastSlash !== -1) {
                                        path = path.substring(0, lastSlash)
                                    } else {
                                        path = ""
                                    }
                                    fileSharingHandlers.listFiles(winSockClient.getGroupId(), path)
                                }
                            }
                        }

                        // Toolbar
                        RowLayout {
                             Layout.fillWidth: true
                             spacing: 8
                             enabled: winSockClient.groupId !== 0

                             Button {
                                 text: "Tải lên"
                                 onClicked: fileFileDialog.open()
                             }
                             Button {
                                 text: "Tạo thư mục"
                                 onClicked: createFolderDialog.open()
                             }
                             Button {
                                 text: "Làm mới"
                                 onClicked: fileSharingHandlers.listFiles(winSockClient.getGroupId(), fileSharingHandlers.currentPath)
                             }
                             
                             Button {
                                 text: "Dán"
                                 visible: fileSharingHandlers.clipboardAction !== ""
                                 onClicked: {
                                    if (fileSharingHandlers.clipboardAction === "copy") {
                                        fileSharingHandlers.copyItem(winSockClient.getGroupId(), fileSharingHandlers.clipboardPath, fileSharingHandlers.currentPath + "/" + fileSharingHandlers.clipboardPath.split('/').pop())
                                    } else if (fileSharingHandlers.clipboardAction === "move") {
                                        fileSharingHandlers.moveItem(winSockClient.getGroupId(), fileSharingHandlers.clipboardPath, fileSharingHandlers.currentPath + "/" + fileSharingHandlers.clipboardPath.split('/').pop())
                                    }
                                    // Should refresh automatically or wait for result
                                    // fileSharingHandlers.listFiles(winSockClient.getGroupId(), fileSharingHandlers.currentPath)
                                 }
                             }

                             Item { Layout.fillWidth: true }
                        }
                        
                        Label {
                             text: "Chọn nhóm từ danh sách để xem file"
                             visible: winSockClient.groupId === 0
                             color: "red"
                             font.italic: true
                             Layout.alignment: Qt.AlignHCenter
                        }

                        // File List
                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            model: winSockClient.groupId !== 0 ? fileSharingHandlers.files : []
                            delegate: ItemDelegate {
                                width: parent.width
                                height: 40
                                
                                MouseArea {
                                    anchors.fill: parent
                                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                                    onClicked: (mouse) => {
                                        if (mouse.button === Qt.RightButton) {
                                           contextMenu.popup()
                                        } else {
                                            if (modelData.isDir) {
                                                var newPath = fileSharingHandlers.currentPath === "" ? modelData.name : fileSharingHandlers.currentPath + "/" + modelData.name
                                                fileSharingHandlers.listFiles(winSockClient.getGroupId(), newPath)
                                            }
                                        }
                                    }
                                }

                                Menu {
                                    id: contextMenu
                                    MenuItem {
                                        text: "Đổi tên"
                                        onTriggered: {
                                            renameDialog.currentName = modelData.name
                                            renameDialog.currentPath = fileSharingHandlers.currentPath
                                            renameDialog.open()
                                        }
                                    }
                                    MenuItem {
                                        text: "Sao chép"
                                        onTriggered: {
                                            var path = fileSharingHandlers.currentPath === "" ? modelData.name : fileSharingHandlers.currentPath + "/" + modelData.name
                                            fileSharingHandlers.setClipboard("copy", path)
                                        }
                                    }
                                    MenuItem {
                                        text: "Di chuyển đến..."
                                        onTriggered: {
                                            var path = fileSharingHandlers.currentPath === "" ? modelData.name : fileSharingHandlers.currentPath + "/" + modelData.name
                                            moveToDialog.sourcePath = path
                                            moveToDialog.sourceName = modelData.name
                                            moveToDialog.open()
                                        }
                                    }
                                    MenuItem {
                                        text: "Xóa"
                                        onTriggered: {
                                            fileSharingHandlers.deleteItem(winSockClient.getGroupId(), fileSharingHandlers.currentPath, modelData.name, modelData.isDir)
                                            // Auto refresh likely triggered by operationResult or manual
                                            fileSharingHandlers.listFiles(winSockClient.getGroupId(), fileSharingHandlers.currentPath)
                                        }
                                    }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 5
                                    spacing: 10

                                    Image {
                                        source: modelData.isDir ? "qrc:/icons/folder.png" : "qrc:/icons/file.png"
                                        Layout.preferredWidth: 24
                                        Layout.preferredHeight: 24
                                        // Fallback text if icon missing
                                        Text {
                                            visible: parent.status !== Image.Ready
                                            text: modelData.isDir ? "[D]" : "[F]"
                                            anchors.centerIn: parent
                                        }
                                    }

                                    Label {
                                        text: modelData.name
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                        font.bold: modelData.isDir
                                    }

                                    Label {
                                        text: modelData.isDir ? "" : (modelData.size + " B")
                                        Layout.preferredWidth: 80
                                        horizontalAlignment: Text.AlignRight
                                    }

                                    Button {
                                        text: "Tải xuống"
                                        visible: !modelData.isDir
                                        onClicked: {
                                            saveFileDialog.targetFileName = modelData.name
                                            saveFileDialog.open()
                                        }
                                    }
                                }

                                onClicked: {
                                    if (modelData.isDir) {
                                        var newPath = fileSharingHandlers.currentPath
                                        if (newPath !== "") newPath += "/"
                                        newPath += modelData.name
                                        fileSharingHandlers.listFiles(winSockClient.getGroupId(), newPath)
                                    }
                                }
                            }
                        }
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
                        model: winSockClient.groupId !== 0 ? groupHandlers.groupMessages : friendHandlers.messages
                        spacing: 10
                        enabled: winSockClient.isConnected
                        
                        delegate: Item {
                            width: messageListView.width
                            height: msgColumn.height

                            Column {
                                id: msgColumn
                                anchors.right: modelData.senderID === winSockClient.userId ? parent.right : undefined
                                anchors.left: modelData.senderID !== winSockClient.userId ? parent.left : undefined
                                spacing: 2
                                
                                // Show sender name for group messages (not self)
                                Text {
                                    text: modelData.senderName || ""
                                    font.pixelSize: 11
                                    color: "#666"
                                    visible: winSockClient.groupId !== 0 && modelData.senderID !== winSockClient.userId && (modelData.senderName || "") !== ""
                                    leftPadding: 8
                                }

                                Rectangle {
                                    id: msgRect
                                    color: modelData.senderID === winSockClient.userId ? "#0078d4" : "white"
                                    radius: 10
                                    width: Math.min(msgText.implicitWidth + 24, messageListView.width * 0.7)
                                    height: msgText.implicitHeight + 20
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
                            enabled: winSockClient.isConnected
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
                            enabled: winSockClient.isConnected
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
                                    if (winSockClient.groupId !== 0) {
                                        groupHandlers.sendGroupMessage(messageInput.text)
                                    } else {
                                        friendHandlers.sendMessage(messageInput.text)
                                    }
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
                    text: "Quản lý thành viên nhóm"
                    Layout.fillWidth: true
                    visible: contactTabBar.currentIndex === 3
                    highlighted: true
                    enabled: winSockClient.isConnected
                    onClicked: {
                        friendHandlers.fetchFriends();
                        friendHandlers.fetchNonFriendUsers();
                        groupHandlers.loadGroupMembers(winSockClient.getGroupId());
                        groupMemberDialog.open();
                    }
                }
                
                Item { Layout.fillHeight: true }
            }
        }
    }

    // File Dialogs
    QD.FileDialog {
        id: fileFileDialog
        title: "Chọn file để tải lên"
        fileMode: QD.FileDialog.OpenFile
        onAccepted: {
            fileSharingHandlers.uploadFile(winSockClient.getGroupId(), fileSharingHandlers.currentPath, selectedFile)
        }
    }

    QD.FileDialog {
        id: saveFileDialog
        title: "Lưu file"
        fileMode: QD.FileDialog.SaveFile
        property string targetFileName: ""
        onAccepted: {
             fileSharingHandlers.downloadFile(winSockClient.getGroupId(), fileSharingHandlers.currentPath, targetFileName, selectedFile)
        }
    }

    Dialog {
        id: createFolderDialog
        title: "Tạo thư mục mới"
        standardButtons: Dialog.Ok | Dialog.Cancel
        
        ColumnLayout {
            TextField {
                id: newFolderName
                placeholderText: "Tên thư mục"
            }
        }
        onAccepted: {
            if (newFolderName.text !== "") {
                fileSharingHandlers.createFolder(winSockClient.getGroupId(), fileSharingHandlers.currentPath, newFolderName.text)
                newFolderName.text = ""
                // Refresh
                fileSharingHandlers.listFiles(winSockClient.getGroupId(), fileSharingHandlers.currentPath)
            }
        }
    }

    // Dialog quản lý thành viên nhóm
    Dialog {
        id: groupMemberDialog
        title: "Quản lý thành viên nhóm"
        modal: true
        standardButtons: Dialog.Close
        width: 700
        height: 560
        visible: false

        onVisibleChanged: {
            if (visible) {
                groupHandlers.fetchGroups()
                friendHandlers.fetchFriends()
                friendHandlers.fetchNonFriendUsers()
                if (winSockClient.groupId !== 0) {
                    groupHandlers.loadGroupMembers(winSockClient.getGroupId())
                }
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            // === LEFT SIDEBAR: Danh sách nhóm ===
            Rectangle {
                Layout.preferredWidth: 180
                Layout.fillHeight: true
                color: "#f3f2f1"
                border.color: "#e1dfdd"
                radius: 6

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 6

                    Label {
                        text: "Danh sách nhóm"
                        font.bold: true
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#e1dfdd"
                    }

                    ListView {
                        id: dialogGroupList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: groupHandlers.groups
                        currentIndex: -1
                        delegate: ItemDelegate {
                            width: dialogGroupList.width
                            height: 44
                            highlighted: winSockClient.groupId === modelData.groupID
                            onClicked: {
                                winSockClient.setGroupId(modelData.groupID)
                                groupHandlers.loadGroupMembers(modelData.groupID)
                            }
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 6
                                Rectangle {
                                    width: 28; height: 28; radius: 4
                                    color: winSockClient.groupId === modelData.groupID ? "#1976D2" : "#cccccc"
                                    Text { anchors.centerIn: parent; text: "G"; color: "white"; font.bold: true }
                                }
                                Label {
                                    text: modelData.groupName
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                    font.bold: winSockClient.groupId === modelData.groupID
                                    color: winSockClient.groupId === modelData.groupID ? "#1976D2" : "black"
                                }
                            }
                        }
                        ScrollIndicator.vertical: ScrollIndicator {}

                        Label {
                            anchors.centerIn: parent
                            text: groupHandlers.groups.length === 0 ? "Chưa có nhóm" : ""
                            visible: groupHandlers.groups.length === 0
                            color: "gray"
                        }
                    }
                }
            }

            // === RIGHT CONTENT: Quản lý thành viên ===
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                // Hint banner when no group is selected
                Rectangle {
                    visible: winSockClient.groupId === 0
                    Layout.fillWidth: true
                    height: 32
                    color: "#FFF4E5"
                    border.color: "#FFD699"
                    radius: 4
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        Label { text: "Chọn một nhóm ở bên trái để quản lý thành viên."; color: "#8A6D3B"; wrapMode: Text.Wrap }
                    }
                }

                TabBar {
                    id: addMemberTabs
                    Layout.fillWidth: true
                    TabButton { text: "Bạn bè" }
                    TabButton { text: "Người lạ" }
                    TabButton { text: "Yêu cầu" }
                    onCurrentIndexChanged: {
                         if (currentIndex === 2) {
                             if (winSockClient.groupId !== 0) {
                                 groupHandlers.getGroupRequests(winSockClient.getGroupId())
                             }
                         } else if (currentIndex === 0) {
                             friendHandlers.fetchFriends()
                         } else if (currentIndex === 1) {
                             friendHandlers.fetchNonFriendUsers()
                         }
                    }
                }

                TextField {
                    id: searchField
                    placeholderText: "Tìm kiếm người dùng"
                    Layout.fillWidth: true
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: addMemberTabs.currentIndex === 0 ? friendHandlers.friends : (addMemberTabs.currentIndex === 1 ? friendHandlers.nonFriendUsers : groupHandlers.groupRequests)
                    delegate: ItemDelegate {
                        width: parent.width
                        height: 50
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 8
                            Label { text: modelData.username; Layout.fillWidth: true; elide: Text.ElideRight }
                            Button {
                                text: "Thêm"
                                visible: addMemberTabs.currentIndex < 2
                                enabled: winSockClient.isConnected && winSockClient.groupId !== 0
                                onClicked: {
                                    groupHandlers.addUserToGroup(winSockClient.getGroupId(), modelData.userID)
                                }
                            }
                            Button {
                                text: "Duyệt"
                                visible: addMemberTabs.currentIndex === 2
                                onClicked: groupHandlers.respondToRequest(modelData.requestID, 1)
                            }
                            Button {
                                text: "Từ chối"
                                visible: addMemberTabs.currentIndex === 2
                                onClicked: groupHandlers.respondToRequest(modelData.requestID, 2)
                            }
                        }
                        visible: modelData.username.toLowerCase().indexOf(searchField.text.toLowerCase()) !== -1
                    }
                    ScrollIndicator.vertical: ScrollIndicator {}

                    Label {
                        anchors.centerIn: parent
                        text: winSockClient.isConnected ? "Đang tải danh sách hoặc chưa có dữ liệu" : "Chưa kết nối tới server"
                        color: "gray"
                        visible: (addMemberTabs.currentIndex === 0 ? friendHandlers.friends.length : friendHandlers.nonFriendUsers.length) === 0
                    }
                    BusyIndicator {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 8
                        running: winSockClient.isConnected
                        visible: (addMemberTabs.currentIndex === 0 ? friendHandlers.friends.length : friendHandlers.nonFriendUsers.length) === 0
                    }
                }

                Label { text: "Thành viên hiện tại"; font.bold: true; Layout.topMargin: 8 }
                ListView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 180
                    clip: true
                    model: groupHandlers.groupMembers
                    delegate: ItemDelegate {
                        width: parent.width
                        height: 50
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 8
                            Label { text: modelData.username; Layout.fillWidth: true; elide: Text.ElideRight }
                            Button {
                                text: "Xóa"
                                enabled: winSockClient.isConnected && winSockClient.groupId !== 0
                                onClicked: {
                                    groupHandlers.removeUserFromGroup(winSockClient.getGroupId(), modelData.userID)
                                }
                            }
                        }
                    }
                    ScrollIndicator.vertical: ScrollIndicator {}
                    Label {
                        anchors.centerIn: parent
                        text: groupHandlers.groupMembers.length === 0 ? "Nhóm chưa có thành viên hoặc đang tải" : ""
                        visible: groupHandlers.groupMembers.length === 0
                        color: "gray"
                    }
                }
            }
        }
    }

    RenameDialog {
        id: renameDialog
        anchors.centerIn: parent
        onRenameConfirmed: (oldName, newName) => {
            fileSharingHandlers.renameItem(winSockClient.getGroupId(), fileSharingHandlers.currentPath, oldName, newName)
            fileSharingHandlers.listFiles(winSockClient.getGroupId(), fileSharingHandlers.currentPath)
        }
    }

    MoveToDialog {
        id: moveToDialog
        anchors.centerIn: parent
        onMoveConfirmed: (src, dest) => {
             fileSharingHandlers.moveItem(winSockClient.getGroupId(), src, dest)
             fileSharingHandlers.listFiles(winSockClient.getGroupId(), fileSharingHandlers.currentPath)
        }
    }

    QD.MessageDialog {
        id: errorDialog
        title: "Thông báo"
        buttons: QD.MessageDialog.Ok
    }

    Connections {
        target: fileSharingHandlers
        function onFileOperationFinished(success, message) {
            if (!success) {
                errorDialog.text = message
                errorDialog.open()
            }
        }
    }

    QD.MessageDialog {
        id: requestJoinResultDialog
        title: "Thông báo"
        buttons: QD.MessageDialog.Ok
    }

    Connections {
        target: groupHandlers
        function onRequestJoinFinished(success, message) {
            requestJoinResultDialog.text = message
            requestJoinResultDialog.open()
        }
    }

    Popup {
        id: exploreGroupsDialog
        parent: Overlay.overlay
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        width: 400
        height: 500
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Rectangle {
            anchors.fill: parent
            color: "white"
            border.color: "lightgray"
            radius: 5

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                Label { text: "Tìm kiếm nhóm"; font.bold: true; font.pixelSize: 16 }
                
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: groupHandlers.allGroups
                    delegate: ItemDelegate {
                        width: parent.width
                        height: 50
                        RowLayout {
                            anchors.fill: parent
                            spacing: 10
                            Label { text: modelData.groupName; Layout.fillWidth: true }
                            Button {
                                text: "Xin vào"
                                onClicked: groupHandlers.requestJoinGroup(modelData.groupID)
                            }
                        }
                    }
                    Label {
                        anchors.centerIn: parent
                        text: "Không có nhóm nào để tham gia."
                        visible: groupHandlers.allGroups.length === 0
                        color: "gray"
                    }
                }
                
                Button {
                    text: "Đóng"
                    Layout.alignment: Qt.AlignRight
                    onClicked: exploreGroupsDialog.close()
                }
            }
        }
    }
}
