// UI Native Functions Implementation
// Exposes UI system to Pixel code

// LCOV_EXCL_START - native wrappers require VM context for testing

#include "engine/ui_natives.h"
#include "engine/ui_menus.h"
#include "engine/ui.h"
#include "engine/engine.h"
#include "runtime/stdlib.h"
#include "vm/object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

// ============================================================================
// Helper Functions
// ============================================================================

// Helper for runtime errors from native functions
static Value ui_native_error(const char* message) {
    fprintf(stderr, "Runtime error: %s\n", message);
    return NONE_VAL;
}

// Get the UI manager from the engine
static UIManager* get_ui_manager(void) {
    Engine* engine = engine_get();
    return engine ? engine->ui : NULL;
}

// ============================================================================
// Element Creation Functions
// ============================================================================

// ui_button(x, y, width, height, text) -> UIElement
static Value native_ui_button(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) ||
        !IS_NUMBER(args[2]) || !IS_NUMBER(args[3]) || !IS_STRING(args[4])) {
        return ui_native_error("ui_button() requires (x, y, width, height, text)");
    }

    ObjUIElement* button = ui_element_new(UI_BUTTON);
    button->x = AS_NUMBER(args[0]);
    button->y = AS_NUMBER(args[1]);
    button->width = AS_NUMBER(args[2]);
    button->height = AS_NUMBER(args[3]);
    button->data.button.text = AS_STRING(args[4]);

    return OBJECT_VAL(button);
}

// ui_label(x, y, text) -> UIElement
static Value native_ui_label(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) || !IS_STRING(args[2])) {
        return ui_native_error("ui_label() requires (x, y, text)");
    }

    ObjUIElement* label = ui_element_new(UI_LABEL);
    label->x = AS_NUMBER(args[0]);
    label->y = AS_NUMBER(args[1]);
    label->data.label.text = AS_STRING(args[2]);

    // Auto-size based on text (approximate)
    label->width = label->data.label.text->length * 10;

    return OBJECT_VAL(label);
}

// ui_panel(x, y, width, height) -> UIElement
static Value native_ui_panel(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) ||
        !IS_NUMBER(args[2]) || !IS_NUMBER(args[3])) {
        return ui_native_error("ui_panel() requires (x, y, width, height)");
    }

    ObjUIElement* panel = ui_element_new(UI_PANEL);
    panel->x = AS_NUMBER(args[0]);
    panel->y = AS_NUMBER(args[1]);
    panel->width = AS_NUMBER(args[2]);
    panel->height = AS_NUMBER(args[3]);

    return OBJECT_VAL(panel);
}

// ui_slider(x, y, width, min, max, value) -> UIElement
static Value native_ui_slider(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) || !IS_NUMBER(args[2]) ||
        !IS_NUMBER(args[3]) || !IS_NUMBER(args[4]) || !IS_NUMBER(args[5])) {
        return ui_native_error("ui_slider() requires (x, y, width, min, max, value)");
    }

    ObjUIElement* slider = ui_element_new(UI_SLIDER);
    slider->x = AS_NUMBER(args[0]);
    slider->y = AS_NUMBER(args[1]);
    slider->width = AS_NUMBER(args[2]);
    slider->data.slider.min = AS_NUMBER(args[3]);
    slider->data.slider.max = AS_NUMBER(args[4]);
    slider->data.slider.value = AS_NUMBER(args[5]);

    return OBJECT_VAL(slider);
}

// ui_checkbox(x, y, label, checked) -> UIElement
static Value native_ui_checkbox(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) ||
        !IS_STRING(args[2]) || !IS_BOOL(args[3])) {
        return ui_native_error("ui_checkbox() requires (x, y, label, checked)");
    }

    ObjUIElement* checkbox = ui_element_new(UI_CHECKBOX);
    checkbox->x = AS_NUMBER(args[0]);
    checkbox->y = AS_NUMBER(args[1]);
    checkbox->data.checkbox.label = AS_STRING(args[2]);
    checkbox->data.checkbox.checked = AS_BOOL(args[3]);

    return OBJECT_VAL(checkbox);
}

