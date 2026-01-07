// Pre-Built Menu Functions
// Provides ready-to-use menu implementations for common game UI patterns

// LCOV_EXCL_START - menu functions require full engine setup

#include "engine/ui_menus.h"
#include "engine/ui.h"
#include "engine/engine.h"
#include "runtime/stdlib.h"
#include "vm/object.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// Helper Functions
// ============================================================================

static Value menu_native_error(const char* message) {
    fprintf(stderr, "Runtime error: %s\n", message);
    return NONE_VAL;
}

static UIManager* get_ui_manager(void) {
    Engine* engine = engine_get();
    return engine ? engine->ui : NULL;
}

static int get_window_width(void) {
    Engine* engine = engine_get();
    return engine ? engine_get_width(engine) : 800;
}

static int get_window_height(void) {
    Engine* engine = engine_get();
    return engine ? engine_get_height(engine) : 600;
}

// ============================================================================
// Main Menu
// ============================================================================

// main_menu(title, options) -> UIElement
// options is a list of [label, callback] pairs
// Creates a centered vertical menu with title and buttons
static Value native_main_menu(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_STRING(args[0]) || !IS_LIST(args[1])) {
        return menu_native_error("main_menu() requires (title, options)");
    }

    UIManager* ui = get_ui_manager();
    if (!ui) return NONE_VAL;

    ObjString* title = AS_STRING(args[0]);
    ObjList* options = AS_LIST(args[1]);

    int win_width = get_window_width();
    int win_height = get_window_height();

    // Menu dimensions
    int menu_width = 300;
    int button_height = 50;
    int button_spacing = 10;
    int title_height = 60;
    int padding = 20;

    int num_buttons = options->count;
    int menu_height = title_height + padding + (button_height + button_spacing) * num_buttons + padding;

    // Center the menu
    int menu_x = (win_width - menu_width) / 2;
    int menu_y = (win_height - menu_height) / 2;

    // Create panel
    ObjUIElement* panel = ui_element_new(UI_PANEL);
    panel->x = menu_x;
    panel->y = menu_y;
    panel->width = menu_width;
    panel->height = menu_height;
    panel->bg_color = 0xE0303030;  // Semi-transparent dark background

    // Create title label
    ObjUIElement* title_label = ui_element_new(UI_LABEL);
    title_label->x = menu_width / 2;
    title_label->y = padding;
    title_label->data.label.text = title;
    title_label->data.label.align = UI_ALIGN_CENTER;
    title_label->fg_color = 0xFFFFFFFF;
    ui_add_child(panel, title_label);

    // Create buttons for each option
    int button_y = title_height + padding;
    for (int i = 0; i < options->count; i++) {
        Value option = options->items[i];
        if (!IS_LIST(option)) continue;

        ObjList* opt_pair = AS_LIST(option);
        if (opt_pair->count < 2) continue;

        ObjString* label = NULL;
        ObjClosure* callback = NULL;

        if (IS_STRING(opt_pair->items[0])) {
            label = AS_STRING(opt_pair->items[0]);
        }
        if (IS_CLOSURE(opt_pair->items[1])) {
            callback = AS_CLOSURE(opt_pair->items[1]);
        }

        if (!label) continue;

        ObjUIElement* button = ui_element_new(UI_BUTTON);
        button->x = padding;
        button->y = button_y;
        button->width = menu_width - 2 * padding;
        button->height = button_height;
        button->data.button.text = label;
        button->on_click = callback;
        button->bg_color = 0xFF4A4A4A;
        button->hover_color = 0xFF5A5A5A;
        button->pressed_color = 0xFF3A3A3A;
        button->fg_color = 0xFFFFFFFF;

        ui_add_child(panel, button);
        button_y += button_height + button_spacing;
    }

    // Show the panel
    ui_show(ui, panel);

    return OBJECT_VAL(panel);
}

// ============================================================================
// Pause Menu
// ============================================================================

