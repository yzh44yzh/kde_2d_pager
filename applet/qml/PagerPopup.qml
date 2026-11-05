pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts

PagerGrid {
    id: root
    required property bool popupOpen
    signal closeRequested()

    Layout.minimumWidth: minimumWidth
    Layout.minimumHeight: minimumHeight
    Layout.preferredWidth: implicitWidth
    Layout.preferredHeight: implicitHeight

    function focusDestination() {
        if (!popupOpen)
            return
        forceActiveFocus(Qt.PopupFocusReason)
        focusCurrentCell()
    }

    onPopupOpenChanged: if (popupOpen) Qt.callLater(focusDestination)
    onActivityCountChanged: if (popupOpen) Qt.callLater(focusDestination)
    onDesktopCountChanged: if (popupOpen) Qt.callLater(focusDestination)
    Component.onCompleted: if (popupOpen) Qt.callLater(focusDestination)
    Keys.onEscapePressed: closeRequested()

    Connections {
        target: root.backend
        function onSwitchSucceeded() {
            if (root.popupOpen)
                root.closeRequested()
        }
    }
}
