import QtQuick
import org.kde.plasma.components as PlasmaComponents

PlasmaComponents.ToolButton {
    id: root
    required property bool popupOpen
    property bool expandedAtPress: false
    signal expansionRequested(bool open)

    text: "2D Activity Pager"
    icon.source: Qt.resolvedUrl("icons/local-twodpager.svg")
    display: PlasmaComponents.AbstractButton.IconOnly
    checked: popupOpen
    Accessible.name: text
    Accessible.description: popupOpen ? "Close activity and desktop switcher"
                                      : "Open activity and desktop switcher"
    onPressed: expandedAtPress = popupOpen
    onClicked: expansionRequested(!expandedAtPress)
    Keys.onReturnPressed: expansionRequested(!popupOpen)
    Keys.onEnterPressed: expansionRequested(!popupOpen)
}
