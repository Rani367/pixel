// Pre-Built Menu Functions
// Provides ready-to-use menu implementations for common game UI patterns
//
// This module provides high-level menu functions that create complete,
// styled UI layouts with minimal code. Each function creates a panel with
// child elements and automatically shows it.
//
// Available menus:
//
// main_menu(title, options)
//   Creates a centered vertical menu with a title and button list.
//   Options is a list of [label, callback] pairs.
//
//   Example:
//     main_menu("My Game", [
//         ["Play", fn() { load_scene("game") }],
//         ["Settings", fn() { show_settings() }],
//         ["Quit", fn() { quit() }]
//     ])
//
// pause_menu(on_resume, on_quit)
//   Creates a simple pause menu with Resume and Quit buttons.
//   Displayed as a modal dialog (blocks input to other elements).
//
//   Example:
//     pause = pause_menu(
//         fn() { ui_hide(pause) },
//         fn() { load_scene("menu") }
//     )
//
// settings_menu(settings_list)
//   Creates a settings panel with various control types.
//   Each setting is a list: ["type", label, ...type_specific_args]
//
//   Supported types:
//     ["slider", "Label", min, max, value, on_change]
//     ["checkbox", "Label", checked, on_change]
//     ["button", "Label", on_click]
//
//   Example:
//     settings_menu([
//         ["slider", "Volume", 0, 100, 80, fn(v) { set_volume(v) }],
//         ["checkbox", "Fullscreen", false, fn(c) { set_fullscreen(c) }],
//         ["button", "Back", fn() { ui_hide(settings) }]
//     ])
//
// dialog(title, message, buttons)
//   Creates a modal dialog box with a title, message, and button row.
//   Buttons is a list of [label, callback] pairs.
//
//   Example:
//     dialog("Confirm", "Are you sure?", [
//         ["Yes", fn() { do_action() }],
//         ["No", fn() { ui_hide(dlg) }]
//     ])
//
// message_box(message, on_close)
//   Creates a simple modal message with an OK button.
//
//   Example:
//     message_box("Game saved!", fn() { })

#ifndef PH_UI_MENUS_H
#define PH_UI_MENUS_H

#include "vm/vm.h"

// ui_menus_init - Register all pre-built menu native functions
//
// Registers the following Pixel functions:
//   - main_menu(title, options) -> UIElement
//   - pause_menu(on_resume, on_quit) -> UIElement
//   - settings_menu(settings_list) -> UIElement
//   - dialog(title, message, buttons) -> UIElement
//   - message_box(message, on_close) -> UIElement
//
// This function is called automatically by ui_natives_init() and should
// not be called directly.
//
// Parameters:
//   vm - The virtual machine to register functions with
void ui_menus_init(VM* vm);

#endif // PH_UI_MENUS_H