// ui_text_input(x, y, width, placeholder) -> UIElement
static Value native_ui_text_input(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) ||
        !IS_NUMBER(args[2]) || !IS_STRING(args[3])) {
        return ui_native_error("ui_text_input() requires (x, y, width, placeholder)");
    }

    ObjUIElement* input = ui_element_new(UI_TEXT_INPUT);
    input->x = AS_NUMBER(args[0]);
    input->y = AS_NUMBER(args[1]);
    input->width = AS_NUMBER(args[2]);
    input->data.text_input.placeholder = AS_STRING(args[3]);

    return OBJECT_VAL(input);
}

// ui_list(x, y, width, height, items) -> UIElement
static Value native_ui_list(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) ||
        !IS_NUMBER(args[2]) || !IS_NUMBER(args[3]) || !IS_LIST(args[4])) {
        return ui_native_error("ui_list() requires (x, y, width, height, items)");
    }

    ObjUIElement* list = ui_element_new(UI_LIST);
    list->x = AS_NUMBER(args[0]);
    list->y = AS_NUMBER(args[1]);
    list->width = AS_NUMBER(args[2]);
    list->height = AS_NUMBER(args[3]);

    // Copy items
    ObjList* src = AS_LIST(args[4]);
    for (int i = 0; i < src->count; i++) {
        list_append(list->data.list.items, src->items[i]);
    }

    return OBJECT_VAL(list);
}

// ui_progress_bar(x, y, width, height, value) -> UIElement
static Value native_ui_progress_bar(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) ||
        !IS_NUMBER(args[2]) || !IS_NUMBER(args[3]) || !IS_NUMBER(args[4])) {
        return ui_native_error("ui_progress_bar() requires (x, y, width, height, value)");
    }

    ObjUIElement* bar = ui_element_new(UI_PROGRESS_BAR);
    bar->x = AS_NUMBER(args[0]);
    bar->y = AS_NUMBER(args[1]);
    bar->width = AS_NUMBER(args[2]);
    bar->height = AS_NUMBER(args[3]);
    bar->data.progress_bar.value = AS_NUMBER(args[4]);

    return OBJECT_VAL(bar);
}

// ============================================================================
// Element Configuration Functions
// ============================================================================

// ui_set_text(element, text) -> nil
static Value native_ui_set_text(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_STRING(args[1])) {
        return ui_native_error("ui_set_text() requires (element, text)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    ObjString* text = AS_STRING(args[1]);

    switch (element->kind) {
        case UI_BUTTON:
            element->data.button.text = text;
            break;
        case UI_LABEL:
            element->data.label.text = text;
            break;
        case UI_TEXT_INPUT:
            element->data.text_input.text = text;
            element->data.text_input.cursor_pos = text->length;
            break;
        default:
            return ui_native_error("ui_set_text() not applicable to this element type");
    }

    return NONE_VAL;
}

// ui_get_text(element) -> string
static Value native_ui_get_text(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0])) {
        return ui_native_error("ui_get_text() requires an element");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);

    switch (element->kind) {
        case UI_BUTTON:
            return element->data.button.text ? OBJECT_VAL(element->data.button.text) : NONE_VAL;
        case UI_LABEL:
            return element->data.label.text ? OBJECT_VAL(element->data.label.text) : NONE_VAL;
        case UI_TEXT_INPUT:
            return element->data.text_input.text ? OBJECT_VAL(element->data.text_input.text) : NONE_VAL;
        default:
            return NONE_VAL;
    }
}

// ui_set_value(element, value) -> nil
static Value native_ui_set_value(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_NUMBER(args[1])) {
        return ui_native_error("ui_set_value() requires (element, value)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    double value = AS_NUMBER(args[1]);

    switch (element->kind) {
        case UI_SLIDER:
            element->data.slider.value = value;
            break;
        case UI_PROGRESS_BAR:
            element->data.progress_bar.value = value;
            break;
        default:
            return ui_native_error("ui_set_value() not applicable to this element type");
    }

    return NONE_VAL;
}

// ui_get_value(element) -> number
static Value native_ui_get_value(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0])) {
        return ui_native_error("ui_get_value() requires an element");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);

    switch (element->kind) {
        case UI_SLIDER:
            return NUMBER_VAL(element->data.slider.value);
        case UI_PROGRESS_BAR:
            return NUMBER_VAL(element->data.progress_bar.value);
        default:
            return NUMBER_VAL(0);
    }
}

