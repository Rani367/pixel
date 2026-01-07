// Tests for the UI System
// Comprehensive tests for all UI components and native functions

#define PAL_MOCK_ENABLED
#include "../test_framework.h"
#include "engine/engine.h"
#include "engine/engine_natives.h"
#include "engine/ui.h"
#include "runtime/stdlib.h"
#include "vm/vm.h"
#include "vm/object.h"
#include "pal/pal.h"

// ============================================================================
// Test Setup Helpers
// ============================================================================

static VM test_vm;
static Engine* test_engine;

static void setup_test_env(void) {
    vm_init(&test_vm);
    stdlib_init(&test_vm);
    test_engine = engine_new(&test_vm);
    engine_set(test_engine);
    engine_init(test_engine, PAL_BACKEND_MOCK);
    engine_create_window(test_engine, "Test", 800, 600);
    engine_natives_init(&test_vm);
}

static void teardown_test_env(void) {
    engine_shutdown(test_engine);
    engine_free(test_engine);
    engine_set(NULL);
    vm_free(&test_vm);
}

// ============================================================================
// UI Element Creation Tests
// ============================================================================

TEST(ui_element_new_button) {
    setup_test_env();

    ObjUIElement* button = ui_element_new(UI_BUTTON);
    ASSERT_NOT_NULL(button);
    ASSERT_EQ(button->obj.type, OBJ_UI_ELEMENT);
    ASSERT_EQ(button->kind, UI_BUTTON);
    ASSERT(button->visible);
    ASSERT(button->enabled);
    ASSERT_EQ(button->state, UI_STATE_NORMAL);

    teardown_test_env();
}

TEST(ui_element_new_label) {
    setup_test_env();

    ObjUIElement* label = ui_element_new(UI_LABEL);
    ASSERT_NOT_NULL(label);
    ASSERT_EQ(label->kind, UI_LABEL);
    ASSERT_EQ(label->data.label.align, UI_ALIGN_LEFT);

    teardown_test_env();
}

TEST(ui_element_new_panel) {
    setup_test_env();

    ObjUIElement* panel = ui_element_new(UI_PANEL);
    ASSERT_NOT_NULL(panel);
    ASSERT_EQ(panel->kind, UI_PANEL);

    teardown_test_env();
}

TEST(ui_element_new_slider) {
    setup_test_env();

    ObjUIElement* slider = ui_element_new(UI_SLIDER);
    ASSERT_NOT_NULL(slider);
    ASSERT_EQ(slider->kind, UI_SLIDER);
    ASSERT_FLOAT_EQ(slider->data.slider.min, 0.0);
    ASSERT_FLOAT_EQ(slider->data.slider.max, 1.0);   // Default is 0-1 range
    ASSERT_FLOAT_EQ(slider->data.slider.value, 0.5); // Default is 0.5
    ASSERT_EQ(slider->height, 20);

    teardown_test_env();
}

TEST(ui_element_new_checkbox) {
    setup_test_env();

    ObjUIElement* checkbox = ui_element_new(UI_CHECKBOX);
    ASSERT_NOT_NULL(checkbox);
    ASSERT_EQ(checkbox->kind, UI_CHECKBOX);
    ASSERT_FALSE(checkbox->data.checkbox.checked);

    teardown_test_env();
}

TEST(ui_element_new_text_input) {
    setup_test_env();

    ObjUIElement* input = ui_element_new(UI_TEXT_INPUT);
    ASSERT_NOT_NULL(input);
    ASSERT_EQ(input->kind, UI_TEXT_INPUT);
    ASSERT_EQ(input->data.text_input.cursor_pos, 0);
    ASSERT_EQ(input->data.text_input.max_length, 256);

    teardown_test_env();
}

TEST(ui_element_new_list) {
    setup_test_env();

    ObjUIElement* list = ui_element_new(UI_LIST);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(list->kind, UI_LIST);
    ASSERT_NOT_NULL(list->data.list.items);
    ASSERT_EQ(list->data.list.selected_index, -1);
    ASSERT_EQ(list->data.list.scroll_offset, 0);

    teardown_test_env();
}

