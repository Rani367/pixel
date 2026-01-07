// UI System
// Provides retained-mode UI elements for game menus and interfaces
//
// The UI system provides a hierarchical structure of UI elements that can be
// used to build menus, dialogs, and in-game interfaces. Elements are organized
// in a tree structure with the UIManager maintaining the list of root elements.
//
// Key concepts:
// - Elements must be shown with ui_show() to become visible
// - Parent/child relationships allow grouping and relative positioning
// - Focus system enables keyboard navigation between interactive elements
// - Modal elements block input to elements behind them

#ifndef PH_UI_H
#define PH_UI_H

#include "core/common.h"
#include "vm/object.h"
#include "vm/vm.h"

// ============================================================================
// UIManager
// ============================================================================

// Maximum number of UI elements that can be shown at root level
#define UI_MAX_ELEMENTS 256

// UIManager - Central coordinator for the UI system
//
// The UIManager tracks all visible root-level UI elements and maintains state
// for focus, hover, and press interactions. One UIManager is created per Engine
// and is automatically updated and drawn during the game loop.
//
// Fields:
//   elements      - Array of root-level UI elements (shown with ui_show)
//   element_count - Number of elements currently in the root list
//   focused       - Element that receives keyboard input (NULL if none)
//   hovered       - Element currently under the mouse cursor (NULL if none)
//   pressed       - Element being clicked (mouse button held down)
//   modal_active  - True when a modal dialog is open
//   modal         - The current modal element (blocks input to other elements)
//   default_font  - Fallback font for elements without a custom font
typedef struct UIManager {
    ObjUIElement* elements[UI_MAX_ELEMENTS];
    int element_count;

    ObjUIElement* focused;
    ObjUIElement* hovered;
    ObjUIElement* pressed;

    bool modal_active;
    ObjUIElement* modal;

    ObjFont* default_font;
} UIManager;

// ============================================================================
// UIManager Lifecycle
// ============================================================================

// ui_manager_init - Initialize a UIManager to its default state
//
// Sets all pointers to NULL, element_count to 0, and modal_active to false.
// Must be called before using any other UIManager functions.
//
// Parameters:
//   ui - Pointer to the UIManager to initialize
void ui_manager_init(UIManager* ui);

// ui_manager_free - Clean up UIManager resources
//
// Currently a no-op as elements are managed by the GC, but should be called
// for forward compatibility.
//
// Parameters:
//   ui - Pointer to the UIManager to clean up
void ui_manager_free(UIManager* ui);

// ============================================================================
// UI Update and Draw
// ============================================================================

// ui_update - Process input and update UI state
//
// Handles mouse hover detection, click events, focus changes, and keyboard
// input for the focused element. Called automatically each frame by the engine
// before the user's on_update callback.
//
// Parameters:
//   ui - Pointer to the UIManager
//   vm - Virtual machine for calling element callbacks (on_click, on_change)
//   dt - Delta time since last frame (in seconds)
void ui_update(UIManager* ui, VM* vm, double dt);

// ui_draw - Render all visible UI elements
//
// Draws all root-level elements and their children in order. Called
// automatically each frame by the engine after the user's on_draw callback,
// ensuring UI appears on top of game content.
//
// Parameters:
//   ui - Pointer to the UIManager
void ui_draw(UIManager* ui);

// ============================================================================
// UI Element Management
// ============================================================================

// ui_show - Add an element to the visible root list
//
// Makes an element visible by adding it to the UIManager's root list.
// Elements must be shown to be rendered and receive input.
//
// Parameters:
//   ui      - Pointer to the UIManager
//   element - The element to show
//
// Returns:
//   true if the element was added or was already shown
//   false if the root list is full (UI_MAX_ELEMENTS reached)
bool ui_show(UIManager* ui, ObjUIElement* element);

