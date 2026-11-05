---
status: accepted
---

# Two-dimensional activity and desktop pager

The grid orientation and framing below are superseded by
[ADR 0002](0002-activity-rows-and-opaque-frame.md). Switching behavior and scope
remain in effect. [ADR 0003](0003-panel-button-and-popup-grid.md) replaces the
always-visible grid with a panel button and popup.

Create a KDE Plasma panel widget with an always-visible grid of virtual desktop rows
and activity columns. Each cell selects an exact activity and desktop pair with one
click, providing direct access without opening a popup first.

## Grid and presentation

- Show all running activities; omit stopped activities without starting them.
- Follow KDE's activity and desktop additions, removals, renames, and ordering.
  Do not maintain separate names or custom ordering.
- Use activity names as column headers and desktop numbers as row labels.
  Show full desktop names in tooltips.
- Highlight the actual current activity and desktop cell, including changes made
  outside the widget.
- Allow enough panel height to keep every row readable and comfortable to click.
  The user accepts increasing the panel height for this layout.
- Support keyboard focus and activation as well as mouse clicks.

## Switching behavior

- A cell is a fixed destination. Selecting Work × Desktop 2 must end on that pair,
  regardless of the desktop last used in Work.
- Coordinate activity and desktop switching. KDE restores an activity's remembered
  desktop during activity activation, so independent requests can interfere with
  reaching the requested destination.
- Show the requested destination as pending until KDE reaches it. Keep the actual
  current cell highlighted while switching.
- If another cell is selected during a switch, the latest selection replaces the
  earlier destination.
- If switching fails, show a brief error and reflect the actual resulting position.
  Do not automatically switch back: the user may already have taken another action.

## Initial scope

Target the user's current Plasma 6.6.6 Wayland setup and provide local installation
instructions. The first version focuses on navigation; window previews, dragging
windows between cells, and managing activities or desktops are deferred. Broader
Plasma compatibility and KDE Store distribution can follow after the interaction
has been verified.

## Consequences

The full grid preserves one-click access at the cost of panel space. More desktops
require more panel height, and more activities require more width. Labels and a
current-position indicator keep the initial interface compact without window previews.

Activity and desktop changes are not atomic. A failed switch may leave the user
partway to the requested pair; the widget must represent the actual state accurately.
