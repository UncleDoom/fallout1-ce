#include "plib/gnw/button.h"

#include "plib/color/color.h"
#include "plib/gnw/gnw.h"
#include "plib/gnw/grbuf.h"
#include "plib/gnw/input.h"
#include "plib/gnw/memory.h"
#include "plib/gnw/mouse.h"
#include "plib/gnw/text.h"

namespace fallout {

// The maximum number of button groups.
static constexpr int BUTTON_GROUP_LIST_CAPACITY = 64;

// 0x53A258
static int last_button_winID = -1;

// 0x6AC2D0
static ButtonGroup btn_grp[BUTTON_GROUP_LIST_CAPACITY];

static Button* button_create(int win, int x, int y, int width, int height, int mouseEnterEventCode, int mouseExitEventCode, int mouseDownEventCode, int mouseUpEventCode, int flags, unsigned char* up, unsigned char* dn, unsigned char* hover);

// 0x4C4320
int win_register_button(int win, int x, int y, int width, int height, int mouseEnterEventCode, int mouseExitEventCode, int mouseDownEventCode, int mouseUpEventCode, unsigned char* up, unsigned char* dn, unsigned char* hover, int flags)
{
    Window* w = GNW_find(win);

    if (!GNW_win_init_flag) {
        return -1;
    }

    if (w == nullptr) {
        return -1;
    }

    if (up == nullptr && (dn != nullptr || hover != nullptr)) {
        return -1;
    }

    Button* button = button_create(win, x, y, width, height, mouseEnterEventCode, mouseExitEventCode, mouseDownEventCode, mouseUpEventCode, flags | BUTTON_FLAG_GRAPHIC, up, dn, hover);
    if (button == nullptr) {
        return -1;
    }

    button->draw(w, button->normalImage, false, nullptr, false);

    return button->id;
}

// 0x4C43C8
int win_register_text_button(int win, int x, int y, int mouseEnterEventCode, int mouseExitEventCode, int mouseDownEventCode, int mouseUpEventCode, const char* title, int flags)
{
    Window* w = GNW_find(win);

    if (!GNW_win_init_flag) {
        return -1;
    }

    if (w == nullptr) {
        return -1;
    }

    int buttonWidth = text_width(title) + 16;
    int buttonHeight = text_height() + 7;
    unsigned char* normal = static_cast<unsigned char*>(mem_malloc(buttonWidth * buttonHeight));
    if (normal == nullptr) {
        return -1;
    }

    unsigned char* pressed = static_cast<unsigned char*>(mem_malloc(buttonWidth * buttonHeight));
    if (pressed == nullptr) {
        mem_free(normal);
        return -1;
    }

    if (w->color == 256 && GNW_texture != nullptr) {
        // TODO: Incomplete.
    } else {
        buf_fill(normal, buttonWidth, buttonHeight, buttonWidth, w->color);
        buf_fill(pressed, buttonWidth, buttonHeight, buttonWidth, w->color);
    }

    lighten_buf(normal, buttonWidth, buttonHeight, buttonWidth);

    text_to_buf(normal + buttonWidth * 3 + 8, title, buttonWidth, buttonWidth, colorTable[GNW_wcolor[3]]);
    draw_shaded_box(normal,
        buttonWidth,
        2,
        2,
        buttonWidth - 3,
        buttonHeight - 3,
        colorTable[GNW_wcolor[1]],
        colorTable[GNW_wcolor[2]]);
    draw_shaded_box(normal,
        buttonWidth,
        1,
        1,
        buttonWidth - 2,
        buttonHeight - 2,
        colorTable[GNW_wcolor[1]],
        colorTable[GNW_wcolor[2]]);
    draw_box(normal, buttonWidth, 0, 0, buttonWidth - 1, buttonHeight - 1, colorTable[0]);

    text_to_buf(pressed + buttonWidth * 4 + 9, title, buttonWidth, buttonWidth, colorTable[GNW_wcolor[3]]);
    draw_shaded_box(pressed,
        buttonWidth,
        2,
        2,
        buttonWidth - 3,
        buttonHeight - 3,
        colorTable[GNW_wcolor[2]],
        colorTable[GNW_wcolor[1]]);
    draw_shaded_box(pressed,
        buttonWidth,
        1,
        1,
        buttonWidth - 2,
        buttonHeight - 2,
        colorTable[GNW_wcolor[2]],
        colorTable[GNW_wcolor[1]]);
    draw_box(pressed, buttonWidth, 0, 0, buttonWidth - 1, buttonHeight - 1, colorTable[0]);

    Button* button = button_create(win,
        x,
        y,
        buttonWidth,
        buttonHeight,
        mouseEnterEventCode,
        mouseExitEventCode,
        mouseDownEventCode,
        mouseUpEventCode,
        flags,
        normal,
        pressed,
        nullptr);
    if (button == nullptr) {
        mem_free(normal);
        mem_free(pressed);
        return -1;
    }

    button->draw(w, button->normalImage, false, nullptr, false);

    return button->id;
}

// 0x4C4734
int win_register_button_disable(int btn, unsigned char* up, unsigned char* down, unsigned char* hover)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    Button* button = GNW_find_button(btn, nullptr);
    if (button == nullptr) {
        return -1;
    }