// ui_set_checked(element, checked) -> nil
static Value native_ui_set_checked(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_BOOL(args[1])) {
        return ui_native_error("ui_set_checked() requires (element, checked)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    if (element->kind != UI_CHECKBOX) {
        return ui_native_error("ui_set_checked() only works on checkboxes");
    }

    element->data.checkbox.checked = AS_BOOL(args[1]);
    return NONE_VAL;
}

// ui_is_checked(element) -> bool
static Value native_ui_is_checked(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0])) {
        return ui_native_error("ui_is_checked() requires an element");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    if (element->kind != UI_CHECKBOX) {
        return BOOL_VAL(false);
    }

    return BOOL_VAL(element->data.checkbox.checked);
}

// ui_set_enabled(element, enabled) -> nil
static Value native_ui_set_enabled(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_BOOL(args[1])) {
        return ui_native_error("ui_set_enabled() requires (element, enabled)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    element->enabled = AS_BOOL(args[1]);
    if (!element->enabled) {
        element->state = UI_STATE_DISABLED;
    } else if (element->state == UI_STATE_DISABLED) {
        element->state = UI_STATE_NORMAL;
    }

    return NONE_VAL;
}

// ui_set_visible(element, visible) -> nil
static Value native_ui_set_visible(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_BOOL(args[1])) {
        return ui_native_error("ui_set_visible() requires (element, visible)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    element->visible = AS_BOOL(args[1]);

    return NONE_VAL;
}

// ui_set_position(element, x, y) -> nil
static Value native_ui_set_position(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return ui_native_error("ui_set_position() requires (element, x, y)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    element->x = AS_NUMBER(args[1]);
    element->y = AS_NUMBER(args[2]);

    return NONE_VAL;
}

// ui_set_size(element, width, height) -> nil
static Value native_ui_set_size(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return ui_native_error("ui_set_size() requires (element, width, height)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    element->width = AS_NUMBER(args[1]);
    element->height = AS_NUMBER(args[2]);

    return NONE_VAL;
}

// ============================================================================
// Styling Functions
// ============================================================================

// ui_set_colors(element, bg, fg, border) -> nil
static Value native_ui_set_colors(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_NUMBER(args[1]) ||
        !IS_NUMBER(args[2]) || !IS_NUMBER(args[3])) {
        return ui_native_error("ui_set_colors() requires (element, bg, fg, border)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    element->bg_color = (uint32_t)AS_NUMBER(args[1]);
    element->fg_color = (uint32_t)AS_NUMBER(args[2]);
    element->border_color = (uint32_t)AS_NUMBER(args[3]);

    return NONE_VAL;
}

// ui_set_hover_color(element, color) -> nil
static Value native_ui_set_hover_color(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_NUMBER(args[1])) {
        return ui_native_error("ui_set_hover_color() requires (element, color)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    element->hover_color = (uint32_t)AS_NUMBER(args[1]);

    return NONE_VAL;
}

// ui_set_font(element, font) -> nil
static Value native_ui_set_font(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_FONT(args[1])) {
        return ui_native_error("ui_set_font() requires (element, font)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    element->font = AS_FONT(args[1]);

    return NONE_VAL;
}

// ui_set_padding(element, padding) -> nil
static Value native_ui_set_padding(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_NUMBER(args[1])) {
        return ui_native_error("ui_set_padding() requires (element, padding)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    element->padding = (int)AS_NUMBER(args[1]);

    return NONE_VAL;
}

// ui_set_border(element, width) -> nil
static Value native_ui_set_border(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_NUMBER(args[1])) {
        return ui_native_error("ui_set_border() requires (element, width)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    element->border_width = (int)AS_NUMBER(args[1]);

    return NONE_VAL;
}

// ============================================================================
// Callback Functions
// ============================================================================

// ui_on_click(element, callback) -> nil
static Value native_ui_on_click(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_CLOSURE(args[1])) {
        return ui_native_error("ui_on_click() requires (element, callback)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    element->on_click = AS_CLOSURE(args[1]);

    return NONE_VAL;
}

// ui_on_change(element, callback) -> nil
static Value native_ui_on_change(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_CLOSURE(args[1])) {
        return ui_native_error("ui_on_change() requires (element, callback)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    element->on_change = AS_CLOSURE(args[1]);

    return NONE_VAL;
}

// ============================================================================
// Hierarchy Functions
// ============================================================================

// ui_add_child(parent, child) -> nil
static Value native_ui_add_child(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_UI_ELEMENT(args[1])) {
        return ui_native_error("ui_add_child() requires (parent, child)");
    }

    ObjUIElement* parent = AS_UI_ELEMENT(args[0]);
    ObjUIElement* child = AS_UI_ELEMENT(args[1]);
    ui_add_child(parent, child);

    return NONE_VAL;
}

// ui_remove_child(parent, child) -> nil
static Value native_ui_remove_child(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_UI_ELEMENT(args[1])) {
        return ui_native_error("ui_remove_child() requires (parent, child)");
    }

    ObjUIElement* parent = AS_UI_ELEMENT(args[0]);
    ObjUIElement* child = AS_UI_ELEMENT(args[1]);
    ui_remove_child(parent, child);

    return NONE_VAL;
}

// ui_show(element) -> nil
static Value native_ui_show(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0])) {
        return ui_native_error("ui_show() requires an element");
    }

    UIManager* ui = get_ui_manager();
    if (!ui) return NONE_VAL;

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    ui_show(ui, element);

    return NONE_VAL;
}

// ui_hide(element) -> nil
static Value native_ui_hide(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0])) {
        return ui_native_error("ui_hide() requires an element");
    }

    UIManager* ui = get_ui_manager();
    if (!ui) return NONE_VAL;

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    ui_hide(ui, element);

    return NONE_VAL;
}

// ui_destroy(element) -> nil
static Value native_ui_destroy(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0])) {
        return ui_native_error("ui_destroy() requires an element");
    }

    UIManager* ui = get_ui_manager();
    if (!ui) return NONE_VAL;

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    ui_hide(ui, element);
    // Element will be garbage collected when no longer referenced

    return NONE_VAL;
}