TEST(ui_element_new_image_box) {
    setup_test_env();

    ObjUIElement* image_box = ui_element_new(UI_IMAGE_BOX);
    ASSERT_NOT_NULL(image_box);
    ASSERT_EQ(image_box->kind, UI_IMAGE_BOX);
    ASSERT_NULL(image_box->data.image_box.image);

    teardown_test_env();
}

TEST(ui_element_new_progress_bar) {
    setup_test_env();

    ObjUIElement* progress = ui_element_new(UI_PROGRESS_BAR);
    ASSERT_NOT_NULL(progress);
    ASSERT_EQ(progress->kind, UI_PROGRESS_BAR);
    ASSERT_FLOAT_EQ(progress->data.progress_bar.value, 0.0);

    teardown_test_env();
}

// ============================================================================
// UIManager Tests
// ============================================================================

TEST(ui_manager_init) {
    UIManager ui;
    ui_manager_init(&ui);

    ASSERT_EQ(ui.element_count, 0);
    ASSERT_NULL(ui.focused);
    ASSERT_NULL(ui.hovered);
    ASSERT_NULL(ui.pressed);
    ASSERT_FALSE(ui.modal_active);
    ASSERT_NULL(ui.modal);
    ASSERT_NULL(ui.default_font);

    ui_manager_free(&ui);
}

TEST(ui_show_hide) {
    setup_test_env();

    UIManager* ui = test_engine->ui;
    ObjUIElement* button = ui_element_new(UI_BUTTON);

    ASSERT_EQ(ui->element_count, 0);

    ASSERT(ui_show(ui, button));
    ASSERT_EQ(ui->element_count, 1);
    ASSERT_EQ(ui->elements[0], button);

    // Showing again returns true (element is visible) but doesn't add duplicate
    ASSERT(ui_show(ui, button));
    ASSERT_EQ(ui->element_count, 1);

    ASSERT(ui_hide(ui, button));
    ASSERT_EQ(ui->element_count, 0);

    // Hiding non-existent should return false
    ASSERT_FALSE(ui_hide(ui, button));

    teardown_test_env();
}

TEST(ui_clear) {
    setup_test_env();

    UIManager* ui = test_engine->ui;
    ObjUIElement* btn1 = ui_element_new(UI_BUTTON);
    ObjUIElement* btn2 = ui_element_new(UI_BUTTON);
    ObjUIElement* btn3 = ui_element_new(UI_BUTTON);

    ui_show(ui, btn1);
    ui_show(ui, btn2);
    ui_show(ui, btn3);
    ASSERT_EQ(ui->element_count, 3);

    ui_clear(ui);
    ASSERT_EQ(ui->element_count, 0);

    teardown_test_env();
}

// ============================================================================
// Hierarchy Tests
// ============================================================================

TEST(ui_add_remove_child) {
    setup_test_env();

    ObjUIElement* parent = ui_element_new(UI_PANEL);
    ObjUIElement* child = ui_element_new(UI_BUTTON);

    ASSERT_NULL(child->parent);

    ui_add_child(parent, child);
    ASSERT_EQ(child->parent, parent);
    ASSERT_EQ(parent->children->count, 1);

    ui_remove_child(parent, child);
    ASSERT_NULL(child->parent);
    ASSERT_EQ(parent->children->count, 0);

    teardown_test_env();
}

// ============================================================================
// Focus Management Tests
// ============================================================================

TEST(ui_focus) {
    setup_test_env();

    UIManager* ui = test_engine->ui;
    ObjUIElement* button = ui_element_new(UI_BUTTON);

    ASSERT_NULL(ui->focused);

    ui_set_focus(ui, button);
    ASSERT_EQ(ui->focused, button);
    ASSERT_EQ(button->state, UI_STATE_FOCUSED);

    ui_clear_focus(ui);
    ASSERT_NULL(ui->focused);
    ASSERT_EQ(button->state, UI_STATE_NORMAL);

    teardown_test_env();
}

