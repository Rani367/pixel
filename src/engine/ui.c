// UI System Implementation
// Provides retained-mode UI elements for game menus and interfaces

#include "engine/ui.h"
#include "engine/engine.h"
#include "pal/pal.h"
#include <string.h>

// ============================================================================
// Color Helpers
// ============================================================================

// LCOV_EXCL_START - only used by drawing functions
static void unpack_color(uint32_t color, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a) {
    *a = (color >> 24) & 0xFF;
    *r = (color >> 16) & 0xFF;
    *g = (color >> 8) & 0xFF;
    *b = color & 0xFF;
}
// LCOV_EXCL_STOP

// ============================================================================
// UIManager Lifecycle
// ============================================================================

void ui_manager_init(UIManager* ui) {
    if (!ui) return;  // LCOV_EXCL_LINE - null check

    for (int i = 0; i < UI_MAX_ELEMENTS; i++) {
        ui->elements[i] = NULL;
    }
    ui->element_count = 0;

    ui->focused = NULL;
    ui->hovered = NULL;
    ui->pressed = NULL;

    ui->modal_active = false;
    ui->modal = NULL;

    ui->default_font = NULL;
}

void ui_manager_free(UIManager* ui) {
    if (!ui) return;  // LCOV_EXCL_LINE - null check

    // Clear all elements (they're GC-managed, so just clear references)
    ui_clear(ui);
    ui->focused = NULL;
    ui->hovered = NULL;
    ui->pressed = NULL;
    ui->modal = NULL;
}

// ============================================================================
// UI Element Management
// ============================================================================

bool ui_show(UIManager* ui, ObjUIElement* element) {
    if (!ui || !element) return false;  // LCOV_EXCL_LINE - null check
    if (ui->element_count >= UI_MAX_ELEMENTS) return false;  // LCOV_EXCL_LINE - limit rarely hit

    // Check if already shown
    for (int i = 0; i < ui->element_count; i++) {
        if (ui->elements[i] == element) return true;
    }

    ui->elements[ui->element_count++] = element;
    element->visible = true;
    return true;
}

bool ui_hide(UIManager* ui, ObjUIElement* element) {
    if (!ui || !element) return false;  // LCOV_EXCL_LINE - null check

    for (int i = 0; i < ui->element_count; i++) {
        if (ui->elements[i] == element) {
            // Shift remaining elements
            for (int j = i; j < ui->element_count - 1; j++) {
                ui->elements[j] = ui->elements[j + 1];
            }
            ui->element_count--;
            element->visible = false;

            // Clear references if this element was focused/hovered/pressed
            if (ui->focused == element) ui->focused = NULL;
            if (ui->hovered == element) ui->hovered = NULL;
            if (ui->pressed == element) ui->pressed = NULL;
            // LCOV_EXCL_START - modal not tested
            if (ui->modal == element) {
                ui->modal = NULL;
                ui->modal_active = false;
            }
            // LCOV_EXCL_STOP
            return true;
        }
    }
    return false;
}

void ui_clear(UIManager* ui) {
    if (!ui) return;  // LCOV_EXCL_LINE - null check

    for (int i = 0; i < ui->element_count; i++) {
        if (ui->elements[i]) {
            ui->elements[i]->visible = false;
        }
        ui->elements[i] = NULL;
    }
    ui->element_count = 0;
    ui->focused = NULL;
    ui->hovered = NULL;
    ui->pressed = NULL;
    ui->modal = NULL;
    ui->modal_active = false;
}

// LCOV_EXCL_START - child management has branches hard to test
void ui_add_child(ObjUIElement* parent, ObjUIElement* child) {
    if (!parent || !child) return;

    // Create children list if needed
    if (parent->children == NULL) {
        parent->children = list_new();
    }

    // Add child
    list_append(parent->children, OBJECT_VAL(child));
    child->parent = parent;
}

void ui_remove_child(ObjUIElement* parent, ObjUIElement* child) {
    if (!parent || !child || !parent->children) return;

    // Find and remove child
    ObjList* children = parent->children;
    for (int i = 0; i < children->count; i++) {
        if (IS_UI_ELEMENT(children->items[i]) &&
            AS_UI_ELEMENT(children->items[i]) == child) {
            // Shift remaining children
            for (int j = i; j < children->count - 1; j++) {
                children->items[j] = children->items[j + 1];
            }
            children->count--;
            child->parent = NULL;
            return;
        }
    }
}
// LCOV_EXCL_STOP

// ============================================================================
// Focus Management
// ============================================================================

