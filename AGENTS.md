# Implementation stack

Use QML for the Plasma widget interface and a small C++ backend for KDE integration.

- QML owns the grid, labels, highlighting, tooltips, and keyboard interaction.
- C++ owns activity and virtual desktop discovery, change notifications, coordinated
  switching, and failure handling through native KDE APIs.
- Expose available activities and desktops, the current position, the pending
  destination, and a `switchTo(activityId, desktopId)` operation to QML.

This split keeps presentation in Plasma's native UI technology and switching logic
close to KDE's native APIs. The C++ plugin requires compilation and compatible KDE
development packages; account for these in local installation instructions.

Before implementing or changing widget behavior, read
[ADR 0001](docs/adr/0001-two-dimensional-activity-desktop-pager.md) for the agreed
interaction, scope, and target environment.
