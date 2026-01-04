import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root
    property string sourcePath: ""
    property string sourceName: ""
    property string currentMovePath: ""
    
    // Signal emitted when move is confirmed
    signal moveConfirmed(string src, string dest)

    width: 400
    height: 500
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    onOpened: {
        currentMovePath = "" // Start at root or maybe same as source?
        // Let's start at root for simplicity
        fileSharingHandlers.browseDirectories(winSockClient.getGroupId(), currentMovePath)
    }

    Rectangle {
        anchors.fill: parent
        color: "white"
        border.color: "lightgray"
        radius: 5

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            
            Label { 
                text: "Di chuyển: " + sourceName
                font.bold: true
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }

            Label {
                text: "Đến: " + (currentMovePath === "" ? "Thư mục gốc" : currentMovePath)
                color: "gray"
                Layout.fillWidth: true
                elide: Text.ElideStart
            }

            // Navigation
            RowLayout {
                Button {
                    text: "Lên một cấp"
                    enabled: currentMovePath !== ""
                    onClicked: {
                        let parts = currentMovePath.split("/")
                        parts.pop() // Remove last segment
                        currentMovePath = parts.join("/")
                        fileSharingHandlers.browseDirectories(winSockClient.getGroupId(), currentMovePath)
                    }
                }
                Item { Layout.fillWidth: true }
            }

            ListView {
                id: folderList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: fileSharingHandlers.moveDialogFolders

                delegate: ItemDelegate {
                    width: parent.width
                    text: modelData.name
                    icon.name: "folder" // If using icon theme, or use custom icon
                    
                    // Simple icon workaround if standard icon not available
                    contentItem: RowLayout {
                        spacing: 10
                        Label { text: "📁"; font.pixelSize: 16 }
                        Label { 
                            text: modelData.name
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }

                    onClicked: {
                        // Navigate into folder
                        root.currentMovePath = modelData.path
                        fileSharingHandlers.browseDirectories(winSockClient.getGroupId(), root.currentMovePath)
                    }
                }
                
                Label {
                    anchors.centerIn: parent
                    text: "Không có thư mục con"
                    visible: folderList.count === 0
                    color: "gray"
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                Button {
                    text: "Hủy"
                    onClicked: root.close()
                }
                Button {
                    text: "Chuyển đến đây"
                    highlighted: true
                    onClicked: {
                        // Source path contains filename, Dest path is directory
                        // We need full dest path = currentMovePath + sourceName
                        // Actually moveItem expects src/dest full paths.
                        // Wait, moveItem(groupID, srcPath, destPath)
                        // srcPath = "foo/bar.txt", destPath = "baz/bar.txt"
                        
                        let dest = currentMovePath
                        if(dest !== "") dest += "/"
                        dest += sourceName
                        
                        // Check if trying to move to same location?
                        // srcPath might be "foo.txt" and dest "foo.txt" if currentMovePath is empty.
                        if (sourcePath === dest) {
                            // Indicate error or just close?
                            console.log("Same location")
                        } else {
                            moveConfirmed(sourcePath, dest)
                        }
                        root.close()
                    }
                }
            }
        }
    }
}