// ui_hide - Remove an element from the visible root list
//
// Hides an element by removing it from the UIManager's root list.
// The element is not destroyed and can be shown again later.
//
// Parameters:
//   ui      - Pointer to the UIManager
//   element - The element to hide
//
// Returns:
//   true if the element was found and removed
//   false if the element was not in the root list
bool ui_hide(UIManager* ui, ObjUIElement* element);

// ui_clear - Remove all elements from the root list
//
// Hides all visible elements. Useful when changing scenes or closing menus.
// Also clears the focused, hovered, and pressed state.
//
// Parameters:
//   ui - Pointer to the UIManager
void ui_clear(UIManager* ui);

// ui_add_child - Add a child element to a parent
//
// Creates a parent/child relationship. Child positions become relative to
// the parent's position plus padding. Children are drawn after (on top of)
// their parent.
//
// Parameters:
//   parent - The parent element
//   child  - The element to add as a child
void ui_add_child(ObjUIElement* parent, ObjUIElement* child);

// ui_remove_child - Remove a child element from a parent
//
// Breaks the parent/child relationship. The child's position will no longer
// be relative to the parent.
//
// Parameters:
//   parent - The parent element
//   child  - The element to remove
void ui_remove_child(ObjUIElement* parent, ObjUIElement* child);

// ============================================================================
// Focus Management
// ============================================================================

// ui_set_focus - Set keyboard focus to an element
//
// The focused element receives keyboard input (TAB, ENTER, arrow keys, etc.)
// Only one element can be focused at a time. The previous focused element
// loses its focused state.
//
// Parameters:
//   ui      - Pointer to the UIManager
//   element - The element to focus (or NULL to clear focus)
void ui_set_focus(UIManager* ui, ObjUIElement* element);

// ui_clear_focus - Remove focus from all elements
//
// Clears the current focus. No element will receive keyboard input until
// ui_set_focus is called or the user clicks/tabs to an element.
//
// Parameters:
//   ui - Pointer to the UIManager
void ui_clear_focus(UIManager* ui);

// ui_focus_next - Move focus to the next focusable element
//
// Cycles through visible, enabled root elements in order. Wraps from the
// last element back to the first. If no element is focused, focuses the first.
//
// Parameters:
//   ui - Pointer to the UIManager
void ui_focus_next(UIManager* ui);

// ui_focus_prev - Move focus to the previous focusable element
//
// Cycles through visible, enabled root elements in reverse order. Wraps from
// the first element to the last. If no element is focused, focuses the last.
//
// Parameters:
//   ui - Pointer to the UIManager
void ui_focus_prev(UIManager* ui);

// ============================================================================
// Hit Testing
// ============================================================================

// ui_hit_test - Find the element at a screen position
//
// Searches through all visible elements (including children) to find the
// topmost element at the given coordinates. Modal elements take priority.
//
// Parameters:
//   ui - Pointer to the UIManager
//   x  - Screen X coordinate
//   y  - Screen Y coordinate
//
// Returns:
//   The topmost element at (x,y) or NULL if no element is at that position
ObjUIElement* ui_hit_test(UIManager* ui, double x, double y);

// ui_point_in_element - Check if a point is inside an element
//
// Tests if the given coordinates are within the element's bounding rectangle.
// Uses absolute coordinates (accounting for parent positions).
//
// Parameters:
//   element - The element to test
//   x       - Screen X coordinate
//   y       - Screen Y coordinate
//
// Returns:
//   true if (x,y) is inside the element's bounds
bool ui_point_in_element(ObjUIElement* element, double x, double y);

// ============================================================================
// Element Helpers
// ============================================================================

// ui_get_absolute_position - Calculate absolute screen position
//
// Walks up the parent chain to calculate the element's final screen position,
// adding parent positions and padding at each level.
//
// Parameters:
//   element - The element to calculate position for
//   x       - Output: absolute X position
//   y       - Output: absolute Y position
void ui_get_absolute_position(ObjUIElement* element, double* x, double* y);