    button->disabledNormalImage = up;
    button->disabledPressedImage = down;
    button->disabledHoverImage = hover;

    return 0;
}

// 0x4C4768
int win_register_button_image(int btn, unsigned char* up, unsigned char* down, unsigned char* hover, bool draw)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    if (up == nullptr && (down != nullptr || hover != nullptr)) {
        return -1;
    }

    Window* w;
    Button* button = GNW_find_button(btn, &w);
    if (button == nullptr) {
        return -1;
    }

    if ((button->flags & BUTTON_FLAG_GRAPHIC) == 0) {
        return -1;
    }

    unsigned char* data = button->currentImage;
    if (data == button->normalImage) {
        button->currentImage = up;
    } else if (data == button->pressedImage) {
        button->currentImage = down;
    } else if (data == button->hoverImage) {
        button->currentImage = hover;
    }

    button->normalImage = up;
    button->pressedImage = down;
    button->hoverImage = hover;

    button->draw(w, button->currentImage, draw, nullptr, false);

    return 0;
}

// 0x4C4810
int win_register_button_func(int btn, ButtonCallback* mouseEnterProc, ButtonCallback* mouseExitProc, ButtonCallback* mouseDownProc, ButtonCallback* mouseUpProc)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    Button* button = GNW_find_button(btn, nullptr);
    if (button == nullptr) {
        return -1;
    }

    button->mouseEnterProc = mouseEnterProc;
    button->mouseExitProc = mouseExitProc;
    button->leftMouseDownProc = mouseDownProc;
    button->leftMouseUpProc = mouseUpProc;

    return 0;
}

// 0x4C4850
int win_register_right_button(int btn, int rightMouseDownEventCode, int rightMouseUpEventCode, ButtonCallback* rightMouseDownProc, ButtonCallback* rightMouseUpProc)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    Button* button = GNW_find_button(btn, nullptr);
    if (button == nullptr) {
        return -1;
    }

    button->rightMouseDownEventCode = rightMouseDownEventCode;
    button->rightMouseUpEventCode = rightMouseUpEventCode;
    button->rightMouseDownProc = rightMouseDownProc;
    button->rightMouseUpProc = rightMouseUpProc;

    if (rightMouseDownEventCode != -1 || rightMouseUpEventCode != -1 || rightMouseDownProc != nullptr || rightMouseUpProc != nullptr) {
        button->flags |= BUTTON_FLAG_RIGHT_MOUSE_BUTTON_CONFIGURED;
    } else {
        button->flags &= ~BUTTON_FLAG_RIGHT_MOUSE_BUTTON_CONFIGURED;
    }

    return 0;
}

// 0x4C48B0
int win_register_button_sound_func(int btn, ButtonCallback* pressSoundFunc, ButtonCallback* releaseSoundFunc)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    Button* button = GNW_find_button(btn, nullptr);
    if (button == nullptr) {
        return -1;
    }

    button->pressSoundFunc = pressSoundFunc;
    button->releaseSoundFunc = releaseSoundFunc;

    return 0;
}

// 0x4C48E0
int win_register_button_mask(int btn, unsigned char* mask)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    Button* button = GNW_find_button(btn, nullptr);
    if (button == nullptr) {
        return -1;
    }

    button->mask = mask;

    return 0;
}

