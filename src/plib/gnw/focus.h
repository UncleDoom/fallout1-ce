#pragma once

namespace fallout {

// ---------------------------------------------------------------------------
// Focus Manager — lightweight system for D-Pad navigation of UI buttons.
// Each screen registers its focusable elements after creating buttons.
// The focus manager tracks which element is "selected" and provides
// directional navigation (nearest neighbor in requested direction).
// ---------------------------------------------------------------------------

static constexpr int FOCUS_MAX_ELEMENTS = 64;

struct FocusElement {
    int id;        // unique id within the window
    int x;         // center x
    int y;         // center y
    int w;         // width (for highlight rect)
    int h;         // height
    int eventCode; // the GNW event code to inject when activated
};

enum FocusDirection {
    FOCUS_DIR_UP = 0,
    FOCUS_DIR_DOWN,
    FOCUS_DIR_LEFT,
    FOCUS_DIR_RIGHT,
};

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------

// Initialize / reset the focus system for a new screen.
void focus_init();

// Register a focusable element. Returns the element index.
int focus_register(int id, int x, int y, int w, int h, int eventCode);

// Clear all registered elements.
void focus_clear();

// Move focus in a direction. Returns true if focus changed.
bool focus_move(FocusDirection dir);

// Get the currently focused element's event code. Returns -1 if none.
int focus_get_current_event_code();

// Get the currently focused element index. Returns -1 if none.
int focus_get_current_index();

// Set the focused element by index.
void focus_set_current(int index);

// Activate the currently focused element (inject its event code).
void focus_activate();

// Get the focused element's bounding rect for highlight drawing.
// Returns false if no element is focused.
bool focus_get_rect(int* out_x, int* out_y, int* out_w, int* out_h);

// Get the number of registered elements.
int focus_get_count();

// Warp the mouse cursor to the center of the currently focused element.
// This provides visual feedback by triggering the GNW button hover state.
void focus_warp_mouse_to_current();

// Draw highlight rectangle around the focused element into a pixel buffer.
// buffer: destination pixel buffer
// bufferWidth: pitch of the buffer
// color: palette index for the highlight
void focus_draw_highlight(unsigned char* buffer, int bufferWidth, unsigned char color);

} // namespace fallout
