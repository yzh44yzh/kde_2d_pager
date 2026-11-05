---
status: accepted
---

# Activity rows, desktop columns, and an opaque frame

Transpose the grid from [ADR 0001](0001-two-dimensional-activity-desktop-pager.md):
activities are rows with names on the left, and virtual desktops are columns with
numbers across the top. Arrow-key navigation follows these visible axes.

Give the entire widget an opaque KDE theme background, including its headers and
the gaps between cells. Add a thin theme-colored outer border with slightly rounded
corners and inner padding, keeping the individual cell borders. This gives the grid
a continuous surface and a clear boundary in the panel while following light and
dark themes.

Current-cell highlighting, the pending indicator, tooltips, and the switching
behavior from ADR 0001 remain in effect. Panel height now scales with the number of
activities; panel width scales with the number of desktops and the activity labels.