// 0x4C490C
static Button* button_create(int win, int x, int y, int width, int height, int mouseEnterEventCode, int mouseExitEventCode, int mouseDownEventCode, int mouseUpEventCode, int flags, unsigned char* up, unsigned char* dn, unsigned char* hover)
{
    Window* w = GNW_find(win);
    if (w == nullptr) {
        return nullptr;
    }

    Button* button = static_cast<Button*>(mem_malloc(sizeof(*button)));
    if (button == nullptr) {
        return nullptr;
    }

    if ((flags & BUTTON_FLAG_0x01) == 0) {
        if ((flags & BUTTON_FLAG_0x02) != 0) {
            flags &= ~BUTTON_FLAG_0x02;
        }

        if ((flags & BUTTON_FLAG_0x04) != 0) {
            flags &= ~BUTTON_FLAG_0x04;
        }
    }

    // NOTE: Uninline.
    int buttonId = button_new_id();

    button->id = buttonId;
    button->flags = flags;
    button->rect.ulx = x;
    button->rect.uly = y;
    button->rect.lrx = x + width - 1;
    button->rect.lry = y + height - 1;
    button->mouseEnterEventCode = mouseEnterEventCode;
    button->mouseExitEventCode = mouseExitEventCode;
    button->lefMouseDownEventCode = mouseDownEventCode;
    button->leftMouseUpEventCode = mouseUpEventCode;
    button->rightMouseDownEventCode = -1;
    button->rightMouseUpEventCode = -1;
    button->normalImage = up;
    button->pressedImage = dn;
    button->hoverImage = hover;
    button->disabledNormalImage = nullptr;
    button->disabledPressedImage = nullptr;
    button->disabledHoverImage = nullptr;
    button->currentImage = nullptr;
    button->mask = nullptr;
    button->mouseEnterProc = nullptr;
    button->mouseExitProc = nullptr;
    button->leftMouseDownProc = nullptr;
    button->leftMouseUpProc = nullptr;
    button->rightMouseDownProc = nullptr;
    button->rightMouseUpProc = nullptr;
    button->pressSoundFunc = nullptr;
    button->releaseSoundFunc = nullptr;
    button->buttonGroup = nullptr;
    button->prev = nullptr;

    button->next = w->buttonListHead;
    if (button->next != nullptr) {
        button->next->prev = button;
    }
    w->buttonListHead = button;

    return button;
}

// 0x4C4A9C
bool win_button_down(int btn)
{
    if (!GNW_win_init_flag) {
        return false;
    }

    Button* button = GNW_find_button(btn, nullptr);
    if (button == nullptr) {
        return false;
    }

    if ((button->flags & BUTTON_FLAG_0x01) != 0 && (button->flags & BUTTON_FLAG_CHECKED) != 0) {
        return true;
    }

    return false;
}

