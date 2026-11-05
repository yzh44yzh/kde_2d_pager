pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: root
    required property var backend
    readonly property int activityCount: backend.activities.length
    readonly property int desktopCount: backend.desktops.length
    readonly property real gap: Kirigami.Units.smallSpacing
    readonly property real rowLabelWidth: Kirigami.Units.gridUnit * 1.5
    readonly property real columnWidth: Kirigami.Units.gridUnit * 5
    readonly property real cellMinimumHeight: Kirigami.Units.gridUnit * 1.5
    readonly property real minimumWidth: activityCount > 0
        ? rowLabelWidth + activityCount * (columnWidth + gap) : columnWidth * 2
    readonly property real minimumHeight: (Math.max(1, desktopCount) + 1) * cellMinimumHeight
        + Math.max(1, desktopCount) * gap
    readonly property real rowHeight: Math.max(1,
        (height - Math.max(1, desktopCount) * gap) / (Math.max(1, desktopCount) + 1))
    property int focusedCell: 0
    implicitWidth: minimumWidth
    implicitHeight: minimumHeight

    function focusCell(index) {
        if (cells.count === 0)
            return
        focusedCell = Math.max(0, Math.min(cells.count - 1, index))
        cells.itemAt(focusedCell).forceActiveFocus(Qt.TabFocusReason)
    }

    GridLayout {
        id: grid
        anchors.fill: parent
        columns: root.activityCount + 1
        rowSpacing: root.gap
        columnSpacing: root.gap
        visible: root.activityCount > 0 && root.desktopCount > 0

        Controls.Label {
            text: "#"
            horizontalAlignment: Text.AlignHCenter
            Layout.preferredWidth: root.rowLabelWidth
            Layout.preferredHeight: root.rowHeight
            verticalAlignment: Text.AlignVCenter
            Accessible.name: "Virtual desktops"
        }

        Repeater {
            model: root.backend.activities
            delegate: Controls.Label {
                required property var modelData
                text: modelData.name
                textFormat: Text.PlainText
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
                Layout.preferredWidth: root.columnWidth
                Layout.preferredHeight: root.rowHeight
                Controls.ToolTip.visible: headerHover.hovered
                Controls.ToolTip.text: modelData.name
                HoverHandler { id: headerHover }
            }
        }

        Repeater {
            model: root.backend.desktops
            delegate: Controls.Label {
                required property int index
                required property var modelData
                text: index + 1
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.row: index + 1
                Layout.column: 0
                Layout.preferredWidth: root.rowLabelWidth
                Layout.preferredHeight: root.rowHeight
                Controls.ToolTip.visible: rowHover.hovered
                Controls.ToolTip.text: modelData.name
                Accessible.name: modelData.name
                HoverHandler { id: rowHover }
            }
        }

        Repeater {
            id: cells
            model: root.activityCount * root.desktopCount
            delegate: Controls.Button {
                id: cell
                required property int index
                readonly property int activityIndex: index % Math.max(1, root.activityCount)
                readonly property int desktopIndex: Math.floor(index / Math.max(1, root.activityCount))
                readonly property var activity: root.backend.activities[activityIndex] ?? {id: "", name: ""}
                readonly property var desktop: root.backend.desktops[desktopIndex] ?? {id: "", name: ""}
                readonly property bool current: root.backend.currentActivity === activity.id
                    && root.backend.currentDesktop === desktop.id
                readonly property bool pending: root.backend.pending
                    && root.backend.pendingActivity === activity.id
                    && root.backend.pendingDesktop === desktop.id

                objectName: "cell-" + desktopIndex + "-" + activityIndex
                Layout.row: desktopIndex + 1
                Layout.column: activityIndex + 1
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: root.columnWidth
                Layout.preferredHeight: root.rowHeight
                enabled: root.backend.available
                focusPolicy: Qt.StrongFocus
                text: pending ? "…" : (current ? "●" : "")
                Accessible.name: activity.name + ", " + desktop.name
                Accessible.description: pending ? "Switching to this destination"
                    : (current ? "Current destination" : "Switch to this destination")
                Accessible.checkable: true
                Accessible.checked: current
                Controls.ToolTip.visible: hovered || activeFocus
                Controls.ToolTip.text: activity.name + " · " + desktop.name
                    + (pending ? " — switching…" : "")
                onClicked: {
                    forceActiveFocus(Qt.MouseFocusReason)
                    root.backend.switchTo(activity.id, desktop.id)
                }
                onActiveFocusChanged: if (activeFocus) root.focusedCell = index

                Keys.onPressed: event => {
                    let next = index
                    if (event.key === Qt.Key_Left && activityIndex > 0) next--
                    else if (event.key === Qt.Key_Right && activityIndex + 1 < root.activityCount) next++
                    else if (event.key === Qt.Key_Up && desktopIndex > 0) next -= root.activityCount
                    else if (event.key === Qt.Key_Down && desktopIndex + 1 < root.desktopCount) next += root.activityCount
                    else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        cell.clicked()
                        event.accepted = true
                        return
                    } else return
                    root.focusCell(next)
                    event.accepted = true
                }

                background: Rectangle {
                    radius: Kirigami.Units.cornerRadius
                    color: cell.current ? Kirigami.Theme.highlightColor
                        : (cell.hovered ? Kirigami.Theme.alternateBackgroundColor
                                        : Kirigami.Theme.backgroundColor)
                    opacity: cell.enabled ? 1 : 0.5
                    border.width: cell.activeFocus || cell.pending ? 2 : 1
                    border.color: cell.activeFocus || cell.pending
                        ? Kirigami.Theme.focusColor : Kirigami.Theme.disabledTextColor
                }
                contentItem: Controls.Label {
                    text: cell.text
                    color: cell.current ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    Controls.Label {
        anchors.fill: parent
        visible: root.activityCount === 0 || root.desktopCount === 0
        text: root.backend.status || "Waiting for activity and desktop information…"
        textFormat: Text.PlainText
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    // A passive message leaves the actual position and destination cells visible.
    Controls.ToolTip {
        parent: root
        visible: root.backend.error.length > 0
        text: root.backend.error
        timeout: -1
        contentItem: Controls.Label {
            text: root.backend.error
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            Accessible.role: Accessible.AlertMessage
        }
    }
}
