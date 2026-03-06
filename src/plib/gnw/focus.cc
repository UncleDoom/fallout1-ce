#include "plib/gnw/focus.h"

#include <climits>
#include <cmath>
#include <cstring>

#include "plib/gnw/input.h"
#include "plib/gnw/mouse.h"

namespace fallout {

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------

static FocusElement g_elements[FOCUS_MAX_ELEMENTS];
static int g_element_count = 0;
static int g_current_focus = -1;

// ---------------------------------------------------------------------------
// Implementation
// ---------------------------------------------------------------------------

void focus_init()
{
    g_element_count = 0;
    g_current_focus = -1;
}

int focus_register(int id, int x, int y, int w, int h, int eventCode)
{
    if (g_element_count >= FOCUS_MAX_ELEMENTS) {
        return -1;
    }

    int idx = g_element_count;
    g_elements[idx].id = id;
    g_elements[idx].x = x;
    g_elements[idx].y = y;
    g_elements[idx].w = w;
    g_elements[idx].h = h;
    g_elements[idx].eventCode = eventCode;
    g_element_count++;

    // Auto-focus on the first element registered
    if (g_current_focus == -1) {
        g_current_focus = 0;
    }

    return idx;
}

void focus_clear()
{
    g_element_count = 0;
    g_current_focus = -1;
}

bool focus_move(FocusDirection dir)
{
    if (g_element_count == 0 || g_current_focus < 0) {
        return false;
    }

    const FocusElement& cur = g_elements[g_current_focus];
    int best_index = -1;
    int best_score = INT_MAX;

    for (int i = 0; i < g_element_count; i++) {
        if (i == g_current_focus) continue;

        const FocusElement& candidate = g_elements[i];
        int dx = candidate.x - cur.x;
        int dy = candidate.y - cur.y;

        // Check if candidate is in the correct direction
        bool valid = false;
        int primary_dist = 0;
        int secondary_dist = 0;

        switch (dir) {
        case FOCUS_DIR_UP:
            if (dy < 0) {
                valid = true;
                primary_dist = -dy;
                secondary_dist = abs(dx);
            }
            break;
        case FOCUS_DIR_DOWN:
            if (dy > 0) {
                valid = true;
                primary_dist = dy;
                secondary_dist = abs(dx);
            }
            break;
        case FOCUS_DIR_LEFT:
            if (dx < 0) {
                valid = true;
                primary_dist = -dx;
                secondary_dist = abs(dy);
            }
            break;
        case FOCUS_DIR_RIGHT:
            if (dx > 0) {
                valid = true;
                primary_dist = dx;
                secondary_dist = abs(dy);
            }
            break;
        }

        if (!valid) continue;

        // Score: prefer closer in primary direction, penalize off-axis
        int score = primary_dist + secondary_dist * 3;
        if (score < best_score) {
            best_score = score;
            best_index = i;
        }
    }

    if (best_index >= 0) {
        g_current_focus = best_index;
        return true;
    }

    return false;
}

int focus_get_current_event_code()
{
    if (g_current_focus < 0 || g_current_focus >= g_element_count) {
        return -1;
    }
    return g_elements[g_current_focus].eventCode;
}

int focus_get_current_index()
{
    return g_current_focus;
}

void focus_set_current(int index)
{
    if (index >= 0 && index < g_element_count) {
        g_current_focus = index;
    }
}

void focus_activate()
{
    int code = focus_get_current_event_code();
    if (code != -1) {
        GNW_add_input_buffer(code);
    }
}

bool focus_get_rect(int* out_x, int* out_y, int* out_w, int* out_h)
{
    if (g_current_focus < 0 || g_current_focus >= g_element_count) {
        return false;
    }

    const FocusElement& el = g_elements[g_current_focus];
    *out_x = el.x - el.w / 2;
    *out_y = el.y - el.h / 2;
    *out_w = el.w;
    *out_h = el.h;
    return true;
}

int focus_get_count()
{
    return g_element_count;
}

void focus_warp_mouse_to_current()
{
    if (g_current_focus < 0 || g_current_focus >= g_element_count) {
        return;
    }
    mouse_set_position(g_elements[g_current_focus].x, g_elements[g_current_focus].y);
    // Ensure the cursor is visible so the user can see where focus is.
    if (mouse_hidden()) {
        mouse_show();
    }
}

void focus_draw_highlight(unsigned char* buffer, int bufferWidth, unsigned char color)
{
    int rx, ry, rw, rh;
    if (!focus_get_rect(&rx, &ry, &rw, &rh)) {
        return;
    }

    // Draw a 1-pixel rectangle outline
    // Top edge
    for (int x = rx; x < rx + rw; x++) {
        buffer[ry * bufferWidth + x] = color;
    }
    // Bottom edge
    for (int x = rx; x < rx + rw; x++) {
        buffer[(ry + rh - 1) * bufferWidth + x] = color;
    }
    // Left edge
    for (int y = ry; y < ry + rh; y++) {
        buffer[y * bufferWidth + rx] = color;
    }
    // Right edge
    for (int y = ry; y < ry + rh; y++) {
        buffer[y * bufferWidth + (rx + rw - 1)] = color;
    }
}

} // namespace fallout