// 0x4C4AC8
int Window::checkButtons(int* keyCodePtr)
{
    Rect v58;
    Button* hoveredButton;
    Button* clickedButton;
    Button* button;

    if ((flags & WINDOW_HIDDEN) != 0) {
        return -1;
    }

    button = buttonListHead;
    hoveredButton = this->hoveredButton;
    clickedButton = this->clickedButton;

    if (hoveredButton != nullptr) {
        v58 = hoveredButton->rect;
        v58.offset(rect.ulx, rect.uly);
    } else if (clickedButton != nullptr) {
        v58 = clickedButton->rect;
        v58.offset(rect.ulx, rect.uly);
    }

    *keyCodePtr = -1;

    if (mouse_click_in(rect.ulx, rect.uly, rect.lrx, rect.lry)) {
        int mouseEvent = mouse_get_buttons();
        if ((flags & WINDOW_FLAG_0x40) || (mouseEvent & MOUSE_EVENT_LEFT_BUTTON_DOWN) == 0) {
            if (mouseEvent == 0) {
                this->clickedButton = nullptr;
            }
        } else {
            win_show(id);
        }

        if (hoveredButton != nullptr) {
            if (!hoveredButton->underMouse(&v58)) {
                if (!(hoveredButton->flags & BUTTON_FLAG_DISABLED)) {
                    *keyCodePtr = hoveredButton->mouseExitEventCode;
                }

                if ((hoveredButton->flags & BUTTON_FLAG_0x01) && (hoveredButton->flags & BUTTON_FLAG_CHECKED)) {
                    hoveredButton->draw(this, hoveredButton->pressedImage, true, nullptr, true);
                } else {
                    hoveredButton->draw(this, hoveredButton->normalImage, true, nullptr, true);
                }

                this->hoveredButton = nullptr;

                last_button_winID = id;

                if (!(hoveredButton->flags & BUTTON_FLAG_DISABLED)) {
                    if (hoveredButton->mouseExitProc != nullptr) {
                        hoveredButton->mouseExitProc(hoveredButton->id, *keyCodePtr);
                        if (!(hoveredButton->flags & BUTTON_FLAG_0x40)) {
                            *keyCodePtr = -1;
                        }
                    }
                }
                return 0;
            }
            button = hoveredButton;
        } else if (clickedButton != nullptr) {
            if (clickedButton->underMouse(&v58)) {
                if (!(clickedButton->flags & BUTTON_FLAG_DISABLED)) {
                    *keyCodePtr = clickedButton->mouseEnterEventCode;
                }

                if ((clickedButton->flags & BUTTON_FLAG_0x01) && (clickedButton->flags & BUTTON_FLAG_CHECKED)) {
                    clickedButton->draw(this, clickedButton->pressedImage, true, nullptr, true);
                } else {
                    clickedButton->draw(this, clickedButton->normalImage, true, nullptr, true);
                }

                this->hoveredButton = clickedButton;

                last_button_winID = id;

                if (!(clickedButton->flags & BUTTON_FLAG_DISABLED)) {
                    if (clickedButton->mouseEnterProc != nullptr) {
                        clickedButton->mouseEnterProc(clickedButton->id, *keyCodePtr);
                        if (!(clickedButton->flags & BUTTON_FLAG_0x40)) {
                            *keyCodePtr = -1;
                        }
                    }
                }
                return 0;
            }
        }

        int v25 = last_button_winID;
        if (last_button_winID != -1 && last_button_winID != id) {
            Window* v26 = GNW_find(last_button_winID);
            if (v26 != nullptr) {
                last_button_winID = -1;

                Button* v28 = v26->hoveredButton;
                if (v28 != nullptr) {
                    if (!(v28->flags & BUTTON_FLAG_DISABLED)) {
                        *keyCodePtr = v28->mouseExitEventCode;
                    }

                    if ((v28->flags & BUTTON_FLAG_0x01) && (v28->flags & BUTTON_FLAG_CHECKED)) {
                        v28->draw(v26, v28->pressedImage, true, nullptr, true);
                    } else {
                        v28->draw(v26, v28->normalImage, true, nullptr, true);
                    }

                    v26->clickedButton = nullptr;
                    v26->hoveredButton = nullptr;

                    if (!(v28->flags & BUTTON_FLAG_DISABLED)) {
                        if (v28->mouseExitProc != nullptr) {
                            v28->mouseExitProc(v28->id, *keyCodePtr);
                            if (!(v28->flags & BUTTON_FLAG_0x40)) {
                                *keyCodePtr = -1;
                            }
                        }
                    }
                    return 0;
                }
            }
        }

        ButtonCallback* cb = nullptr;

        while (button != nullptr) {
            if (!(button->flags & BUTTON_FLAG_DISABLED)) {
                v58 = button->rect;
                v58.offset(rect.ulx, rect.uly);
                if (button->underMouse(&v58)) {
                    if (!(button->flags & BUTTON_FLAG_DISABLED)) {
                        if ((mouseEvent & MOUSE_EVENT_ANY_BUTTON_DOWN) != 0) {
                            if ((mouseEvent & MOUSE_EVENT_RIGHT_BUTTON_DOWN) != 0 && (button->flags & BUTTON_FLAG_RIGHT_MOUSE_BUTTON_CONFIGURED) == 0) {
                                button = nullptr;
                                break;
                            }

                            if (button != this->hoveredButton && button != this->clickedButton) {
                                break;
                            }

                            this->clickedButton = button;
                            this->hoveredButton = button;

                            if ((button->flags & BUTTON_FLAG_0x01) != 0) {
                                if ((button->flags & BUTTON_FLAG_0x02) != 0) {
                                    if ((button->flags & BUTTON_FLAG_CHECKED) != 0) {
                                        if (!(button->flags & BUTTON_FLAG_0x04)) {
                                            if (button->buttonGroup != nullptr) {
                                                button->buttonGroup->currChecked--;
                                            }

                                            if ((mouseEvent & MOUSE_EVENT_LEFT_BUTTON_DOWN) != 0) {
                                                *keyCodePtr = button->leftMouseUpEventCode;
                                                cb = button->leftMouseUpProc;
                                            } else {
                                                *keyCodePtr = button->rightMouseUpEventCode;
                                                cb = button->rightMouseUpProc;
                                            }

                                            button->flags &= ~BUTTON_FLAG_CHECKED;
                                        }
                                    } else {
                                        if (button->checkGroup() == -1) {
                                            button = nullptr;
                                            break;
                                        }

                                        if ((mouseEvent & MOUSE_EVENT_LEFT_BUTTON_DOWN) != 0) {
                                            *keyCodePtr = button->lefMouseDownEventCode;
                                            cb = button->leftMouseDownProc;
                                        } else {
                                            *keyCodePtr = button->rightMouseDownEventCode;
                                            cb = button->rightMouseDownProc;
                                        }

                                        button->flags |= BUTTON_FLAG_CHECKED;
                                    }
                                }
                            } else {
                                if (button->checkGroup() == -1) {
                                    button = nullptr;
                                    break;
                                }

                                if ((mouseEvent & MOUSE_EVENT_LEFT_BUTTON_DOWN) != 0) {
                                    *keyCodePtr = button->lefMouseDownEventCode;
                                    cb = button->leftMouseDownProc;
                                } else {
                                    *keyCodePtr = button->rightMouseDownEventCode;
                                    cb = button->rightMouseDownProc;
                                }
                            }

                            button->draw(this, button->pressedImage, true, nullptr, true);
                            break;
                        }

                        Button* v49 = this->clickedButton;
                        if (button == v49 && (mouseEvent & MOUSE_EVENT_ANY_BUTTON_UP) != 0) {
                            this->clickedButton = nullptr;
                            this->hoveredButton = v49;

                            if (v49->flags & BUTTON_FLAG_0x01) {
                                if (!(v49->flags & BUTTON_FLAG_0x02)) {
                                    if (v49->flags & BUTTON_FLAG_CHECKED) {
                                        if (!(v49->flags & BUTTON_FLAG_0x04)) {
                                            if (v49->buttonGroup != nullptr) {
                                                v49->buttonGroup->currChecked--;
                                            }

                                            if ((mouseEvent & MOUSE_EVENT_LEFT_BUTTON_UP) != 0) {
                                                *keyCodePtr = button->leftMouseUpEventCode;
                                                cb = button->leftMouseUpProc;
                                            } else {
                                                *keyCodePtr = button->rightMouseUpEventCode;
                                                cb = button->rightMouseUpProc;
                                            }

                                            button->flags &= ~BUTTON_FLAG_CHECKED;
                                        }
                                    } else {
                                        if (v49->checkGroup() == -1) {
                                            button = nullptr;
                                            v49->draw(this, v49->normalImage, true, nullptr, true);
                                            break;
                                        }

                                        if ((mouseEvent & MOUSE_EVENT_LEFT_BUTTON_UP) != 0) {
                                            *keyCodePtr = v49->lefMouseDownEventCode;
                                            cb = v49->leftMouseDownProc;
                                        } else {
                                            *keyCodePtr = v49->rightMouseDownEventCode;
                                            cb = v49->rightMouseDownProc;
                                        }

                                        v49->flags |= BUTTON_FLAG_CHECKED;
                                    }
                                }
                            } else {
                                if (v49->flags & BUTTON_FLAG_CHECKED) {
                                    if (v49->buttonGroup != nullptr) {
                                        v49->buttonGroup->currChecked--;
                                    }
                                }

                                if ((mouseEvent & MOUSE_EVENT_LEFT_BUTTON_UP) != 0) {
                                    *keyCodePtr = v49->leftMouseUpEventCode;
                                    cb = v49->leftMouseUpProc;
                                } else {
                                    *keyCodePtr = v49->rightMouseUpEventCode;
                                    cb = v49->rightMouseUpProc;
                                }
                            }

                            if (button->hoverImage != nullptr) {
                                button->draw(this, button->hoverImage, true, nullptr, true);
                            } else {
                                button->draw(this, button->normalImage, true, nullptr, true);
                            }
                            break;
                        }
                    }

                    if (this->hoveredButton == nullptr && mouseEvent == 0) {
                        this->hoveredButton = button;
                        if (!(button->flags & BUTTON_FLAG_DISABLED)) {
                            *keyCodePtr = button->mouseEnterEventCode;
                            cb = button->mouseEnterProc;
                        }

                        button->draw(this, button->hoverImage, true, nullptr, true);
                    }
                    break;
                }
            }
            button = button->next;
        }

        if (button != nullptr) {
            if ((button->flags & BUTTON_FLAG_0x10) != 0
                && (mouseEvent & MOUSE_EVENT_ANY_BUTTON_DOWN) != 0
                && (mouseEvent & MOUSE_EVENT_ANY_BUTTON_REPEAT) == 0) {
                win_drag(id);
                button->draw(this, button->normalImage, true, nullptr, true);
            }
        } else if ((flags & WINDOW_FLAG_0x80) != 0) {
            v25 |= mouseEvent << 8;
            if ((mouseEvent & MOUSE_EVENT_ANY_BUTTON_DOWN) != 0
                && (mouseEvent & MOUSE_EVENT_ANY_BUTTON_REPEAT) == 0) {
                win_drag(id);
            }
        }

        last_button_winID = id;

        if (button != nullptr) {
            if (cb != nullptr) {
                cb(button->id, *keyCodePtr);
                if (!(button->flags & BUTTON_FLAG_0x40)) {
                    *keyCodePtr = -1;
                }
            }
        }

        return 0;
    }

    if (hoveredButton != nullptr) {
        *keyCodePtr = hoveredButton->mouseExitEventCode;

        unsigned char* data;
        if ((hoveredButton->flags & BUTTON_FLAG_0x01) && (hoveredButton->flags & BUTTON_FLAG_CHECKED)) {
            data = hoveredButton->pressedImage;
        } else {
            data = hoveredButton->normalImage;
        }

        hoveredButton->draw(this, data, true, nullptr, true);

        this->hoveredButton = nullptr;
    }

    if (*keyCodePtr != -1) {
        last_button_winID = id;

        if ((hoveredButton->flags & BUTTON_FLAG_DISABLED) == 0) {
            if (hoveredButton->mouseExitProc != nullptr) {
                hoveredButton->mouseExitProc(hoveredButton->id, *keyCodePtr);
                if (!(hoveredButton->flags & BUTTON_FLAG_0x40)) {
                    *keyCodePtr = -1;
                }
            }
        }
        return 0;
    }

    if (hoveredButton != nullptr) {
        if ((hoveredButton->flags & BUTTON_FLAG_DISABLED) == 0) {
            if (hoveredButton->mouseExitProc != nullptr) {
                hoveredButton->mouseExitProc(hoveredButton->id, *keyCodePtr);
            }
        }
    }

    return -1;
}