// pause_menu(on_resume, on_quit) -> UIElement
// Creates a simple pause menu with Resume and Quit buttons
static Value native_pause_menu(int arg_count, Value* args) {
    (void)arg_count;

    UIManager* ui = get_ui_manager();
    if (!ui) return NONE_VAL;

    ObjClosure* on_resume = NULL;
    ObjClosure* on_quit = NULL;

    if (IS_CLOSURE(args[0])) {
        on_resume = AS_CLOSURE(args[0]);
    }
    if (IS_CLOSURE(args[1])) {
        on_quit = AS_CLOSURE(args[1]);
    }

    int win_width = get_window_width();
    int win_height = get_window_height();

    // Menu dimensions
    int menu_width = 250;
    int button_height = 50;
    int button_spacing = 15;
    int title_height = 50;
    int padding = 20;
    int menu_height = title_height + padding + (button_height + button_spacing) * 2 + padding;

    // Center the menu
    int menu_x = (win_width - menu_width) / 2;
    int menu_y = (win_height - menu_height) / 2;

    // Create panel with dark overlay effect
    ObjUIElement* panel = ui_element_new(UI_PANEL);
    panel->x = menu_x;
    panel->y = menu_y;
    panel->width = menu_width;
    panel->height = menu_height;
    panel->bg_color = 0xF0202020;

    // Title
    ObjUIElement* title_label = ui_element_new(UI_LABEL);
    title_label->x = menu_width / 2;
    title_label->y = padding;
    title_label->data.label.text = string_copy("PAUSED", 6);
    title_label->data.label.align = UI_ALIGN_CENTER;
    title_label->fg_color = 0xFFFFFFFF;
    ui_add_child(panel, title_label);

    int button_y = title_height + padding;

    // Resume button
    ObjUIElement* resume_btn = ui_element_new(UI_BUTTON);
    resume_btn->x = padding;
    resume_btn->y = button_y;
    resume_btn->width = menu_width - 2 * padding;
    resume_btn->height = button_height;
    resume_btn->data.button.text = string_copy("Resume", 6);
    resume_btn->on_click = on_resume;
    resume_btn->bg_color = 0xFF4CAF50;  // Green
    resume_btn->hover_color = 0xFF66BB6A;
    resume_btn->pressed_color = 0xFF388E3C;
    resume_btn->fg_color = 0xFFFFFFFF;
    ui_add_child(panel, resume_btn);

    button_y += button_height + button_spacing;

    // Quit button
    ObjUIElement* quit_btn = ui_element_new(UI_BUTTON);
    quit_btn->x = padding;
    quit_btn->y = button_y;
    quit_btn->width = menu_width - 2 * padding;
    quit_btn->height = button_height;
    quit_btn->data.button.text = string_copy("Quit", 4);
    quit_btn->on_click = on_quit;
    quit_btn->bg_color = 0xFFF44336;  // Red
    quit_btn->hover_color = 0xFFEF5350;
    quit_btn->pressed_color = 0xFFD32F2F;
    quit_btn->fg_color = 0xFFFFFFFF;
    ui_add_child(panel, quit_btn);

    // Show as modal
    ui_show(ui, panel);
    ui->modal_active = true;
    ui->modal = panel;

    return OBJECT_VAL(panel);
}

// ============================================================================
// Settings Menu
// ============================================================================

