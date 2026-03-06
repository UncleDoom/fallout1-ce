#include "game/radial_menu.h"

#include <cmath>
#include <cstring>

#include "fps_limiter.h"
#include "plib/color/color.h"
#include "plib/gnw/gamepad.h"
#include "plib/gnw/gnw.h"
#include "plib/gnw/grbuf.h"
#include "plib/gnw/input.h"
#include "plib/gnw/svga.h"
#include "plib/gnw/text.h"

namespace fallout {

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

static constexpr int RADIAL_NUM_SECTORS = 8;
static constexpr int RADIAL_RADIUS = 100;
static constexpr int RADIAL_INNER_RADIUS = 30;
static constexpr int RADIAL_WIN_SIZE = (RADIAL_RADIUS + 20) * 2;

static const char* g_skill_labels[RADIAL_NUM_SECTORS] = {
    "Sneak",
    "Lockpick",
    "Steal",
    "Traps",
    "First Aid",
    "Doctor",
    "Science",
    "Repair",
};

static bool g_radial_active = false;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static void draw_circle_outline(unsigned char* buf, int pitch, int cx, int cy,
    int radius, unsigned char color)
{
    // Simple midpoint circle algorithm
    int x = 0;
    int y = radius;
    int d = 1 - radius;

    auto plot = [&](int px, int py) {
        if (px >= 0 && px < RADIAL_WIN_SIZE && py >= 0 && py < RADIAL_WIN_SIZE) {
            buf[py * pitch + px] = color;
        }
    };

    while (x <= y) {
        plot(cx + x, cy + y);
        plot(cx - x, cy + y);
        plot(cx + x, cy - y);
        plot(cx - x, cy - y);
        plot(cx + y, cy + x);
        plot(cx - y, cy + x);
        plot(cx + y, cy - x);
        plot(cx - y, cy - x);

        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

static void draw_line(unsigned char* buf, int pitch, int x0, int y0,
    int x1, int y1, unsigned char color)
{
    // Bresenham's line
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x0 >= 0 && x0 < RADIAL_WIN_SIZE && y0 >= 0 && y0 < RADIAL_WIN_SIZE) {
            buf[y0 * pitch + x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

static int get_sector_from_stick(float sx, float sy)
{
    if (sqrtf(sx * sx + sy * sy) < 0.3f) return -1; // dead zone

    float angle = atan2f(sy, sx); // -π to π
    float degrees = angle * (180.0f / 3.14159265f);

    // Normalize to 0-360
    if (degrees < 0) degrees += 360.0f;

    // Each sector = 45 degrees, offset by half-sector so 0° is centered on sector 0 (East=right)
    // Sector 0 starts at -22.5° (337.5°)
    int sector = (int)((degrees + 22.5f) / 45.0f) % RADIAL_NUM_SECTORS;
    return sector;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

int radial_menu_open()
{
    g_radial_active = true;

    gamepad_push_context(GAMEPAD_CTX_RADIAL);

    int screen_w = screenGetWidth();
    int screen_h = screenGetHeight();
    int win_x = (screen_w - RADIAL_WIN_SIZE) / 2;
    int win_y = (screen_h - RADIAL_WIN_SIZE) / 2;

    int win = win_add(win_x, win_y, RADIAL_WIN_SIZE, RADIAL_WIN_SIZE, 0, 0x10 | 0x04);
    if (win == -1) {
        gamepad_pop_context();
        g_radial_active = false;
        return 0;
    }

    unsigned char* buf = win_get_buf(win);
    int pitch = RADIAL_WIN_SIZE;
    int cx = RADIAL_WIN_SIZE / 2;
    int cy = RADIAL_WIN_SIZE / 2;

    int selected_sector = -1;
    int result = 0;
    bool done = false;
    bool x_was_confirmed_held = false;

    while (!done) {
        sharedFpsLimiter.mark();

        int input = get_input();

        // B or Escape → cancel
        if (input == KEY_ESCAPE) {
            result = 0;
            done = true;
            break;
        }

        // Check stick for sector selection
        float lx, ly;
        gamepad_get_left_stick(&lx, &ly);
        selected_sector = get_sector_from_stick(lx, ly);

        // A button → confirm
        if (input == KEY_RETURN && selected_sector >= 0) {
            result = selected_sector + 1; // 1-8
            done = true;
            break;
        }

        // Track X being held so we only trigger release after it was
        // actually detected as held at least once (prevents instant close
        // on the first frame if X was already released before the loop).
        if (gamepad_is_button_held(SDL_CONTROLLER_BUTTON_X)) {
            x_was_confirmed_held = true;
        }

        // Confirm if X is released while a sector is selected
        if (x_was_confirmed_held && !gamepad_is_button_held(SDL_CONTROLLER_BUTTON_X) && selected_sector >= 0) {
            result = selected_sector + 1;
            done = true;
            break;
        }

        // --- Render the radial menu ---
        // Clear buffer to transparent (color 0)
        memset(buf, 0, RADIAL_WIN_SIZE * RADIAL_WIN_SIZE);

        // Draw outer and inner circles
        unsigned char circle_color = colorTable[32767]; // bright white-ish
        draw_circle_outline(buf, pitch, cx, cy, RADIAL_RADIUS, circle_color);
        draw_circle_outline(buf, pitch, cx, cy, RADIAL_INNER_RADIUS, circle_color);

        // Draw sector divider lines
        for (int i = 0; i < RADIAL_NUM_SECTORS; i++) {
            float angle = (i * 45.0f - 22.5f) * (3.14159265f / 180.0f);
            int x0 = cx + (int)(RADIAL_INNER_RADIUS * cosf(angle));
            int y0 = cy + (int)(RADIAL_INNER_RADIUS * sinf(angle));
            int x1 = cx + (int)(RADIAL_RADIUS * cosf(angle));
            int y1 = cy + (int)(RADIAL_RADIUS * sinf(angle));
            draw_line(buf, pitch, x0, y0, x1, y1, circle_color);
        }

        // Draw skill labels
        for (int i = 0; i < RADIAL_NUM_SECTORS; i++) {
            float angle = (i * 45.0f) * (3.14159265f / 180.0f);
            int label_r = (RADIAL_INNER_RADIUS + RADIAL_RADIUS) / 2;
            int lx_pos = cx + (int)(label_r * cosf(angle));
            int ly_pos = cy + (int)(label_r * sinf(angle));

            const char* label = g_skill_labels[i];
            int tw = text_width(label);
            int th = text_height();
            int tx = lx_pos - tw / 2;
            int ty = ly_pos - th / 2;

            if (tx < 0) tx = 0;
            if (ty < 0) ty = 0;
            if (tx + tw >= RADIAL_WIN_SIZE) tx = RADIAL_WIN_SIZE - tw - 1;
            if (ty + th >= RADIAL_WIN_SIZE) ty = RADIAL_WIN_SIZE - th - 1;

            unsigned char text_color_val = (i == selected_sector)
                ? colorTable[32767]  // highlighted
                : colorTable[10570]; // dimmer
            text_to_buf(buf + ty * pitch + tx, label, tw + 10, pitch, text_color_val | 0x2000000);
        }

        // Highlight selected sector with a filled wedge approximation
        if (selected_sector >= 0) {
            // Draw a thicker arc for the selected sector by drawing a second
            // circle at radius-2
            draw_circle_outline(buf, pitch, cx, cy, RADIAL_RADIUS - 2, colorTable[32328]);
        }

        win_draw(win);

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    win_delete(win);
    gamepad_pop_context();
    g_radial_active = false;

    return result;
}

bool radial_menu_is_active()
{
    return g_radial_active;
}

} // namespace fallout