// 0x4C52CC
bool Button::underMouse(Rect* rect)
{
    if (!mouse_click_in(rect->ulx, rect->uly, rect->lrx, rect->lry)) {
        return false;
    }

    if (mask == nullptr) {
        return true;
    }

    int x;
    int y;
    mouse_get_position(&x, &y);
    x -= rect->ulx;
    y -= rect->uly;

    int width = this->rect.lrx - this->rect.ulx + 1;
    return mask[width * y + x] != 0;
}

// 0x4C5334
int win_button_winID(int btn)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    Window* w;
    if (GNW_find_button(btn, &w) == nullptr) {
        return -1;
    }

    return w->id;
}

// 0x4C536C
int win_last_button_winID()
{
    return last_button_winID;
}

// 0x4C5374
int win_delete_button(int btn)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    Window* w;
    Button* button = GNW_find_button(btn, &w);
    if (button == nullptr) {
        return -1;
    }

    if (button->prev != nullptr) {
        button->prev->next = button->next;
    } else {
        w->buttonListHead = button->next;
    }

    if (button->next != nullptr) {
        button->next->prev = button->prev;
    }

    win_fill(w->id, button->rect.ulx, button->rect.uly, button->rect.lrx - button->rect.ulx + 1, button->rect.lry - button->rect.uly + 1, w->color);

    if (button == w->hoveredButton) {
        w->hoveredButton = nullptr;
    }

    if (button == w->clickedButton) {
        w->clickedButton = nullptr;
    }

    button->destroy();

    return 0;
}

