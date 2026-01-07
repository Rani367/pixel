// UI Native Functions
// Exposes the UI system to Pixel code through native function bindings
//
// This module registers all UI-related native functions that can be called
// from Pixel code. Functions are organized into categories:
//
// Element Creation:
//   ui_button, ui_label, ui_panel, ui_slider, ui_checkbox,
//   ui_text_input, ui_list, ui_progress_bar
//
// Element Configuration:
//   ui_set_text, ui_get_text, ui_set_value, ui_get_value,
//   ui_set_checked, ui_is_checked, ui_set_enabled, ui_set_visible,
//   ui_set_position, ui_set_size
//
// Styling:
//   ui_set_colors, ui_set_hover_color, ui_set_font, ui_set_padding,
//   ui_set_border
//
// Callbacks:
//   ui_on_click, ui_on_change
//
// Hierarchy:
//   ui_add_child, ui_remove_child, ui_show, ui_hide, ui_destroy
//
// List Operations:
//   ui_list_add, ui_list_remove, ui_list_clear, ui_list_selected,
//   ui_list_set_selected
//
// Settings Persistence:
//   set_setting, get_setting, save_settings, load_settings
//
// Example usage in Pixel code:
//
//   btn = ui_button(100, 100, 200, 50, "Click Me!")
//   ui_on_click(btn, fn() { println("Clicked!") })
//   ui_show(btn)

#ifndef PH_UI_NATIVES_H
#define PH_UI_NATIVES_H

#include "vm/vm.h"

// ui_natives_init - Register all UI native functions with the VM
//
// Registers element creation functions, configuration functions, styling
// functions, callback setters, hierarchy management, list operations,
// settings persistence, and pre-built menu functions.
//
// This function is called automatically by engine_natives_init() and should
// not be called directly.
//
// Parameters:
//   vm - The virtual machine to register functions with
void ui_natives_init(VM* vm);

#endif // PH_UI_NATIVES_H
