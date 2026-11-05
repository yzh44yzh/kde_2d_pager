pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Rectangle {
    id: root
    required property var backend
    readonly property int activityCount: backend.activities.length
    readonly property int desktopCount: backend.desktops.length
    readonly property real gap: Kirigami.Units.smallSpacing
    readonly property real framePadding: gap + border.width
    readonly property real rowLabelWidth: Kirigami.Units.gridUnit * 5
    readonly property real columnWidth: Kirigami.Units.gridUnit * 2
    readonly property real cellMinimumHeight: Kirigami.Units.gridUnit * 1.5
    readonly property real minimumWidth: activityCount > 0
        ? rowLabelWidth + desktopCount * (columnWidth + gap) + framePadding * 2
        : rowLabelWidth * 2 + framePadding * 2
    readonly property real minimumHeight: (Math.max(1, activityCount) + 1) * cellMinimumHeight
        + Math.max(1, activityCount) * gap + framePadding * 2
    readonly property real rowHeight: Math.max(1,
        (height - framePadding * 2 - Math.max(1, activityCount) * gap)
            / (Math.max(1, activityCount) + 1))
    property int focusedCell: 0
    implicitWidth: minimumWidth
    implicitHeight: minimumHeight
    color: Qt.rgba(Kirigami.Theme.backgroundColor.r, Kirigami.Theme.backgroundColor.g,
                   Kirigami.Theme.backgroundColor.b, 1)
    radius: Kirigami.Units.cornerRadius
    border.width: 1
    border.color: Kirigami.Theme.disabledTextColor

    function focusCell(index) {
        if (cells.count === 0)
            return
        focusedCell = Math.max(0, Math.min(cells.count - 1, index))
        cells.itemAt(focusedCell).forceActiveFocus(Qt.TabFocusReason)
    }

    function focusCurrentCell() {
        let activity = -1
        let desktop = -1
        for (let i = 0; i < activityCount; ++i)
            if (backend.activities[i].id === backend.currentActivity) activity = i
        for (let i = 0; i < desktopCount; ++i)
            if (backend.desktops[i].id === backend.currentDesktop) desktop = i
        focusCell(activity >= 0 && desktop >= 0 ? activity * desktopCount + desktop : 0)
    }

    GridLayout {
        id: grid
        anchors.fill: parent
        anchors.margins: root.framePadding
        columns: root.desktopCount + 1
        rowSpacing: root.gap
        columnSpacing: root.gap
        visible: root.activityCount > 0 && root.desktopCount > 0

        Controls.Label {
            text: "Activity"
            horizontalAlignment: Text.AlignLeft
            Layout.preferredWidth: root.rowLabelWidth
            Layout.preferredHeight: root.rowHeight
            verticalAlignment: Text.AlignVCenter
            Accessible.name: "Activities and virtual desktops"
        }

        Repeater {
            model: root.backend.desktops
            delegate: Controls.Label {
                required property int index
                required property var modelData
                text: index + 1
                textFormat: Text.PlainText
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
                Layout.row: 0
                Layout.column: index + 1
                Layout.preferredWidth: root.columnWidth
                Layout.preferredHeight: root.rowHeight
                Controls.ToolTip.visible: headerHover.hovered
                Controls.ToolTip.text: modelData.name
                Accessible.name: modelData.name
                HoverHandler { id: headerHover }
            }
        }

        Repeater {
            model: root.backend.activities
            delegate: Controls.Label {
                required property int index
                required property var modelData
                text: modelData.name
                textFormat: Text.PlainText
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
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
                readonly property int activityIndex: Math.floor(index / Math.max(1, root.desktopCount))
                readonly property int desktopIndex: index % Math.max(1, root.desktopCount)
                readonly property var activity: root.backend.activities[activityIndex] ?? {id: "", name: ""}
                readonly property var desktop: root.backend.desktops[desktopIndex] ?? {id: "", name: ""}
                readonly property bool current: root.backend.currentActivity === activity.id
                    && root.backend.currentDesktop === desktop.id
                readonly property bool pending: root.backend.pending
                    && root.backend.pendingActivity === activity.id
                    && root.backend.pendingDesktop === desktop.id

                objectName: "cell-" + activityIndex + "-" + desktopIndex
                Layout.row: activityIndex + 1
                Layout.column: desktopIndex + 1
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
                    if (event.key === Qt.Key_Left && desktopIndex > 0) next--
                    else if (event.key === Qt.Key_Right && desktopIndex + 1 < root.desktopCount) next++
                    else if (event.key === Qt.Key_Up && activityIndex > 0) next -= root.desktopCount
                    else if (event.key === Qt.Key_Down && activityIndex + 1 < root.activityCount) next += root.desktopCount
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
        anchors.margins: root.framePadding
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