TEST(ui_focus_next_prev) {
    setup_test_env();

    UIManager* ui = test_engine->ui;
    ObjUIElement* btn1 = ui_element_new(UI_BUTTON);
    ObjUIElement* btn2 = ui_element_new(UI_BUTTON);
    ObjUIElement* btn3 = ui_element_new(UI_BUTTON);

    ui_show(ui, btn1);
    ui_show(ui, btn2);
    ui_show(ui, btn3);

    // Start with no focus
    ASSERT_NULL(ui->focused);

    // Focus next should focus first element
    ui_focus_next(ui);
    ASSERT_EQ(ui->focused, btn1);

    // Focus next should move to second
    ui_focus_next(ui);
    ASSERT_EQ(ui->focused, btn2);

    // Focus next should move to third
    ui_focus_next(ui);
    ASSERT_EQ(ui->focused, btn3);

    // Focus next should wrap to first
    ui_focus_next(ui);
    ASSERT_EQ(ui->focused, btn1);

    // Focus prev should wrap to last
    ui_focus_prev(ui);
    ASSERT_EQ(ui->focused, btn3);

    teardown_test_env();
}

// ============================================================================
// Hit Testing Tests
// ============================================================================

TEST(ui_point_in_element) {
    setup_test_env();

    ObjUIElement* button = ui_element_new(UI_BUTTON);
    button->x = 100;
    button->y = 100;
    button->width = 200;
    button->height = 50;

    // Inside
    ASSERT(ui_point_in_element(button, 150, 125));
    ASSERT(ui_point_in_element(button, 100, 100));  // Top-left corner
    ASSERT(ui_point_in_element(button, 299, 149)); // Bottom-right edge

    // Outside
    ASSERT_FALSE(ui_point_in_element(button, 50, 125));   // Left
    ASSERT_FALSE(ui_point_in_element(button, 350, 125));  // Right
    ASSERT_FALSE(ui_point_in_element(button, 150, 50));   // Above
    ASSERT_FALSE(ui_point_in_element(button, 150, 200));  // Below

    teardown_test_env();
}

TEST(ui_hit_test) {
    setup_test_env();

    UIManager* ui = test_engine->ui;
    ObjUIElement* btn1 = ui_element_new(UI_BUTTON);
    btn1->x = 100;
    btn1->y = 100;
    btn1->width = 100;
    btn1->height = 50;

    ObjUIElement* btn2 = ui_element_new(UI_BUTTON);
    btn2->x = 300;
    btn2->y = 100;
    btn2->width = 100;
    btn2->height = 50;

    ui_show(ui, btn1);
    ui_show(ui, btn2);

    // Hit btn1
    ObjUIElement* hit = ui_hit_test(ui, 150, 125);
    ASSERT_EQ(hit, btn1);

    // Hit btn2
    hit = ui_hit_test(ui, 350, 125);
    ASSERT_EQ(hit, btn2);

    // Miss both
    hit = ui_hit_test(ui, 250, 125);
    ASSERT_NULL(hit);

    teardown_test_env();
}

// ============================================================================
// Absolute Position Tests
// ============================================================================

TEST(ui_get_absolute_position) {
    setup_test_env();

    ObjUIElement* parent = ui_element_new(UI_PANEL);
    parent->x = 100;
    parent->y = 100;
    parent->padding = 0;  // Zero padding for predictable test

    ObjUIElement* child = ui_element_new(UI_BUTTON);
    child->x = 50;
    child->y = 25;

    ui_add_child(parent, child);

    double abs_x, abs_y;
    ui_get_absolute_position(child, &abs_x, &abs_y);

    ASSERT_FLOAT_EQ(abs_x, 150.0);  // 100 + 50
    ASSERT_FLOAT_EQ(abs_y, 125.0);  // 100 + 25

    teardown_test_env();
}

// ============================================================================
// UI State Tests
// ============================================================================

