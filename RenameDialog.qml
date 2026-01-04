
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: "Đổi tên"
    standardButtons: Dialog.Ok | Dialog.Cancel
    
    property string currentName: ""
    property string currentPath: "" // Full path relative to group
    
    // Signal for parent to handle
    signal renameConfirmed(string oldName, string newName)

    ColumnLayout {
        TextField {
            id: nameField
            text: root.currentName
            selectByMouse: true
            Layout.preferredWidth: 200
            onAccepted: root.accept()
        }
    }
    
    onOpened: {
        nameField.text = currentName
        nameField.forceActiveFocus()
        nameField.selectAll()
    }
    
    onAccepted: {
        if (nameField.text !== "" && nameField.text !== currentName) {
            renameConfirmed(currentName, nameField.text)
        }
    }
}
