#pragma once

namespace fallout {

// ---------------------------------------------------------------------------
// Radial Menu — triggered by holding X (or dedicated button) on gamepad.
// Renders a circle divided into 8 sectors for the 8 Skilldex skills.
// Left stick angle selects a sector; releasing the trigger confirms selection.
// ---------------------------------------------------------------------------

// Skill labels matching Skilldex order:
// 0=Sneak, 1=Lockpick, 2=Steal, 3=Traps, 4=First Aid, 5=Doctor, 6=Science, 7=Repair

// Open the radial menu. Creates a modal overlay window.
// Returns the selected skill index (1-8, matching skilldex RC), or 0 if cancelled.
int radial_menu_open();

// Returns true if the radial menu is currently active.
bool radial_menu_is_active();

} // namespace fallout