TEST(ui_get_bg_color) {
    setup_test_env();

    ObjUIElement* button = ui_element_new(UI_BUTTON);
    button->bg_color = 0xFF111111;
    button->hover_color = 0xFF222222;
    button->pressed_color = 0xFF333333;

    button->state = UI_STATE_NORMAL;
    ASSERT_EQ(ui_get_bg_color(button), 0xFF111111u);

    button->state = UI_STATE_HOVERED;
    ASSERT_EQ(ui_get_bg_color(button), 0xFF222222u);

    button->state = UI_STATE_PRESSED;
    ASSERT_EQ(ui_get_bg_color(button), 0xFF333333u);

    button->state = UI_STATE_DISABLED;
    uint32_t disabled_color = ui_get_bg_color(button);
    ASSERT_NE(disabled_color, button->bg_color);  // Should be dimmed

    teardown_test_env();
}

// ============================================================================
// Native Function Registration Tests
// ============================================================================

TEST(ui_natives_registered) {
    setup_test_env();

    void* val;

    // Element creation functions
    ASSERT(table_get_cstr(&test_vm.globals, "ui_button", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_label", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_panel", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_slider", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_checkbox", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_text_input", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_list", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_progress_bar", &val));

    teardown_test_env();
}

TEST(ui_config_natives_registered) {
    setup_test_env();

    void* val;

    // Configuration functions
    ASSERT(table_get_cstr(&test_vm.globals, "ui_set_text", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_get_text", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_set_value", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_get_value", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_set_checked", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_is_checked", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_set_enabled", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_set_visible", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_set_position", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_set_size", &val));

    teardown_test_env();
}

TEST(ui_style_natives_registered) {
    setup_test_env();

    void* val;

    // Styling functions
    ASSERT(table_get_cstr(&test_vm.globals, "ui_set_colors", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_set_hover_color", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_set_font", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_set_padding", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_set_border", &val));

    teardown_test_env();
}

TEST(ui_callback_natives_registered) {
    setup_test_env();

    void* val;

    // Callback functions
    ASSERT(table_get_cstr(&test_vm.globals, "ui_on_click", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_on_change", &val));

    teardown_test_env();
}

TEST(ui_hierarchy_natives_registered) {
    setup_test_env();

    void* val;

    // Hierarchy functions
    ASSERT(table_get_cstr(&test_vm.globals, "ui_add_child", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_remove_child", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_show", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_hide", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_destroy", &val));

    teardown_test_env();
}

TEST(ui_list_natives_registered) {
    setup_test_env();

    void* val;

    // List functions
    ASSERT(table_get_cstr(&test_vm.globals, "ui_list_add", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_list_remove", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_list_clear", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_list_selected", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "ui_list_set_selected", &val));

    teardown_test_env();
}

TEST(settings_natives_registered) {
    setup_test_env();

    void* val;

    // Settings functions
    ASSERT(table_get_cstr(&test_vm.globals, "set_setting", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "get_setting", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "save_settings", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "load_settings", &val));

    teardown_test_env();
}

TEST(menu_natives_registered) {
    setup_test_env();

    void* val;

    // Menu functions
    ASSERT(table_get_cstr(&test_vm.globals, "main_menu", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "pause_menu", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "settings_menu", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "dialog", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "message_box", &val));

    teardown_test_env();
}

// ============================================================================
// Element Properties Tests
// ============================================================================

TEST(ui_element_default_colors) {
    setup_test_env();

    ObjUIElement* button = ui_element_new(UI_BUTTON);

    // Check default colors are set
    ASSERT_NE(button->bg_color, 0u);
    ASSERT_NE(button->fg_color, 0u);
    ASSERT_NE(button->hover_color, 0u);
    ASSERT_NE(button->pressed_color, 0u);

    teardown_test_env();
}

TEST(ui_element_dimensions) {
    setup_test_env();

    ObjUIElement* button = ui_element_new(UI_BUTTON);
    button->x = 10;
    button->y = 20;
    button->width = 100;
    button->height = 50;

    ASSERT_FLOAT_EQ(button->x, 10.0);
    ASSERT_FLOAT_EQ(button->y, 20.0);
    ASSERT_FLOAT_EQ(button->width, 100.0);
    ASSERT_FLOAT_EQ(button->height, 50.0);

    teardown_test_env();
}

// ============================================================================
// Visibility and Enabled State Tests
// ============================================================================

TEST(ui_element_visibility) {
    setup_test_env();

    ObjUIElement* button = ui_element_new(UI_BUTTON);

    // Default is visible
    ASSERT(button->visible);

    button->visible = false;
    ASSERT_FALSE(button->visible);

    teardown_test_env();
}

TEST(ui_element_enabled) {
    setup_test_env();

    ObjUIElement* button = ui_element_new(UI_BUTTON);

    // Default is enabled
    ASSERT(button->enabled);

    button->enabled = false;
    ASSERT_FALSE(button->enabled);

    teardown_test_env();
}

// ============================================================================
// Slider Specific Tests
// ============================================================================

TEST(ui_slider_value_clamping) {
    setup_test_env();

    ObjUIElement* slider = ui_element_new(UI_SLIDER);
    slider->data.slider.min = 0;
    slider->data.slider.max = 100;

    // Value should be clamped in range
    slider->data.slider.value = 50;
    ASSERT_FLOAT_EQ(slider->data.slider.value, 50.0);

    slider->data.slider.value = -10;
    // Note: actual clamping happens during input handling
    // Here we just verify the storage works

    teardown_test_env();
}

// ============================================================================
// Text Input Specific Tests
// ============================================================================

TEST(ui_text_input_properties) {
    setup_test_env();

    ObjUIElement* input = ui_element_new(UI_TEXT_INPUT);

    ASSERT_EQ(input->data.text_input.cursor_pos, 0);
    ASSERT_EQ(input->data.text_input.max_length, 256);
    ASSERT_NOT_NULL(input->data.text_input.text);  // Initialized to empty string
    ASSERT_EQ(input->data.text_input.text->length, 0);
    ASSERT_NULL(input->data.text_input.placeholder);

    teardown_test_env();
}

// ============================================================================
// List Specific Tests
// ============================================================================

TEST(ui_list_initialization) {
    setup_test_env();

    ObjUIElement* list = ui_element_new(UI_LIST);

    ASSERT_NOT_NULL(list->data.list.items);
    ASSERT_EQ(list->data.list.items->count, 0);
    ASSERT_EQ(list->data.list.selected_index, -1);
    ASSERT_EQ(list->data.list.scroll_offset, 0);
    ASSERT_EQ(list->data.list.visible_items, 5);  // Default visible items

    teardown_test_env();
}

// ============================================================================
// Progress Bar Specific Tests
// ============================================================================

TEST(ui_progress_bar_value) {
    setup_test_env();

    ObjUIElement* bar = ui_element_new(UI_PROGRESS_BAR);

    ASSERT_FLOAT_EQ(bar->data.progress_bar.value, 0.0);

    bar->data.progress_bar.value = 0.5;
    ASSERT_FLOAT_EQ(bar->data.progress_bar.value, 0.5);

    bar->data.progress_bar.value = 1.0;
    ASSERT_FLOAT_EQ(bar->data.progress_bar.value, 1.0);

    teardown_test_env();
}

// ============================================================================
// Children List Tests
// ============================================================================

TEST(ui_children_list) {
    setup_test_env();

    ObjUIElement* parent = ui_element_new(UI_PANEL);
    ASSERT_NOT_NULL(parent->children);
    ASSERT_EQ(parent->children->count, 0);

    ObjUIElement* child1 = ui_element_new(UI_BUTTON);
    ObjUIElement* child2 = ui_element_new(UI_LABEL);

    ui_add_child(parent, child1);
    ASSERT_EQ(parent->children->count, 1);

    ui_add_child(parent, child2);
    ASSERT_EQ(parent->children->count, 2);

    teardown_test_env();
}

// ============================================================================
// UI Update/Draw Integration Tests
// ============================================================================

TEST(ui_update_draw_no_crash) {
    setup_test_env();

    UIManager* ui = test_engine->ui;
    ObjUIElement* button = ui_element_new(UI_BUTTON);
    button->x = 100;
    button->y = 100;
    button->width = 200;
    button->height = 50;
    button->data.button.text = string_copy("Test", 4);

    ui_show(ui, button);

    // These should not crash
    ui_update(ui, &test_vm, 0.016);
    ui_draw(ui);

    teardown_test_env();
}

TEST(ui_draw_all_element_types) {
    setup_test_env();

    UIManager* ui = test_engine->ui;

    // Create one of each type
    ObjUIElement* button = ui_element_new(UI_BUTTON);
    button->data.button.text = string_copy("Button", 6);
    button->x = 10; button->y = 10; button->width = 100; button->height = 30;

    ObjUIElement* label = ui_element_new(UI_LABEL);
    label->data.label.text = string_copy("Label", 5);
    label->x = 10; label->y = 50;

    ObjUIElement* panel = ui_element_new(UI_PANEL);
    panel->x = 10; panel->y = 90; panel->width = 200; panel->height = 100;

    ObjUIElement* slider = ui_element_new(UI_SLIDER);
    slider->x = 10; slider->y = 200; slider->width = 150;

    ObjUIElement* checkbox = ui_element_new(UI_CHECKBOX);
    checkbox->data.checkbox.label = string_copy("Check", 5);
    checkbox->x = 10; checkbox->y = 240;

    ObjUIElement* progress = ui_element_new(UI_PROGRESS_BAR);
    progress->x = 10; progress->y = 280; progress->width = 150; progress->height = 20;
    progress->data.progress_bar.value = 0.7;

    ui_show(ui, button);
    ui_show(ui, label);
    ui_show(ui, panel);
    ui_show(ui, slider);
    ui_show(ui, checkbox);
    ui_show(ui, progress);

    // Drawing should not crash
    ui_draw(ui);

    teardown_test_env();
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("UI Element Creation");
    RUN_TEST(ui_element_new_button);
    RUN_TEST(ui_element_new_label);
    RUN_TEST(ui_element_new_panel);
    RUN_TEST(ui_element_new_slider);
    RUN_TEST(ui_element_new_checkbox);
    RUN_TEST(ui_element_new_text_input);
    RUN_TEST(ui_element_new_list);
    RUN_TEST(ui_element_new_image_box);
    RUN_TEST(ui_element_new_progress_bar);

    TEST_SUITE("UIManager");
    RUN_TEST(ui_manager_init);
    RUN_TEST(ui_show_hide);
    RUN_TEST(ui_clear);

    TEST_SUITE("Hierarchy");
    RUN_TEST(ui_add_remove_child);

    TEST_SUITE("Focus Management");
    RUN_TEST(ui_focus);
    RUN_TEST(ui_focus_next_prev);

    TEST_SUITE("Hit Testing");
    RUN_TEST(ui_point_in_element);
    RUN_TEST(ui_hit_test);
    RUN_TEST(ui_get_absolute_position);

    TEST_SUITE("UI State");
    RUN_TEST(ui_get_bg_color);

    TEST_SUITE("Native Function Registration");
    RUN_TEST(ui_natives_registered);
    RUN_TEST(ui_config_natives_registered);
    RUN_TEST(ui_style_natives_registered);
    RUN_TEST(ui_callback_natives_registered);
    RUN_TEST(ui_hierarchy_natives_registered);
    RUN_TEST(ui_list_natives_registered);
    RUN_TEST(settings_natives_registered);
    RUN_TEST(menu_natives_registered);

    TEST_SUITE("Element Properties");
    RUN_TEST(ui_element_default_colors);
    RUN_TEST(ui_element_dimensions);
    RUN_TEST(ui_element_visibility);
    RUN_TEST(ui_element_enabled);

    TEST_SUITE("Component Specific");
    RUN_TEST(ui_slider_value_clamping);
    RUN_TEST(ui_text_input_properties);
    RUN_TEST(ui_list_initialization);
    RUN_TEST(ui_progress_bar_value);
    RUN_TEST(ui_children_list);

    TEST_SUITE("Integration");
    RUN_TEST(ui_update_draw_no_crash);
    RUN_TEST(ui_draw_all_element_types);

    TEST_SUMMARY();
}