// settings_menu(settings_list) -> UIElement
// settings_list is a list of setting definitions:
//   ["slider", "Volume", 0, 100, current_value, on_change]
//   ["checkbox", "Fullscreen", is_checked, on_change]
//   ["button", "Back", on_click]
static Value native_settings_menu(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_LIST(args[0])) {
        return menu_native_error("settings_menu() requires a list of settings");
    }

    UIManager* ui = get_ui_manager();
    if (!ui) return NONE_VAL;

    ObjList* settings = AS_LIST(args[0]);

    int win_width = get_window_width();
    int win_height = get_window_height();

    // Menu dimensions
    int menu_width = 400;
    int item_height = 45;
    int item_spacing = 10;
    int title_height = 50;
    int padding = 25;

    int menu_height = title_height + padding + (item_height + item_spacing) * settings->count + padding;

    // Center the menu
    int menu_x = (win_width - menu_width) / 2;
    int menu_y = (win_height - menu_height) / 2;

    // Create panel
    ObjUIElement* panel = ui_element_new(UI_PANEL);
    panel->x = menu_x;
    panel->y = menu_y;
    panel->width = menu_width;
    panel->height = menu_height;
    panel->bg_color = 0xE0303030;

    // Title
    ObjUIElement* title_label = ui_element_new(UI_LABEL);
    title_label->x = menu_width / 2;
    title_label->y = padding;
    title_label->data.label.text = string_copy("Settings", 8);
    title_label->data.label.align = UI_ALIGN_CENTER;
    title_label->fg_color = 0xFFFFFFFF;
    ui_add_child(panel, title_label);

    int item_y = title_height + padding;

    for (int i = 0; i < settings->count; i++) {
        Value setting = settings->items[i];
        if (!IS_LIST(setting)) continue;

        ObjList* def = AS_LIST(setting);
        if (def->count < 2 || !IS_STRING(def->items[0])) continue;

        const char* type = AS_CSTRING(def->items[0]);

        if (strcmp(type, "slider") == 0 && def->count >= 6) {
            // ["slider", "Label", min, max, value, on_change]
            ObjString* label = IS_STRING(def->items[1]) ? AS_STRING(def->items[1]) : NULL;
            double min_val = IS_NUMBER(def->items[2]) ? AS_NUMBER(def->items[2]) : 0;
            double max_val = IS_NUMBER(def->items[3]) ? AS_NUMBER(def->items[3]) : 100;
            double value = IS_NUMBER(def->items[4]) ? AS_NUMBER(def->items[4]) : 0;
            ObjClosure* on_change = IS_CLOSURE(def->items[5]) ? AS_CLOSURE(def->items[5]) : NULL;

            // Label on left
            if (label) {
                ObjUIElement* lbl = ui_element_new(UI_LABEL);
                lbl->x = padding;
                lbl->y = item_y + item_height / 4;
                lbl->data.label.text = label;
                lbl->fg_color = 0xFFFFFFFF;
                ui_add_child(panel, lbl);
            }

            // Slider on right
            ObjUIElement* slider = ui_element_new(UI_SLIDER);
            slider->x = menu_width / 2;
            slider->y = item_y;
            slider->width = menu_width / 2 - padding;
            slider->height = item_height;
            slider->data.slider.min = min_val;
            slider->data.slider.max = max_val;
            slider->data.slider.value = value;
            slider->on_change = on_change;
            ui_add_child(panel, slider);

        } else if (strcmp(type, "checkbox") == 0 && def->count >= 4) {
            // ["checkbox", "Label", checked, on_change]
            ObjString* label = IS_STRING(def->items[1]) ? AS_STRING(def->items[1]) : NULL;
            bool checked = IS_BOOL(def->items[2]) ? AS_BOOL(def->items[2]) : false;
            ObjClosure* on_change = IS_CLOSURE(def->items[3]) ? AS_CLOSURE(def->items[3]) : NULL;

            ObjUIElement* checkbox = ui_element_new(UI_CHECKBOX);
            checkbox->x = padding;
            checkbox->y = item_y;
            checkbox->data.checkbox.label = label;
            checkbox->data.checkbox.checked = checked;
            checkbox->on_change = on_change;
            checkbox->fg_color = 0xFFFFFFFF;
            ui_add_child(panel, checkbox);

        } else if (strcmp(type, "button") == 0 && def->count >= 3) {
            // ["button", "Label", on_click]
            ObjString* label = IS_STRING(def->items[1]) ? AS_STRING(def->items[1]) : NULL;
            ObjClosure* on_click = IS_CLOSURE(def->items[2]) ? AS_CLOSURE(def->items[2]) : NULL;

            ObjUIElement* button = ui_element_new(UI_BUTTON);
            button->x = padding;
            button->y = item_y;
            button->width = menu_width - 2 * padding;
            button->height = item_height;
            button->data.button.text = label;
            button->on_click = on_click;
            button->bg_color = 0xFF4A4A4A;
            button->hover_color = 0xFF5A5A5A;
            button->pressed_color = 0xFF3A3A3A;
            button->fg_color = 0xFFFFFFFF;
            ui_add_child(panel, button);
        }

        item_y += item_height + item_spacing;
    }

    ui_show(ui, panel);

    return OBJECT_VAL(panel);
}

// ============================================================================
// Dialog Box
// ============================================================================

// dialog(title, message, buttons) -> UIElement
// buttons is a list of [label, callback] pairs
// Creates a modal dialog with message and buttons
static Value native_dialog(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_STRING(args[0]) || !IS_STRING(args[1]) || !IS_LIST(args[2])) {
        return menu_native_error("dialog() requires (title, message, buttons)");
    }

    UIManager* ui = get_ui_manager();
    if (!ui) return NONE_VAL;

    ObjString* title = AS_STRING(args[0]);
    ObjString* message = AS_STRING(args[1]);
    ObjList* buttons = AS_LIST(args[2]);

    int win_width = get_window_width();
    int win_height = get_window_height();

    // Dialog dimensions
    int dialog_width = 350;
    int title_height = 40;
    int message_height = 60;
    int button_height = 40;
    int button_width = 100;
    int button_spacing = 10;
    int padding = 20;

    int buttons_row_width = buttons->count * button_width + (buttons->count - 1) * button_spacing;
    if (buttons_row_width > dialog_width - 2 * padding) {
        dialog_width = buttons_row_width + 2 * padding;
    }

    int dialog_height = title_height + message_height + button_height + 3 * padding;

    // Center the dialog
    int dialog_x = (win_width - dialog_width) / 2;
    int dialog_y = (win_height - dialog_height) / 2;

    // Create panel
    ObjUIElement* panel = ui_element_new(UI_PANEL);
    panel->x = dialog_x;
    panel->y = dialog_y;
    panel->width = dialog_width;
    panel->height = dialog_height;
    panel->bg_color = 0xFF404040;
    panel->border_width = 2;
    panel->border_color = 0xFF606060;

    // Title
    ObjUIElement* title_label = ui_element_new(UI_LABEL);
    title_label->x = dialog_width / 2;
    title_label->y = padding;
    title_label->data.label.text = title;
    title_label->data.label.align = UI_ALIGN_CENTER;
    title_label->fg_color = 0xFFFFFFFF;
    ui_add_child(panel, title_label);

    // Message
    ObjUIElement* msg_label = ui_element_new(UI_LABEL);
    msg_label->x = dialog_width / 2;
    msg_label->y = title_height + padding;
    msg_label->data.label.text = message;
    msg_label->data.label.align = UI_ALIGN_CENTER;
    msg_label->fg_color = 0xFFCCCCCC;
    ui_add_child(panel, msg_label);

    // Buttons row
    int buttons_start_x = (dialog_width - buttons_row_width) / 2;
    int button_y = title_height + message_height + padding;

    for (int i = 0; i < buttons->count; i++) {
        Value btn_def = buttons->items[i];
        if (!IS_LIST(btn_def)) continue;

        ObjList* btn_pair = AS_LIST(btn_def);
        if (btn_pair->count < 2) continue;

        ObjString* label = IS_STRING(btn_pair->items[0]) ? AS_STRING(btn_pair->items[0]) : NULL;
        ObjClosure* callback = IS_CLOSURE(btn_pair->items[1]) ? AS_CLOSURE(btn_pair->items[1]) : NULL;

        if (!label) continue;

        ObjUIElement* button = ui_element_new(UI_BUTTON);
        button->x = buttons_start_x + i * (button_width + button_spacing);
        button->y = button_y;
        button->width = button_width;
        button->height = button_height;
        button->data.button.text = label;
        button->on_click = callback;
        button->bg_color = 0xFF5A5A5A;
        button->hover_color = 0xFF6A6A6A;
        button->pressed_color = 0xFF4A4A4A;
        button->fg_color = 0xFFFFFFFF;
        ui_add_child(panel, button);
    }

    // Show as modal
    ui_show(ui, panel);
    ui->modal_active = true;
    ui->modal = panel;

    return OBJECT_VAL(panel);
}

