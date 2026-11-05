pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import plasma.applet.local.twodpager

PlasmoidItem {
    id: root
    readonly property PagerGrid pagerGrid: fullRepresentationItem as PagerGrid

    preferredRepresentation: fullRepresentation
    switchWidth: 0
    switchHeight: 0
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    Layout.minimumWidth: pagerGrid ? pagerGrid.minimumWidth : 200
    Layout.minimumHeight: pagerGrid ? pagerGrid.minimumHeight : 80
    Layout.preferredWidth: Layout.minimumWidth
    Layout.preferredHeight: Layout.minimumHeight
    Layout.fillHeight: true

    toolTipMainText: "2D Activity Pager"
    toolTipSubText: kdeBackend.error || kdeBackend.status || "Choose an activity column and desktop row."

    PagerBackend { id: kdeBackend }

    fullRepresentation: PagerGrid {
        backend: kdeBackend
    }
}