// ui_get_font - Get the effective font for an element
//
// Returns the element's custom font if set, otherwise returns the UIManager's
// default font. May return NULL if no fonts are available.
//
// Parameters:
//   ui      - Pointer to the UIManager (for default font)
//   element - The element to get the font for
//
// Returns:
//   The font to use for rendering text, or NULL
ObjFont* ui_get_font(UIManager* ui, ObjUIElement* element);

// ui_get_bg_color - Get the background color for current state
//
// Returns different colors based on the element's state:
// - NORMAL: bg_color
// - HOVERED: hover_color
// - PRESSED: pressed_color
// - DISABLED: dimmed version of bg_color
// - FOCUSED: bg_color (same as normal)
//
// Parameters:
//   element - The element to get the color for
//
// Returns:
//   32-bit RGBA color value
uint32_t ui_get_bg_color(ObjUIElement* element);

// ============================================================================
// Element Drawing
// ============================================================================

// ui_draw_element - Draw a single element and its children
//
// Dispatches to the appropriate type-specific draw function based on the
// element's kind. Also recursively draws all children.
//
// Parameters:
//   ui      - Pointer to the UIManager (for font access)
//   element - The element to draw
void ui_draw_element(UIManager* ui, ObjUIElement* element);

// ui_draw_button - Draw a button element with text centered
void ui_draw_button(UIManager* ui, ObjUIElement* element);

// ui_draw_label - Draw a text label with configurable alignment
void ui_draw_label(UIManager* ui, ObjUIElement* element);

// ui_draw_panel - Draw a container panel and its children
void ui_draw_panel(UIManager* ui, ObjUIElement* element);

// ui_draw_slider - Draw a horizontal slider with fill indicator
void ui_draw_slider(UIManager* ui, ObjUIElement* element);

// ui_draw_checkbox - Draw a checkbox with label
void ui_draw_checkbox(UIManager* ui, ObjUIElement* element);

// ui_draw_text_input - Draw a text input field with cursor
void ui_draw_text_input(UIManager* ui, ObjUIElement* element);

// ui_draw_list - Draw a scrollable list of items
void ui_draw_list(UIManager* ui, ObjUIElement* element);

// ui_draw_image_box - Draw an image container with optional scaling
void ui_draw_image_box(UIManager* ui, ObjUIElement* element);

// ui_draw_progress_bar - Draw a horizontal progress bar
void ui_draw_progress_bar(UIManager* ui, ObjUIElement* element);

// ============================================================================
// Input Handling
// ============================================================================

// ui_handle_mouse - Process mouse input for an element
//
// Handles click, hover, and press states. Triggers on_click and on_change
// callbacks when appropriate.
//
// Parameters:
//   ui       - Pointer to the UIManager
//   vm       - Virtual machine for callbacks
//   element  - The element to handle input for
//   mx       - Mouse X position
//   my       - Mouse Y position
//   clicked  - True if mouse button was just pressed this frame
//   released - True if mouse button was just released this frame
//
// Returns:
//   true if the input was consumed (element handled it)
bool ui_handle_mouse(UIManager* ui, VM* vm, ObjUIElement* element,
                     double mx, double my, bool clicked, bool released);

// ui_handle_key - Process keyboard input for the focused element
//
// Handles TAB (focus navigation), ENTER (activation), ESCAPE (close modal),
// and element-specific keys (arrows for sliders, typing for text inputs).
//
// Parameters:
//   ui      - Pointer to the UIManager
//   vm      - Virtual machine for callbacks
//   key     - Key code (PAL_KEY_* constant)
//   pressed - True if key was pressed, false if released
//
// Returns:
//   true if the input was consumed
bool ui_handle_key(UIManager* ui, VM* vm, int key, bool pressed);

// ui_handle_text_input - Process text input for text input elements
//
// Appends typed characters to the focused text input element.
//
// Parameters:
//   ui   - Pointer to the UIManager
//   vm   - Virtual machine for callbacks
//   text - UTF-8 encoded text to append
void ui_handle_text_input(UIManager* ui, VM* vm, const char* text);

#endif // PH_UI_H