// ============================================================================
// List-Specific Functions
// ============================================================================

// ui_list_add(list, item) -> nil
static Value native_ui_list_add(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_STRING(args[1])) {
        return ui_native_error("ui_list_add() requires (list, item)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    if (element->kind != UI_LIST) {
        return ui_native_error("ui_list_add() requires a list element");
    }

    list_append(element->data.list.items, args[1]);
    return NONE_VAL;
}

// ui_list_remove(list, index) -> nil
static Value native_ui_list_remove(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_NUMBER(args[1])) {
        return ui_native_error("ui_list_remove() requires (list, index)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    if (element->kind != UI_LIST) {
        return ui_native_error("ui_list_remove() requires a list element");
    }

    int index = (int)AS_NUMBER(args[1]);
    ObjList* items = element->data.list.items;

    if (index >= 0 && index < items->count) {
        for (int i = index; i < items->count - 1; i++) {
            items->items[i] = items->items[i + 1];
        }
        items->count--;

        if (element->data.list.selected_index >= items->count) {
            element->data.list.selected_index = items->count - 1;
        }
    }

    return NONE_VAL;
}

// ui_list_clear(list) -> nil
static Value native_ui_list_clear(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0])) {
        return ui_native_error("ui_list_clear() requires a list element");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    if (element->kind != UI_LIST) {
        return ui_native_error("ui_list_clear() requires a list element");
    }

    element->data.list.items->count = 0;
    element->data.list.selected_index = -1;
    element->data.list.scroll_offset = 0;

    return NONE_VAL;
}

// ui_list_selected(list) -> number
static Value native_ui_list_selected(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0])) {
        return ui_native_error("ui_list_selected() requires a list element");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    if (element->kind != UI_LIST) {
        return NUMBER_VAL(-1);
    }

    return NUMBER_VAL(element->data.list.selected_index);
}

// ui_list_set_selected(list, index) -> nil
static Value native_ui_list_set_selected(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_UI_ELEMENT(args[0]) || !IS_NUMBER(args[1])) {
        return ui_native_error("ui_list_set_selected() requires (list, index)");
    }

    ObjUIElement* element = AS_UI_ELEMENT(args[0]);
    if (element->kind != UI_LIST) {
        return ui_native_error("ui_list_set_selected() requires a list element");
    }

    element->data.list.selected_index = (int)AS_NUMBER(args[1]);
    return NONE_VAL;
}