// 0x4C542C
void Button::destroy()
{
    if ((flags & BUTTON_FLAG_GRAPHIC) == 0) {
        if (normalImage != nullptr) {
            mem_free(normalImage);
        }

        if (pressedImage != nullptr) {
            mem_free(pressedImage);
        }

        if (hoverImage != nullptr) {
            mem_free(hoverImage);
        }

        if (disabledNormalImage != nullptr) {
            mem_free(disabledNormalImage);
        }

        if (disabledPressedImage != nullptr) {
            mem_free(disabledPressedImage);
        }

        if (disabledHoverImage != nullptr) {
            mem_free(disabledHoverImage);
        }
    }

    ButtonGroup* buttonGroup = this->buttonGroup;
    if (buttonGroup != nullptr) {
        for (int index = 0; index < buttonGroup->buttonsLength; index++) {
            if (this == buttonGroup->buttons[index]) {
                for (; index < buttonGroup->buttonsLength - 1; index++) {
                    buttonGroup->buttons[index] = buttonGroup->buttons[index + 1];
                }

                buttonGroup->buttonsLength--;

                break;
            }
        }
    }

    mem_free(this);
}

// 0x4C54E8
void win_delete_button_win(int btn, int inputEvent)
{
    Button* button;
    Window* w;

    button = GNW_find_button(btn, &w);
    if (button != nullptr) {
        win_delete(w->id);
        GNW_add_input_buffer(inputEvent);
    }
}

// 0x4C5510
int button_new_id()
{
    int btn;

    btn = 1;
    while (GNW_find_button(btn, nullptr) != nullptr) {
        btn++;
    }

    return btn;
}

// 0x4C552C
int win_enable_button(int btn)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    Window* w;
    Button* button = GNW_find_button(btn, &w);
    if (button == nullptr) {
        return -1;
    }

    if ((button->flags & BUTTON_FLAG_DISABLED) != 0) {
        button->flags &= ~BUTTON_FLAG_DISABLED;
        button->draw(w, button->currentImage, true, nullptr, false);
    }

    return 0;
}

