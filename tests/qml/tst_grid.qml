import QtQuick
import QtTest
import "../../applet/qml" as Applet

TestCase {
    id: testCase
    name: "PagerGrid"
    width: 360
    height: 180
    visible: true
    when: windowShown

    QtObject {
        id: backend
        property var activities: [{id: "work", name: "Work"}, {id: "music", name: "Music"}]
        property var desktops: [{id: "one", name: "Writing"}, {id: "two", name: "Tools"}]
        property string currentActivity: "work"
        property string currentDesktop: "one"
        property bool available: true
        property bool pending: false
        property string pendingActivity: ""
        property string pendingDesktop: ""
        property string error: ""
        property string status: ""
        property var requested: []
        function switchTo(activity, desktop) { requested = [activity, desktop] }
    }

    Applet.PagerGrid {
        id: grid
        anchors.fill: parent
        backend: backend
    }

    function init() {
        failOnWarning(/.*/)
        backend.activities = [{id: "work", name: "Work"}, {id: "music", name: "Music"}]
        backend.desktops = [{id: "one", name: "Writing"}, {id: "two", name: "Tools"}]
        backend.currentActivity = "work"
        backend.currentDesktop = "one"
        backend.pending = false
        backend.available = true
        backend.error = ""
        backend.requested = []
        wait(0)
    }

    function cell(row, column) {
        return findChild(grid, "cell-" + row + "-" + column)
    }

    function test_oneClickSelectsExactPair() {
        mouseClick(cell(1, 1))
        compare(backend.requested, ["music", "two"])
    }

    function test_keyboardMovesAndActivates() {
        cell(0, 0).forceActiveFocus()
        keyClick(Qt.Key_Right)
        keyClick(Qt.Key_Down)
        verify(cell(1, 1).activeFocus)
        keyClick(Qt.Key_Return)
        compare(backend.requested, ["music", "two"])
        keyClick(Qt.Key_Left)
        keyClick(Qt.Key_Space)
        compare(backend.requested, ["work", "two"])
    }

    function test_pendingDoesNotMoveCurrentHighlight() {
        backend.pendingActivity = "music"
        backend.pendingDesktop = "two"
        backend.pending = true
        verify(cell(0, 0).current)
        verify(!cell(1, 1).current)
        verify(cell(1, 1).pending)
        backend.currentActivity = "music"
        backend.currentDesktop = "two"
        verify(!cell(0, 0).current)
        verify(cell(1, 1).current)
    }

    function test_tracksRenamesAndRemovals() {
        backend.activities = [{id: "music", name: "Studio"}]
        backend.desktops = [{id: "two", name: "Mixer"}]
        wait(0)
        compare(grid.activityCount, 1)
        compare(grid.desktopCount, 1)
        compare(cell(0, 0).Accessible.name, "Studio, Mixer")
        mouseClick(cell(0, 0))
        compare(backend.requested, ["music", "two"])
    }

    function test_unavailableDisablesSwitching() {
        backend.available = false
        mouseClick(cell(1, 1))
        compare(backend.requested, [])
    }

    function test_errorLeavesPositionVisibleAndAllowsAnotherClick() {
        backend.error = "KDE rejected the switch."
        wait(0)
        verify(cell(0, 0).current)
        mouseClick(cell(1, 1))
        compare(backend.requested, ["music", "two"])
    }

    function test_emptyGrid() {
        backend.activities = []
        backend.desktops = []
        wait(0)
        verify(grid.minimumWidth > 0)
        verify(grid.minimumHeight > 0)
        compare(cell(0, 0), null)
    }
}
