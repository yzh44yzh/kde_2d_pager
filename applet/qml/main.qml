pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import plasma.applet.local.twodpager

PlasmoidItem {
    id: root
    preferredRepresentation: compactRepresentation
    hideOnWindowDeactivate: true
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    Layout.minimumWidth: Kirigami.Units.iconSizes.small
    Layout.minimumHeight: Kirigami.Units.iconSizes.small
    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
    Layout.preferredHeight: Kirigami.Units.iconSizes.medium

    toolTipMainText: "2D Activity Pager"
    toolTipTextFormat: Text.PlainText
    toolTipSubText: kdeBackend.error || kdeBackend.status || currentDestination

    readonly property string currentDestination:
        nameFor(kdeBackend.activities, kdeBackend.currentActivity) + " · "
        + nameFor(kdeBackend.desktops, kdeBackend.currentDesktop)

    function nameFor(entries, id) {
        for (let i = 0; i < entries.length; ++i) {
            if (entries[i].id === id)
                return entries[i].name
        }
        return "Unknown"
    }

    PagerBackend { id: kdeBackend }

    compactRepresentation: PanelButton {
        popupOpen: root.expanded
        onExpansionRequested: open => root.expanded = open
    }

    fullRepresentation: PagerPopup {
        backend: kdeBackend
        popupOpen: root.expanded
        onCloseRequested: root.expanded = false
    }
}