// ============================================================================
// Settings Persistence
// ============================================================================

#define SETTINGS_MAX_ENTRIES 64
#define SETTINGS_MAX_KEY_LEN 64
#define SETTINGS_MAX_VALUE_LEN 256

typedef struct {
    char key[SETTINGS_MAX_KEY_LEN];
    Value value;
} SettingsEntry;

static struct {
    SettingsEntry entries[SETTINGS_MAX_ENTRIES];
    int count;
    bool loaded;
} settings_store = {0};

// Get settings file path
static void get_settings_path(char* buffer, size_t size) {
    const char* home = getenv("HOME");
    if (home) {
        snprintf(buffer, size, "%s/.pixel/settings.dat", home);
    } else {
        snprintf(buffer, size, ".pixel/settings.dat");
    }
}

// Ensure settings directory exists
static void ensure_settings_dir(void) {
    char dir_path[512];
    const char* home = getenv("HOME");
    if (home) {
        snprintf(dir_path, sizeof(dir_path), "%s/.pixel", home);
    } else {
        snprintf(dir_path, sizeof(dir_path), ".pixel");
    }

    // Try to create directory (ignore if exists)
    #ifdef _WIN32
    _mkdir(dir_path);
    #else
    mkdir(dir_path, 0755);
    #endif
}