// 0x4C5588
int win_disable_button(int btn)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    Window* w;
    Button* button = GNW_find_button(btn, &w);
    if (button == nullptr) {
        return -1;
    }

    if ((button->flags & BUTTON_FLAG_DISABLED) == 0) {
        button->flags |= BUTTON_FLAG_DISABLED;

        button->draw(w, button->currentImage, true, nullptr, false);

        if (button == w->hoveredButton) {
            if (w->hoveredButton->mouseExitEventCode != -1) {
                GNW_add_input_buffer(w->hoveredButton->mouseExitEventCode);
                w->hoveredButton = nullptr;
            }
        }
    }

    return 0;
}

// 0x4C560C
int win_set_button_rest_state(int btn, bool checked, int flags)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    Window* w;
    Button* button = GNW_find_button(btn, &w);
    if (button == nullptr) {
        return -1;
    }

    if ((button->flags & BUTTON_FLAG_0x01) != 0) {
        int keyCode = -1;

        if ((button->flags & BUTTON_FLAG_CHECKED) != 0) {
            if (!checked) {
                button->flags &= ~BUTTON_FLAG_CHECKED;

                if ((flags & 0x02) == 0) {
                    button->draw(w, button->normalImage, true, nullptr, false);
                }

                if (button->buttonGroup != nullptr) {
                    button->buttonGroup->currChecked--;
                }

                keyCode = button->leftMouseUpEventCode;
            }
        } else {
            if (checked) {
                button->flags |= BUTTON_FLAG_CHECKED;

                if ((flags & 0x02) == 0) {
                    button->draw(w, button->pressedImage, true, nullptr, false);
                }

                if (button->buttonGroup != nullptr) {
                    button->buttonGroup->currChecked++;
                }

                keyCode = button->lefMouseDownEventCode;
            }
        }

        if (keyCode != -1) {
            if ((flags & 0x01) != 0) {
                GNW_add_input_buffer(keyCode);
            }
        }
    }

    return 0;
}

// 0x4C56E4
int win_group_check_buttons(int buttonCount, int* btns, int maxChecked, RadioButtonCallback* func)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    if (buttonCount >= BUTTON_GROUP_BUTTON_LIST_CAPACITY) {
        return -1;
    }

    for (int groupIndex = 0; groupIndex < BUTTON_GROUP_LIST_CAPACITY; groupIndex++) {
        ButtonGroup* buttonGroup = &(btn_grp[groupIndex]);
        if (buttonGroup->buttonsLength == 0) {
            buttonGroup->currChecked = 0;

            for (int buttonIndex = 0; buttonIndex < buttonCount; buttonIndex++) {
                Button* button = GNW_find_button(btns[buttonIndex], nullptr);
                if (button == nullptr) {
                    return -1;
                }

                buttonGroup->buttons[buttonIndex] = button;

                button->buttonGroup = buttonGroup;

                if ((button->flags & BUTTON_FLAG_CHECKED) != 0) {
                    buttonGroup->currChecked++;
                }
            }

            buttonGroup->buttonsLength = buttonCount;
            buttonGroup->maxChecked = maxChecked;
            buttonGroup->func = func;
            return 0;
        }
    }

    return -1;
}

// 0x4C57A4
int win_group_radio_buttons(int buttonCount, int* btns)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    if (win_group_check_buttons(buttonCount, btns, 1, nullptr) == -1) {
        return -1;
    }

    Button* button = GNW_find_button(btns[0], nullptr);
    ButtonGroup* buttonGroup = button->buttonGroup;

    for (int index = 0; index < buttonGroup->buttonsLength; index++) {
        Button* v1 = buttonGroup->buttons[index];
        v1->flags |= BUTTON_FLAG_RADIO;
    }

    return 0;
}

// 0x4C57FC
int Button::checkGroup()
{
    if (buttonGroup == nullptr) {
        return 0;
    }

    if ((flags & BUTTON_FLAG_RADIO) != 0) {
        if (buttonGroup->currChecked > 0) {
            for (int index = 0; index < buttonGroup->buttonsLength; index++) {
                Button* otherButton = buttonGroup->buttons[index];
                if ((otherButton->flags & BUTTON_FLAG_CHECKED) != 0) {
                    otherButton->flags &= ~BUTTON_FLAG_CHECKED;

                    Window* w;
                    GNW_find_button(otherButton->id, &w);
                    otherButton->draw(w, otherButton->normalImage, true, nullptr, true);

                    if (otherButton->leftMouseUpProc != nullptr) {
                        otherButton->leftMouseUpProc(otherButton->id, otherButton->leftMouseUpEventCode);
                    }
                }
            }
        }

        if ((flags & BUTTON_FLAG_CHECKED) == 0) {
            buttonGroup->currChecked++;
        }

        return 0;
    }

    if (buttonGroup->currChecked < buttonGroup->maxChecked) {
        if ((flags & BUTTON_FLAG_CHECKED) == 0) {
            buttonGroup->currChecked++;
        }

        return 0;
    }

    if (buttonGroup->func != nullptr) {
        buttonGroup->func(id);
    }

    return -1;
}

