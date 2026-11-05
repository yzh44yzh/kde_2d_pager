import QtQuick
import QtTest
import "../../applet/qml" as Applet

TestCase {
    id: testCase
    name: "PanelPopup"
    width: 480
    height: 260
    visible: true
    when: windowShown
    property bool expanded: false

    QtObject {
        id: backend
        property var activities: [{id: "work", name: "Work"}, {id: "music", name: "Music"}]
        property var desktops: [{id: "one", name: "Writing"}, {id: "two", name: "Tools"}]
        property string currentActivity: "music"
        property string currentDesktop: "two"
        property bool available: true
        property bool pending: false
        property string pendingActivity: ""
        property string pendingDesktop: ""
        property string error: ""
        property string status: ""
        signal switchSucceeded()
        function switchTo(activity, desktop) {
            pendingActivity = activity
            pendingDesktop = desktop
            error = ""
            pending = true
        }
        function succeed() {
            currentActivity = pendingActivity
            currentDesktop = pendingDesktop
            pending = false
            switchSucceeded()
        }
    }

    Applet.PanelButton {
        id: button
        width: 40
        height: 40
        popupOpen: testCase.expanded
        onExpansionRequested: open => testCase.expanded = open
    }

    Applet.PagerPopup {
        id: popup
        x: 50
        y: 40
        width: 360
        height: 180
        backend: backend
        popupOpen: testCase.expanded
        visible: popupOpen
        onCloseRequested: testCase.expanded = false
    }

    function cell(row, column) {
        return findChild(popup, "cell-" + row + "-" + column)
    }

    function init() {
        failOnWarning(/.*/)
        testCase.expanded = false
        backend.currentActivity = "music"
        backend.currentDesktop = "two"
        backend.pending = false
        backend.error = ""
        wait(0)
    }

    function test_buttonTogglesAndOpeningFocusesCurrentCell() {
        mouseClick(button)
        compare(testCase.expanded, true)
        tryCompare(cell(1, 1), "activeFocus", true)
        mouseClick(button)
        compare(testCase.expanded, false)
    }

    function test_waitsForSuccessThenCloses() {
        mouseClick(button)
        mouseClick(cell(0, 1))
        verify(backend.pending)
        verify(testCase.expanded)
        backend.succeed()
        compare(testCase.expanded, false)
        mouseClick(button)
        tryCompare(cell(0, 1), "activeFocus", true)
    }

    function test_failureStaysOpenAndAllowsRetry() {
        mouseClick(button)
        mouseClick(cell(0, 0))
        backend.pending = false
        backend.error = "KDE rejected the switch."
        verify(testCase.expanded)
        mouseClick(cell(0, 1))
        verify(backend.pending)
        verify(testCase.expanded)
        backend.succeed()
        compare(testCase.expanded, false)
    }

    function test_escapeClosesFromFocusedCell() {
        mouseClick(button)
        tryCompare(cell(1, 1), "activeFocus", true)
        keyClick(Qt.Key_Escape)
        compare(testCase.expanded, false)
    }

    function test_keyboardOpensAndClosesButton() {
        button.forceActiveFocus()
        keyClick(Qt.Key_Space)
        compare(testCase.expanded, true)
        button.forceActiveFocus()
        keyClick(Qt.Key_Return)
        compare(testCase.expanded, false)
    }
}