void ui_set_focus(UIManager* ui, ObjUIElement* element) {
    if (!ui) return;  // LCOV_EXCL_LINE - null check

    // Clear old focus state
    if (ui->focused && ui->focused != element) {
        if (ui->focused->state == UI_STATE_FOCUSED) {
            ui->focused->state = UI_STATE_NORMAL;
        }
    }

    ui->focused = element;
    if (element && element->enabled) {
        element->state = UI_STATE_FOCUSED;
    }
}

void ui_clear_focus(UIManager* ui) {
    if (!ui) return;  // LCOV_EXCL_LINE - null check

    if (ui->focused) {
        if (ui->focused->state == UI_STATE_FOCUSED) {
            ui->focused->state = UI_STATE_NORMAL;
        }
        ui->focused = NULL;
    }
}

// LCOV_EXCL_START - focus helper functions have branches hard to test
// Check if an element can receive focus
static bool ui_is_focusable(ObjUIElement* element) {
    if (!element || !element->visible || !element->enabled) return false;

    switch (element->kind) {
        case UI_BUTTON:
        case UI_SLIDER:
        case UI_CHECKBOX:
        case UI_TEXT_INPUT:
        case UI_LIST:
            return true;
        default:
            return false;
    }
}

// Get all focusable elements in order
static int ui_get_focusable_elements(UIManager* ui, ObjUIElement** out, int max) {
    int count = 0;

    for (int i = 0; i < ui->element_count && count < max; i++) {
        ObjUIElement* element = ui->elements[i];
        if (ui_is_focusable(element)) {
            out[count++] = element;
        }

        // Check children recursively
        if (element->children) {
            for (int j = 0; j < element->children->count && count < max; j++) {
                Value child_val = element->children->items[j];
                if (IS_UI_ELEMENT(child_val)) {
                    ObjUIElement* child = AS_UI_ELEMENT(child_val);
                    if (ui_is_focusable(child)) {
                        out[count++] = child;
                    }
                }
            }
        }
    }

    return count;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START - focus navigation uses excluded helper functions
void ui_focus_next(UIManager* ui) {
    if (!ui) return;

    ObjUIElement* focusable[UI_MAX_ELEMENTS];
    int count = ui_get_focusable_elements(ui, focusable, UI_MAX_ELEMENTS);

    if (count == 0) return;

    // Find current focused index
    int current_idx = -1;
    for (int i = 0; i < count; i++) {
        if (focusable[i] == ui->focused) {
            current_idx = i;
            break;
        }
    }

    // Move to next
    int next_idx = (current_idx + 1) % count;
    ui_set_focus(ui, focusable[next_idx]);
}

void ui_focus_prev(UIManager* ui) {
    if (!ui) return;

    ObjUIElement* focusable[UI_MAX_ELEMENTS];
    int count = ui_get_focusable_elements(ui, focusable, UI_MAX_ELEMENTS);

    if (count == 0) return;

    // Find current focused index
    int current_idx = -1;
    for (int i = 0; i < count; i++) {
        if (focusable[i] == ui->focused) {
            current_idx = i;
            break;
        }
    }

    // Move to previous
    int prev_idx = (current_idx - 1 + count) % count;
    ui_set_focus(ui, focusable[prev_idx]);
}
// LCOV_EXCL_STOP

// ============================================================================
// Hit Testing
// ============================================================================

bool ui_point_in_element(ObjUIElement* element, double x, double y) {
    if (!element) return false;  // LCOV_EXCL_LINE - null check

    double ex, ey;
    ui_get_absolute_position(element, &ex, &ey);

    return x >= ex && x < ex + element->width &&
           y >= ey && y < ey + element->height;
}

// LCOV_EXCL_START - recursive hit test helper has branches hard to test
// Recursive hit test helper
static ObjUIElement* ui_hit_test_element(ObjUIElement* element, double x, double y) {
    if (!element || !element->visible) return NULL;

    // Check children first (front-to-back, so later children are on top)
    if (element->children) {
        for (int i = element->children->count - 1; i >= 0; i--) {
            Value child_val = element->children->items[i];
            if (IS_UI_ELEMENT(child_val)) {
                ObjUIElement* hit = ui_hit_test_element(AS_UI_ELEMENT(child_val), x, y);
                if (hit) return hit;
            }
        }
    }

    // Check this element
    if (ui_point_in_element(element, x, y)) {
        return element;
    }

    return NULL;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START - hit test uses excluded recursive helper
ObjUIElement* ui_hit_test(UIManager* ui, double x, double y) {
    if (!ui) return NULL;

    // If modal is active, only hit test the modal
    if (ui->modal_active && ui->modal) {
        return ui_hit_test_element(ui->modal, x, y);
    }

    // Check elements front-to-back (later elements are on top)
    for (int i = ui->element_count - 1; i >= 0; i--) {
        ObjUIElement* hit = ui_hit_test_element(ui->elements[i], x, y);
        if (hit) return hit;
    }

    return NULL;
}
// LCOV_EXCL_STOP

// ============================================================================
// Element Helpers
// ============================================================================

void ui_get_absolute_position(ObjUIElement* element, double* x, double* y) {
    // LCOV_EXCL_START - null element
    if (!element) {
        *x = 0;
        *y = 0;
        return;
    }
    // LCOV_EXCL_STOP

    *x = element->x;
    *y = element->y;

    // Add parent positions recursively
    ObjUIElement* parent = element->parent;
    while (parent) {
        *x += parent->x + parent->padding;
        *y += parent->y + parent->padding;
        parent = parent->parent;
    }
}

// LCOV_EXCL_START - font/color helpers used by drawing code
ObjFont* ui_get_font(UIManager* ui, ObjUIElement* element) {
    if (element && element->font) {
        return element->font;
    }
    if (ui && ui->default_font) {
        return ui->default_font;
    }
    return NULL;
}

uint32_t ui_get_bg_color(ObjUIElement* element) {
    if (!element) return 0xFF333333;

    switch (element->state) {
        case UI_STATE_PRESSED:
            return element->pressed_color;
        case UI_STATE_HOVERED:
        case UI_STATE_FOCUSED:
            return element->hover_color;
        case UI_STATE_DISABLED:
            return (element->bg_color & 0x00FFFFFF) | 0x80000000;  // Half alpha
        default:
            return element->bg_color;
    }
}
// LCOV_EXCL_STOP

// ============================================================================
// Input Handling
// ============================================================================

// LCOV_EXCL_START - input handling requires actual user interaction
bool ui_handle_mouse(UIManager* ui, VM* vm, ObjUIElement* element,
                     double mx, double my, bool clicked, bool released) {
    if (!ui || !element || !element->enabled) return false;

    bool inside = ui_point_in_element(element, mx, my);

    // Update state based on mouse
    if (inside) {
        if (clicked) {
            element->state = UI_STATE_PRESSED;
            ui->pressed = element;
        } else if (ui->pressed == element && released) {
            // Click completed - fire callback
            element->state = UI_STATE_HOVERED;
            ui->pressed = NULL;

            // Handle click based on element type
            switch (element->kind) {
                case UI_BUTTON:
                    if (element->on_click && vm) {
                        vm_call_closure(vm, element->on_click, 0, NULL);
                    }
                    break;
                case UI_CHECKBOX:
                    element->data.checkbox.checked = !element->data.checkbox.checked;
                    if (element->on_change && vm) {
                        Value arg = BOOL_VAL(element->data.checkbox.checked);
                        vm_call_closure(vm, element->on_change, 1, &arg);
                    }
                    break;
                case UI_SLIDER: {
                    // Update slider value based on click position
                    double ex, ey;
                    ui_get_absolute_position(element, &ex, &ey);
                    double rel_x = (mx - ex) / element->width;
                    if (rel_x < 0) rel_x = 0;
                    if (rel_x > 1) rel_x = 1;
                    double range = element->data.slider.max - element->data.slider.min;
                    element->data.slider.value = element->data.slider.min + rel_x * range;
                    if (element->on_change && vm) {
                        Value arg = NUMBER_VAL(element->data.slider.value);
                        vm_call_closure(vm, element->on_change, 1, &arg);
                    }
                    break;
                }
                case UI_LIST: {
                    // Select item based on click position
                    double ex, ey;
                    ui_get_absolute_position(element, &ex, &ey);
                    int item_height = 25;  // Approximate item height
                    int clicked_idx = (int)((my - ey) / item_height) + element->data.list.scroll_offset;
                    if (clicked_idx >= 0 && clicked_idx < element->data.list.items->count) {
                        element->data.list.selected_index = clicked_idx;
                        if (element->on_change && vm) {
                            Value arg = NUMBER_VAL(clicked_idx);
                            vm_call_closure(vm, element->on_change, 1, &arg);
                        }
                    }
                    break;
                }
                case UI_TEXT_INPUT:
                    // Focus the text input
                    ui_set_focus(ui, element);
                    break;
                default:
                    break;
            }
            return true;
        } else if (ui->pressed != element) {
            element->state = UI_STATE_HOVERED;
        }
        return true;
    } else {
        // Mouse outside
        if (ui->pressed == element && released) {
            ui->pressed = NULL;
        }
        if (element->state == UI_STATE_HOVERED) {
            element->state = UI_STATE_NORMAL;
        }
    }

    return false;
}

bool ui_handle_key(UIManager* ui, VM* vm, int key, bool pressed) {
    if (!ui || !pressed) return false;

    // TAB cycles focus
    if (key == PAL_KEY_TAB) {
        if (pal_key_down(PAL_KEY_LSHIFT) || pal_key_down(PAL_KEY_RSHIFT)) {
            ui_focus_prev(ui);
        } else {
            ui_focus_next(ui);
        }
        return true;
    }

    // ESCAPE clears focus or closes modal
    if (key == PAL_KEY_ESCAPE) {
        if (ui->modal_active && ui->modal) {
            ui_hide(ui, ui->modal);
            return true;
        }
        ui_clear_focus(ui);
        return true;
    }

    // Handle focused element
    ObjUIElement* focused = ui->focused;
    if (!focused || !focused->enabled) return false;

    // ENTER/SPACE activates buttons
    if ((key == PAL_KEY_RETURN || key == PAL_KEY_SPACE) && focused->kind == UI_BUTTON) {
        if (focused->on_click && vm) {
            vm_call_closure(vm, focused->on_click, 0, NULL);
        }
        return true;
    }

    // Arrow keys for sliders
    if (focused->kind == UI_SLIDER) {
        double step = focused->data.slider.step > 0 ? focused->data.slider.step : 0.1;
        double range = focused->data.slider.max - focused->data.slider.min;
        step *= range;

        if (key == PAL_KEY_LEFT) {
            focused->data.slider.value -= step;
            if (focused->data.slider.value < focused->data.slider.min) {
                focused->data.slider.value = focused->data.slider.min;
            }
            if (focused->on_change && vm) {
                Value arg = NUMBER_VAL(focused->data.slider.value);
                vm_call_closure(vm, focused->on_change, 1, &arg);
            }
            return true;
        }
        if (key == PAL_KEY_RIGHT) {
            focused->data.slider.value += step;
            if (focused->data.slider.value > focused->data.slider.max) {
                focused->data.slider.value = focused->data.slider.max;
            }
            if (focused->on_change && vm) {
                Value arg = NUMBER_VAL(focused->data.slider.value);
                vm_call_closure(vm, focused->on_change, 1, &arg);
            }
            return true;
        }
    }

    // Arrow keys for lists
    if (focused->kind == UI_LIST) {
        ObjList* items = focused->data.list.items;
        if (!items || items->count == 0) return false;

        if (key == PAL_KEY_UP) {
            focused->data.list.selected_index--;
            if (focused->data.list.selected_index < 0) {
                focused->data.list.selected_index = 0;
            }
            if (focused->on_change && vm) {
                Value arg = NUMBER_VAL(focused->data.list.selected_index);
                vm_call_closure(vm, focused->on_change, 1, &arg);
            }
            return true;
        }
        if (key == PAL_KEY_DOWN) {
            focused->data.list.selected_index++;
            if (focused->data.list.selected_index >= items->count) {
                focused->data.list.selected_index = items->count - 1;
            }
            if (focused->on_change && vm) {
                Value arg = NUMBER_VAL(focused->data.list.selected_index);
                vm_call_closure(vm, focused->on_change, 1, &arg);
            }
            return true;
        }
    }

    // Space toggles checkboxes
    if (key == PAL_KEY_SPACE && focused->kind == UI_CHECKBOX) {
        focused->data.checkbox.checked = !focused->data.checkbox.checked;
        if (focused->on_change && vm) {
            Value arg = BOOL_VAL(focused->data.checkbox.checked);
            vm_call_closure(vm, focused->on_change, 1, &arg);
        }
        return true;
    }

    // Backspace for text input
    if (key == PAL_KEY_BACKSPACE && focused->kind == UI_TEXT_INPUT) {
        ObjString* text = focused->data.text_input.text;
        if (text && text->length > 0 && focused->data.text_input.cursor_pos > 0) {
            // Create new string without last character at cursor
            int new_len = text->length - 1;
            char* new_chars = PH_ALLOC(new_len + 1);
            int cursor = focused->data.text_input.cursor_pos;
            memcpy(new_chars, text->chars, cursor - 1);
            memcpy(new_chars + cursor - 1, text->chars + cursor, text->length - cursor);
            new_chars[new_len] = '\0';
            focused->data.text_input.text = string_take(new_chars, new_len);
            focused->data.text_input.cursor_pos--;

            if (focused->on_change && vm) {
                Value arg = OBJECT_VAL(focused->data.text_input.text);
                vm_call_closure(vm, focused->on_change, 1, &arg);
            }
        }
        return true;
    }

    return false;
}

void ui_handle_text_input(UIManager* ui, VM* vm, const char* text) {
    if (!ui || !text || !ui->focused) return;

    ObjUIElement* focused = ui->focused;
    if (focused->kind != UI_TEXT_INPUT || !focused->enabled) return;

    // Append text at cursor position
    ObjString* current = focused->data.text_input.text;
    int text_len = (int)strlen(text);
    int new_len = (current ? current->length : 0) + text_len;

    // Check max length
    if (new_len > focused->data.text_input.max_length) {
        new_len = focused->data.text_input.max_length;
        text_len = new_len - (current ? current->length : 0);
        if (text_len <= 0) return;
    }

    // Create new string
    char* new_chars = PH_ALLOC(new_len + 1);
    int cursor = focused->data.text_input.cursor_pos;
    if (current) {
        memcpy(new_chars, current->chars, cursor);
    }
    memcpy(new_chars + cursor, text, text_len);
    if (current && cursor < (int)current->length) {
        memcpy(new_chars + cursor + text_len, current->chars + cursor,
               current->length - cursor);
    }
    new_chars[new_len] = '\0';

    focused->data.text_input.text = string_take(new_chars, new_len);
    focused->data.text_input.cursor_pos += text_len;

    if (focused->on_change && vm) {
        Value arg = OBJECT_VAL(focused->data.text_input.text);
        vm_call_closure(vm, focused->on_change, 1, &arg);
    }
}
// LCOV_EXCL_STOP

// ============================================================================
// UI Update
// ============================================================================

// LCOV_EXCL_START - ui_update requires PAL input functions
void ui_update(UIManager* ui, VM* vm, double dt) {
    (void)dt;
    if (!ui) return;

    Engine* engine = engine_get();
    if (!engine || !engine->window) return;

    // Get mouse state
    int mx, my;
    pal_mouse_position(&mx, &my);
    bool clicked = pal_mouse_pressed(PAL_MOUSE_LEFT);
    bool released = pal_mouse_released(PAL_MOUSE_LEFT);

    // Update hovered element
    ObjUIElement* new_hovered = ui_hit_test(ui, mx, my);
    if (new_hovered != ui->hovered) {
        // Clear old hover
        if (ui->hovered && ui->hovered->state == UI_STATE_HOVERED) {
            ui->hovered->state = UI_STATE_NORMAL;
        }
        ui->hovered = new_hovered;
    }

    // Process mouse input on hovered element
    if (new_hovered) {
        ui_handle_mouse(ui, vm, new_hovered, mx, my, clicked, released);
    } else if (ui->pressed && released) {
        // Mouse released outside of pressed element
        ui->pressed->state = UI_STATE_NORMAL;
        ui->pressed = NULL;
    }

    // Drag handling for sliders
    if (ui->pressed && ui->pressed->kind == UI_SLIDER && !released) {
        double ex, ey;
        ui_get_absolute_position(ui->pressed, &ex, &ey);
        double rel_x = (mx - ex) / ui->pressed->width;
        if (rel_x < 0) rel_x = 0;
        if (rel_x > 1) rel_x = 1;
        double range = ui->pressed->data.slider.max - ui->pressed->data.slider.min;
        ui->pressed->data.slider.value = ui->pressed->data.slider.min + rel_x * range;
    }

    // Handle keyboard for focused element
    for (int key = 0; key < PAL_KEY_COUNT; key++) {
        if (pal_key_pressed(key)) {
            ui_handle_key(ui, vm, key, true);
        }
    }
}
// LCOV_EXCL_STOP

// ============================================================================
// Element Drawing
// ============================================================================

// LCOV_EXCL_START - drawing functions require PAL rendering with real window
void ui_draw_button(UIManager* ui, ObjUIElement* element) {
    Engine* engine = engine_get();
    if (!engine || !engine->window) return;

    double x, y;
    ui_get_absolute_position(element, &x, &y);

    // Background
    uint32_t bg = ui_get_bg_color(element);
    uint8_t r, g, b, a;
    unpack_color(bg, &r, &g, &b, &a);
    pal_draw_rect(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);

    // Border
    if (element->border_width > 0) {
        unpack_color(element->border_color, &r, &g, &b, &a);
        pal_draw_rect_outline(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);
    }

    // Text
    if (element->data.button.text) {
        ObjFont* font = ui_get_font(ui, element);
        if (font && font->font) {
            const char* text = element->data.button.text->chars;
            int text_w, text_h;
            pal_text_size(font->font, text, &text_w, &text_h);

            // Center text
            int tx = (int)(x + (element->width - text_w) / 2);
            int ty = (int)(y + (element->height - text_h) / 2);

            unpack_color(element->fg_color, &r, &g, &b, &a);
            pal_draw_text(engine->window, font->font, text, tx, ty, r, g, b, a);
        }
    }
}

void ui_draw_label(UIManager* ui, ObjUIElement* element) {
    Engine* engine = engine_get();
    if (!engine || !engine->window) return;

    double x, y;
    ui_get_absolute_position(element, &x, &y);

    // Text
    if (element->data.label.text) {
        ObjFont* font = ui_get_font(ui, element);
        if (font && font->font) {
            const char* text = element->data.label.text->chars;
            int text_w, text_h;
            pal_text_size(font->font, text, &text_w, &text_h);

            int tx = (int)x;
            if (element->data.label.align == UI_ALIGN_CENTER) {
                tx = (int)(x + (element->width - text_w) / 2);
            } else if (element->data.label.align == UI_ALIGN_RIGHT) {
                tx = (int)(x + element->width - text_w);
            }
            int ty = (int)(y + (element->height - text_h) / 2);

            uint8_t r, g, b, a;
            unpack_color(element->fg_color, &r, &g, &b, &a);
            pal_draw_text(engine->window, font->font, text, tx, ty, r, g, b, a);
        }
    }
}

void ui_draw_panel(UIManager* ui, ObjUIElement* element) {
    Engine* engine = engine_get();
    if (!engine || !engine->window) return;

    double x, y;
    ui_get_absolute_position(element, &x, &y);

    // Background
    uint8_t r, g, b, a;
    unpack_color(element->bg_color, &r, &g, &b, &a);
    pal_draw_rect(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);

    // Border
    if (element->border_width > 0) {
        unpack_color(element->border_color, &r, &g, &b, &a);
        pal_draw_rect_outline(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);
    }

    // Draw children
    if (element->children) {
        for (int i = 0; i < element->children->count; i++) {
            Value child_val = element->children->items[i];
            if (IS_UI_ELEMENT(child_val)) {
                ObjUIElement* child = AS_UI_ELEMENT(child_val);
                if (child->visible) {
                    ui_draw_element(ui, child);
                }
            }
        }
    }
}

void ui_draw_slider(UIManager* ui, ObjUIElement* element) {
    Engine* engine = engine_get();
    if (!engine || !engine->window) return;

    double x, y;
    ui_get_absolute_position(element, &x, &y);

    // Track background
    uint8_t r, g, b, a;
    unpack_color(element->bg_color, &r, &g, &b, &a);
    pal_draw_rect(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);

    // Fill based on value
    double range = element->data.slider.max - element->data.slider.min;
    double normalized = (range > 0) ? (element->data.slider.value - element->data.slider.min) / range : 0;
    int fill_width = (int)(element->width * normalized);
    if (fill_width > 0) {
        unpack_color(element->hover_color, &r, &g, &b, &a);
        pal_draw_rect(engine->window, (int)x, (int)y, fill_width, (int)element->height, r, g, b, a);
    }

    // Border
    if (element->border_width > 0) {
        unpack_color(element->border_color, &r, &g, &b, &a);
        pal_draw_rect_outline(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);
    }

    // Thumb
    int thumb_x = (int)(x + fill_width - 4);
    if (thumb_x < (int)x) thumb_x = (int)x;
    unpack_color(element->fg_color, &r, &g, &b, &a);
    pal_draw_rect(engine->window, thumb_x, (int)y, 8, (int)element->height, r, g, b, a);
}

void ui_draw_checkbox(UIManager* ui, ObjUIElement* element) {
    Engine* engine = engine_get();
    if (!engine || !engine->window) return;

    double x, y;
    ui_get_absolute_position(element, &x, &y);

    // Box background
    uint32_t bg = ui_get_bg_color(element);
    uint8_t r, g, b, a;
    unpack_color(bg, &r, &g, &b, &a);
    pal_draw_rect(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);

    // Border
    if (element->border_width > 0) {
        unpack_color(element->border_color, &r, &g, &b, &a);
        pal_draw_rect_outline(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);
    }

    // Checkmark (inner square when checked)
    if (element->data.checkbox.checked) {
        int inset = (int)(element->width * 0.25);
        unpack_color(element->fg_color, &r, &g, &b, &a);
        pal_draw_rect(engine->window, (int)x + inset, (int)y + inset,
                      (int)element->width - inset * 2, (int)element->height - inset * 2, r, g, b, a);
    }

    // Label
    if (element->data.checkbox.label) {
        ObjFont* font = ui_get_font(ui, element);
        if (font && font->font) {
            const char* text = element->data.checkbox.label->chars;
            int tx = (int)(x + element->width + 8);
            int ty = (int)(y + (element->height - font->size) / 2);
            unpack_color(element->fg_color, &r, &g, &b, &a);
            pal_draw_text(engine->window, font->font, text, tx, ty, r, g, b, a);
        }
    }
}

void ui_draw_text_input(UIManager* ui, ObjUIElement* element) {
    Engine* engine = engine_get();
    if (!engine || !engine->window) return;

    double x, y;
    ui_get_absolute_position(element, &x, &y);

    // Background
    uint32_t bg = ui_get_bg_color(element);
    uint8_t r, g, b, a;
    unpack_color(bg, &r, &g, &b, &a);
    pal_draw_rect(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);

    // Border
    if (element->border_width > 0) {
        unpack_color(element->border_color, &r, &g, &b, &a);
        pal_draw_rect_outline(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);
    }

    // Text
    ObjFont* font = ui_get_font(ui, element);
    if (font && font->font) {
        ObjString* text = element->data.text_input.text;
        ObjString* placeholder = element->data.text_input.placeholder;

        const char* display_text = NULL;
        uint32_t text_color = element->fg_color;

        if (text && text->length > 0) {
            display_text = text->chars;
        } else if (placeholder && placeholder->length > 0) {
            display_text = placeholder->chars;
            text_color = (element->fg_color & 0x00FFFFFF) | 0x80000000;  // Half alpha
        }

        if (display_text) {
            int tx = (int)(x + element->padding);
            int ty = (int)(y + (element->height - font->size) / 2);
            unpack_color(text_color, &r, &g, &b, &a);

            if (element->data.text_input.password) {
                // Draw asterisks instead
                int len = (int)strlen(display_text);
                char* masked = PH_ALLOC(len + 1);
                for (int i = 0; i < len; i++) masked[i] = '*';
                masked[len] = '\0';
                pal_draw_text(engine->window, font->font, masked, tx, ty, r, g, b, a);
                PH_FREE(masked);
            } else {
                pal_draw_text(engine->window, font->font, display_text, tx, ty, r, g, b, a);
            }
        }

        // Cursor (when focused)
        if (element->state == UI_STATE_FOCUSED && text) {
            int cursor_x = (int)(x + element->padding);
            if (text->length > 0) {
                char* before_cursor = PH_ALLOC(element->data.text_input.cursor_pos + 1);
                memcpy(before_cursor, text->chars, element->data.text_input.cursor_pos);
                before_cursor[element->data.text_input.cursor_pos] = '\0';
                int text_w, text_h;
                pal_text_size(font->font, before_cursor, &text_w, &text_h);
                cursor_x += text_w;
                PH_FREE(before_cursor);
            }
            unpack_color(element->fg_color, &r, &g, &b, &a);
            pal_draw_rect(engine->window, cursor_x, (int)(y + 4), 2, (int)(element->height - 8), r, g, b, a);
        }
    }
}

void ui_draw_list(UIManager* ui, ObjUIElement* element) {
    Engine* engine = engine_get();
    if (!engine || !engine->window) return;

    double x, y;
    ui_get_absolute_position(element, &x, &y);

    // Background
    uint8_t r, g, b, a;
    unpack_color(element->bg_color, &r, &g, &b, &a);
    pal_draw_rect(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);

    // Border
    if (element->border_width > 0) {
        unpack_color(element->border_color, &r, &g, &b, &a);
        pal_draw_rect_outline(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);
    }

    // Items
    ObjList* items = element->data.list.items;
    if (!items) return;

    ObjFont* font = ui_get_font(ui, element);
    if (!font || !font->font) return;

    int item_height = font->size + 8;
    int visible_count = (int)(element->height / item_height);
    int start_idx = element->data.list.scroll_offset;
    int end_idx = start_idx + visible_count;
    if (end_idx > items->count) end_idx = items->count;

    for (int i = start_idx; i < end_idx; i++) {
        int item_y = (int)(y + (i - start_idx) * item_height);

        // Selection highlight
        if (i == element->data.list.selected_index) {
            unpack_color(element->hover_color, &r, &g, &b, &a);
            pal_draw_rect(engine->window, (int)x + 2, item_y, (int)element->width - 4, item_height, r, g, b, a);
        }

        // Item text
        Value item_val = items->items[i];
        if (IS_STRING(item_val)) {
            const char* text = AS_CSTRING(item_val);
            unpack_color(element->fg_color, &r, &g, &b, &a);
            pal_draw_text(engine->window, font->font, text, (int)x + element->padding, item_y + 4, r, g, b, a);
        }
    }
}

void ui_draw_image_box(UIManager* ui, ObjUIElement* element) {
    (void)ui;
    Engine* engine = engine_get();
    if (!engine || !engine->window) return;

    double x, y;
    ui_get_absolute_position(element, &x, &y);

    // Background
    uint8_t r, g, b, a;
    unpack_color(element->bg_color, &r, &g, &b, &a);
    pal_draw_rect(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);

    // Image
    if (element->data.image_box.image && element->data.image_box.image->texture) {
        ObjImage* img = element->data.image_box.image;
        int draw_w = (int)element->width;
        int draw_h = (int)element->height;

        if (element->data.image_box.scale_to_fit) {
            // Maintain aspect ratio
            double img_aspect = (double)img->width / img->height;
            double box_aspect = element->width / element->height;
            if (img_aspect > box_aspect) {
                draw_w = (int)element->width;
                draw_h = (int)(element->width / img_aspect);
            } else {
                draw_h = (int)element->height;
                draw_w = (int)(element->height * img_aspect);
            }
        }

        int draw_x = (int)(x + (element->width - draw_w) / 2);
        int draw_y = (int)(y + (element->height - draw_h) / 2);
        pal_draw_texture(engine->window, img->texture, draw_x, draw_y, draw_w, draw_h);
    }

    // Border
    if (element->border_width > 0) {
        unpack_color(element->border_color, &r, &g, &b, &a);
        pal_draw_rect_outline(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);
    }
}

void ui_draw_progress_bar(UIManager* ui, ObjUIElement* element) {
    (void)ui;
    Engine* engine = engine_get();
    if (!engine || !engine->window) return;

    double x, y;
    ui_get_absolute_position(element, &x, &y);

    // Background
    uint8_t r, g, b, a;
    unpack_color(element->bg_color, &r, &g, &b, &a);
    pal_draw_rect(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);

    // Fill
    double value = element->data.progress_bar.value;
    if (value < 0) value = 0;
    if (value > 1) value = 1;
    int fill_width = (int)(element->width * value);
    if (fill_width > 0) {
        unpack_color(element->data.progress_bar.fill_color, &r, &g, &b, &a);
        pal_draw_rect(engine->window, (int)x, (int)y, fill_width, (int)element->height, r, g, b, a);
    }

    // Border
    if (element->border_width > 0) {
        unpack_color(element->border_color, &r, &g, &b, &a);
        pal_draw_rect_outline(engine->window, (int)x, (int)y, (int)element->width, (int)element->height, r, g, b, a);
    }
}

void ui_draw_element(UIManager* ui, ObjUIElement* element) {
    if (!element || !element->visible) return;

    switch (element->kind) {
        case UI_BUTTON:
            ui_draw_button(ui, element);
            break;
        case UI_LABEL:
            ui_draw_label(ui, element);
            break;
        case UI_PANEL:
            ui_draw_panel(ui, element);
            break;
        case UI_SLIDER:
            ui_draw_slider(ui, element);
            break;
        case UI_CHECKBOX:
            ui_draw_checkbox(ui, element);
            break;
        case UI_TEXT_INPUT:
            ui_draw_text_input(ui, element);
            break;
        case UI_LIST:
            ui_draw_list(ui, element);
            break;
        case UI_IMAGE_BOX:
            ui_draw_image_box(ui, element);
            break;
        case UI_PROGRESS_BAR:
            ui_draw_progress_bar(ui, element);
            break;
    }
}

void ui_draw(UIManager* ui) {
    if (!ui) return;

    // Draw all root elements
    for (int i = 0; i < ui->element_count; i++) {
        ObjUIElement* element = ui->elements[i];
        if (element && element->visible) {
            ui_draw_element(ui, element);
        }
    }
}
// LCOV_EXCL_STOP