// ============================================================================
// Message Box
// ============================================================================

// message_box(message, on_close) -> UIElement
// Simple message box with OK button
static Value native_message_box(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_STRING(args[0])) {
        return menu_native_error("message_box() requires (message, on_close)");
    }

    UIManager* ui = get_ui_manager();
    if (!ui) return NONE_VAL;

    ObjString* message = AS_STRING(args[0]);
    ObjClosure* on_close = NULL;
    if (arg_count >= 2 && IS_CLOSURE(args[1])) {
        on_close = AS_CLOSURE(args[1]);
    }

    int win_width = get_window_width();
    int win_height = get_window_height();

    // Message box dimensions
    int box_width = 300;
    int message_height = 50;
    int button_height = 40;
    int button_width = 80;
    int padding = 20;
    int box_height = message_height + button_height + 3 * padding;

    // Center the box
    int box_x = (win_width - box_width) / 2;
    int box_y = (win_height - box_height) / 2;

    // Create panel
    ObjUIElement* panel = ui_element_new(UI_PANEL);
    panel->x = box_x;
    panel->y = box_y;
    panel->width = box_width;
    panel->height = box_height;
    panel->bg_color = 0xFF404040;
    panel->border_width = 2;
    panel->border_color = 0xFF606060;

    // Message
    ObjUIElement* msg_label = ui_element_new(UI_LABEL);
    msg_label->x = box_width / 2;
    msg_label->y = padding + message_height / 4;
    msg_label->data.label.text = message;
    msg_label->data.label.align = UI_ALIGN_CENTER;
    msg_label->fg_color = 0xFFFFFFFF;
    ui_add_child(panel, msg_label);

    // OK button
    ObjUIElement* ok_btn = ui_element_new(UI_BUTTON);
    ok_btn->x = (box_width - button_width) / 2;
    ok_btn->y = message_height + 2 * padding;
    ok_btn->width = button_width;
    ok_btn->height = button_height;
    ok_btn->data.button.text = string_copy("OK", 2);
    ok_btn->on_click = on_close;
    ok_btn->bg_color = 0xFF4A90D9;  // Blue
    ok_btn->hover_color = 0xFF5AA0E9;
    ok_btn->pressed_color = 0xFF3A80C9;
    ok_btn->fg_color = 0xFFFFFFFF;
    ui_add_child(panel, ok_btn);

    // Show as modal
    ui_show(ui, panel);
    ui->modal_active = true;
    ui->modal = panel;

    return OBJECT_VAL(panel);
}

// ============================================================================
// Registration
// ============================================================================

void ui_menus_init(VM* vm) {
    define_native(vm, "main_menu", native_main_menu, 2);
    define_native(vm, "pause_menu", native_pause_menu, 2);
    define_native(vm, "settings_menu", native_settings_menu, 1);
    define_native(vm, "dialog", native_dialog, 3);
    define_native(vm, "message_box", native_message_box, -1);  // 1-2 args
}

// LCOV_EXCL_STOP
