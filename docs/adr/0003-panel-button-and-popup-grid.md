---
status: accepted
---

# Panel button and popup grid

Replace the always-visible panel grid from [ADR 0001](0001-two-dimensional-activity-desktop-pager.md)
with a compact grid-icon button that opens a native Plasma popup. This reduces the
panel footprint at the cost of two clicks to select a destination: one to open the
popup and one to choose a cell.

- The button tooltip shows the current activity and desktop.
- Keep activity rows, desktop columns, and the opaque frame from
  [ADR 0002](0002-activity-rows-and-opaque-frame.md).
- Focus the current destination when the popup opens; retain arrow-key navigation
  and Space/Enter activation.
- Keep the popup open while switching. Close it on confirmed success; keep it open
  on failure so the user can see the error and retry.
- Escape, clicking outside, or clicking the panel button again dismisses the popup.
  Dismissing the popup does not cancel an in-progress switch.

Use Plasma's compact/full representations for popup placement and outside-click
dismissal. Popup size follows the grid contents; the panel no longer needs extra
height to fit activity rows. The backend reports success explicitly so the UI can
distinguish completion from failure without depending on notification order.
