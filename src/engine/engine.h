// Game Engine Core
// Provides the game loop, callback detection, and window management

#ifndef PH_ENGINE_H
#define PH_ENGINE_H

#include "core/common.h"
#include "vm/vm.h"
#include "vm/object.h"
#include "pal/pal.h"

// Default window settings
#define ENGINE_DEFAULT_WIDTH 800
#define ENGINE_DEFAULT_HEIGHT 600
#define ENGINE_DEFAULT_TITLE "Placeholder Game"
#define ENGINE_TARGET_FPS 60

// Maximum length for scene names
#define ENGINE_MAX_SCENE_NAME 64

// Engine state
typedef struct {
    // Core references
    VM* vm;
    PalWindow* window;

    // Camera (created on first use, NULL initially)
    ObjCamera* camera;

    // User callbacks (NULL if not defined)
    ObjClosure* on_start;
    ObjClosure* on_update;  // receives: dt (delta time)
    ObjClosure* on_draw;

    // Input callbacks
    ObjClosure* on_key_down;     // receives: key (key code)
    ObjClosure* on_key_up;       // receives: key (key code)
    ObjClosure* on_mouse_click;  // receives: x, y, button
    ObjClosure* on_mouse_move;   // receives: x, y

    // Scene management
    char current_scene[ENGINE_MAX_SCENE_NAME];  // Current scene name ("" = default)
    bool scene_changed;                          // Flag for scene transitions
    char next_scene[ENGINE_MAX_SCENE_NAME];      // Scene to load next frame

    // State
    bool running;
    bool window_created;

    // Timing
    double time;        // Total elapsed time since game start
    double delta_time;  // Time since last frame
    double last_time;   // Time of previous frame
    int target_fps;

    // Input state for tracking changes
    int last_mouse_x;
    int last_mouse_y;
} Engine;

// ============================================================================
// Lifecycle
// ============================================================================

// Create a new engine (does not initialize PAL yet)
Engine* engine_new(VM* vm);

// Free engine resources
void engine_free(Engine* engine);

// Initialize PAL backend
bool engine_init(Engine* engine, PalBackend backend);

// Shutdown the engine
void engine_shutdown(Engine* engine);

// ============================================================================
// Window Management
// ============================================================================

// Create the game window (can be called from native or auto-created)
bool engine_create_window(Engine* engi