// 0x4C58C0
void Button::draw(Window* w, unsigned char* data, bool draw, Rect* bound, bool sound)
{
    unsigned char* previousImage = nullptr;
    if (data != nullptr) {
        Rect v2;
        v2 = rect;
        v2.offset(w->rect.ulx, w->rect.uly);

        Rect v3;
        if (bound != nullptr) {
            if (v2.insideBound(*bound, v2) == -1) {
                return;
            }

            v3 = v2;
            v3.offset(-w->rect.ulx, -w->rect.uly);
        } else {
            v3 = rect;
        }

        if (data == normalImage && (flags & BUTTON_FLAG_CHECKED)) {
            data = pressedImage;
        }

        if (flags & BUTTON_FLAG_DISABLED) {
            if (data == normalImage) {
                data = disabledNormalImage;
            } else if (data == pressedImage) {
                data = disabledPressedImage;
            } else if (data == hoverImage) {
                data = disabledHoverImage;
            }
        } else {
            if (data == disabledNormalImage) {
                data = normalImage;
            } else if (data == disabledPressedImage) {
                data = pressedImage;
            } else if (data == disabledHoverImage) {
                data = hoverImage;
            }
        }

        if (data) {
            if (!draw) {
                int width = rect.lrx - rect.ulx + 1;
                if ((flags & BUTTON_FLAG_TRANSPARENT) != 0) {
                    trans_buf_to_buf(
                        data + (v3.uly - rect.uly) * width + v3.ulx - rect.ulx,
                        v3.lrx - v3.ulx + 1,
                        v3.lry - v3.uly + 1,
                        width,
                        w->buffer + w->width * v3.uly + v3.ulx,
                        w->width);
                } else {
                    buf_to_buf(
                        data + (v3.uly - rect.uly) * width + v3.ulx - rect.ulx,
                        v3.lrx - v3.ulx + 1,
                        v3.lry - v3.uly + 1,
                        width,
                        w->buffer + w->width * v3.uly + v3.ulx,
                        w->width);
                }
            }

            previousImage = currentImage;
            currentImage = data;

            if (draw) {
                w->winRefresh(&v2, 0);
            }
        }
    }

    if (sound) {
        if (previousImage != data) {
            if (data == pressedImage && pressSoundFunc != nullptr) {
                pressSoundFunc(id, lefMouseDownEventCode);
            } else if (data == normalImage && releaseSoundFunc != nullptr) {
                releaseSoundFunc(id, leftMouseUpEventCode);
            }
        }
    }
}

// 0x4C5B10
void Window::buttonRefresh(Rect* rect)
{
    Button* button = buttonListHead;
    if (button != nullptr) {
        while (button->next != nullptr) {
            button = button->next;
        }
    }

    while (button != nullptr) {
        button->draw(this, button->currentImage, false, rect, false);
        button = button->prev;
    }
}

// 0x4C5B58
int win_button_press_and_release(int btn)
{
    if (!GNW_win_init_flag) {
        return -1;
    }

    Window* w;
    Button* button = GNW_find_button(btn, &w);
    if (button == nullptr) {
        return -1;
    }

    button->draw(w, button->pressedImage, true, nullptr, true);

    if (button->leftMouseDownProc != nullptr) {
        button->leftMouseDownProc(btn, button->lefMouseDownEventCode);

        if ((button->flags & BUTTON_FLAG_0x40) != 0) {
            GNW_add_input_buffer(button->lefMouseDownEventCode);
        }
    } else {
        if (button->lefMouseDownEventCode != -1) {
            GNW_add_input_buffer(button->lefMouseDownEventCode);
        }
    }

    button->draw(w, button->normalImage, true, nullptr, true);

    if (button->leftMouseUpProc != nullptr) {
        button->leftMouseUpProc(btn, button->leftMouseUpEventCode);

        if ((button->flags & BUTTON_FLAG_0x40) != 0) {
            GNW_add_input_buffer(button->leftMouseUpEventCode);
        }
    } else {
        if (button->leftMouseUpEventCode != -1) {
            GNW_add_input_buffer(button->leftMouseUpEventCode);
        }
    }

    return 0;
}

} // namespace fallout