// Find entry by key
static int find_setting(const char* key) {
    for (int i = 0; i < settings_store.count; i++) {
        if (strcmp(settings_store.entries[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

// set_setting(key, value) -> nil
static Value native_set_setting(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_STRING(args[0])) {
        return ui_native_error("set_setting() requires (key, value)");
    }

    const char* key = AS_CSTRING(args[0]);
    Value value = args[1];

    int index = find_setting(key);
    if (index >= 0) {
        settings_store.entries[index].value = value;
    } else if (settings_store.count < SETTINGS_MAX_ENTRIES) {
        size_t key_len = strlen(key);
        if (key_len >= SETTINGS_MAX_KEY_LEN) {
            key_len = SETTINGS_MAX_KEY_LEN - 1;
        }
        memcpy(settings_store.entries[settings_store.count].key, key, key_len);
        settings_store.entries[settings_store.count].key[key_len] = '\0';
        settings_store.entries[settings_store.count].value = value;
        settings_store.count++;
    }

    return NONE_VAL;
}

// get_setting(key) -> value
static Value native_get_setting(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_STRING(args[0])) {
        return ui_native_error("get_setting() requires a key");
    }

    const char* key = AS_CSTRING(args[0]);
    int index = find_setting(key);

    if (index >= 0) {
        return settings_store.entries[index].value;
    }

    return NONE_VAL;
}

// save_settings() -> bool
static Value native_save_settings(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    ensure_settings_dir();

    char path[512];
    get_settings_path(path, sizeof(path));

    FILE* file = fopen(path, "w");
    if (!file) {
        return BOOL_VAL(false);
    }

    for (int i = 0; i < settings_store.count; i++) {
        SettingsEntry* entry = &settings_store.entries[i];

        // Write key
        fprintf(file, "%s=", entry->key);

        // Write value based on type
        if (IS_NUMBER(entry->value)) {
            fprintf(file, "N:%g\n", AS_NUMBER(entry->value));
        } else if (IS_BOOL(entry->value)) {
            fprintf(file, "B:%d\n", AS_BOOL(entry->value) ? 1 : 0);
        } else if (IS_STRING(entry->value)) {
            fprintf(file, "S:%s\n", AS_CSTRING(entry->value));
        } else {
            fprintf(file, "X:\n");  // Unknown/nil
        }
    }

    fclose(file);
    return BOOL_VAL(true);
}

// load_settings() -> bool
static Value native_load_settings(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    char path[512];
    get_settings_path(path, sizeof(path));

    FILE* file = fopen(path, "r");
    if (!file) {
        return BOOL_VAL(false);
    }

    // Clear existing settings
    settings_store.count = 0;

    char line[SETTINGS_MAX_KEY_LEN + SETTINGS_MAX_VALUE_LEN + 4];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Find '='
        char* eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        const char* key = line;
        const char* type_value = eq + 1;

        if (strlen(type_value) < 2 || type_value[1] != ':') continue;

        char type = type_value[0];
        const char* value_str = type_value + 2;

        Value value = NONE_VAL;
        switch (type) {
            case 'N':
                value = NUMBER_VAL(atof(value_str));
                break;
            case 'B':
                value = BOOL_VAL(atoi(value_str) != 0);
                break;
            case 'S': {
                ObjString* str = string_copy(value_str, (int)strlen(value_str));
                value = OBJECT_VAL(str);
                break;
            }
            default:
                break;
        }

        if (settings_store.count < SETTINGS_MAX_ENTRIES) {
            size_t key_len = strlen(key);
            if (key_len >= SETTINGS_MAX_KEY_LEN) {
                key_len = SETTINGS_MAX_KEY_LEN - 1;
            }
            memcpy(settings_store.entries[settings_store.count].key, key, key_len);
            settings_store.entries[settings_store.count].key[key_len] = '\0';
            settings_store.entries[settings_store.count].value = value;
            settings_store.count++;
        }
    }

    fclose(file);
    settings_store.loaded = true;
    return BOOL_VAL(true);
}

// ============================================================================
// Registration
// ============================================================================

void ui_natives_init(VM* vm) {
    // Element creation
    define_native(vm, "ui_button", native_ui_button, 5);
    define_native(vm, "ui_label", native_ui_label, 3);
    define_native(vm, "ui_panel", native_ui_panel, 4);
    define_native(vm, "ui_slider", native_ui_slider, 6);
    define_native(vm, "ui_checkbox", native_ui_checkbox, 4);
    define_native(vm, "ui_text_input", native_ui_text_input, 4);
    define_native(vm, "ui_list", native_ui_list, 5);
    define_native(vm, "ui_progress_bar", native_ui_progress_bar, 5);

    // Element configuration
    define_native(vm, "ui_set_text", native_ui_set_text, 2);
    define_native(vm, "ui_get_text", native_ui_get_text, 1);
    define_native(vm, "ui_set_value", native_ui_set_value, 2);
    define_native(vm, "ui_get_value", native_ui_get_value, 1);
    define_native(vm, "ui_set_checked", native_ui_set_checked, 2);
    define_native(vm, "ui_is_checked", native_ui_is_checked, 1);
    define_native(vm, "ui_set_enabled", native_ui_set_enabled, 2);
    define_native(vm, "ui_set_visible", native_ui_set_visible, 2);
    define_native(vm, "ui_set_position", native_ui_set_position, 3);
    define_native(vm, "ui_set_size", native_ui_set_size, 3);

    // Styling
    define_native(vm, "ui_set_colors", native_ui_set_colors, 4);
    define_native(vm, "ui_set_hover_color", native_ui_set_hover_color, 2);
    define_native(vm, "ui_set_font", native_ui_set_font, 2);
    define_native(vm, "ui_set_padding", native_ui_set_padding, 2);
    define_native(vm, "ui_set_border", native_ui_set_border, 2);

    // Callbacks
    define_native(vm, "ui_on_click", native_ui_on_click, 2);
    define_native(vm, "ui_on_change", native_ui_on_change, 2);

    // Hierarchy
    define_native(vm, "ui_add_child", native_ui_add_child, 2);
    define_native(vm, "ui_remove_child", native_ui_remove_child, 2);
    define_native(vm, "ui_show", native_ui_show, 1);
    define_native(vm, "ui_hide", native_ui_hide, 1);
    define_native(vm, "ui_destroy", native_ui_destroy, 1);

    // List operations
    define_native(vm, "ui_list_add", native_ui_list_add, 2);
    define_native(vm, "ui_list_remove", native_ui_list_remove, 2);
    define_native(vm, "ui_list_clear", native_ui_list_clear, 1);
    define_native(vm, "ui_list_selected", native_ui_list_selected, 1);
    define_native(vm, "ui_list_set_selected", native_ui_list_set_selected, 2);

    // Settings persistence
    define_native(vm, "set_setting", native_set_setting, 2);
    define_native(vm, "get_setting", native_get_setting, 1);
    define_native(vm, "save_settings", native_save_settings, 0);
    define_native(vm, "load_settings", native_load_settings, 0);

    // Initialize pre-built menu functions
    ui_menus_init(vm);
}

// LCOV_EXCL_STOP
