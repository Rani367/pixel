// Game Engine Core Implementation

#include "engine/engine.h"
#include "engine/physics.h"
#include "engine/ui.h"
#include "core/table.h"
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// ============================================================================
// Global Engine Instance
// ============================================================================

static Engine* g_engine = NULL;

Engine* engine_get(void) {
    return g_engine;
}

void engine_set(Engine* engine) {
    g_engine = engine;
}

// ============================================================================
// Lifecycle
// ============================================================================

Engine* engine_new(VM* vm) {
    Engine* engine = (Engine*)malloc(sizeof(Engine));
    if (!engine) return NULL;

    engine->vm = vm;
    engine->window = NULL;
    engine->camera = NULL;

    engine->on_start = NULL;
    engine->on_update = NULL;
    engine->on_draw = NULL;

    engine->on_key_down = NULL;
    engine->on_key_up = NULL;
    engine->on_mouse_click = NULL;
    engine->on_mouse_move = NULL;

    engine->current_scene[0] = '\0';
    engine->scene_changed = false;
    engine->next_scene[0] = '\0';

    engine->running = false;
    engine->window_created = false;

    engine->time = 0.0;
    engine->delta_time = 0.0;
    engine->last_time = 0.0;
    engine->target_fps = ENGINE_TARGET_FPS;

    engine->last_mouse_x = 0;
    engine->last_mouse_y = 0;

    // Initialize UI system
    engine->ui = (UIManager*)malloc(sizeof(UIManager));
    if (engine->ui) {
        ui_manager_init(engine->ui);
    }

    return engine;
}

void engine_free(Engine* engine) {
    if (!engine) return;

    // Free UI system
    if (engine->ui) {
        ui_manager_free(engine->ui);
        free(engine->ui);
        engine->ui = NULL;
    }

    // LCOV_EXCL_START - window destruction during cleanup
    if (engine->window) {
        pal_window_destroy(engine->window);
        engine->window = NULL;
    }
    // LCOV_EXCL_STOP

    if (g_engine == engine) {
        g_engine = NULL;
    }

    free(engine);
}

bool engine_init(Engine* engine, PalBackend backend) {
    if (!engine) return false;

    // Set up texture destructor callback for image objects
    image_set_texture_destructor(pal_texture_destroy);

    // Set up font destructor callback for font objects
    font_set_destructor(pal_font_destroy);

    // Set up sound destructor callback for sound objects
    sound_set_destructor(pal_sound_destroy);

    // Set up music destructor callback for music objects
    music_set_destructor(pal_music_destroy);

    return pal_init(backend);
}

void engine_shutdown(Engine* engine) {
    if (!engine) return;

    if (engine->window) {
        pal_window_destroy(engine->window);
        engine->window = NULL;
    }

    pal_quit();
}

// ============================================================================
// Window Management
// ============================================================================

bool engine_create_window(Engine* engine, const char* title, int width, int height) {
    if (!engine) return false;

    // Destroy existing window if any
    if (engine->window) {
        pal_window_destroy(engine->window);
    }

    engine->window = pal_window_create(title, width, height);
    engine->window_created = (engine->window != NULL);
    return engine->window_created;
}

void engine_set_title(Engine* engine, const char* title) {
    if (!engine || !engine->window) return;
    pal_window_set_title(engine->window, title);
}

int engine_get_width(Engine* engine) {
    if (!engine || !engine->window) return 0;
    int width = 0, height = 0;
    pal_window_get_size(engine->window, &width, &height);
    return width;
}

int engine_get_height(Engine* engine) {
    if (!engine || !engine->window) return 0;
    int width = 0, height = 0;
    pal_window_get_size(engine->window, &width, &height);
    return height;
}

// ============================================================================
// Callback Detection
// ============================================================================

// Helper to look up a global and check if it's a closure
// LCOV_EXCL_START - callback lookup requires compiled game code
static ObjClosure* lookup_callback(VM* vm, const char* name) {
    void* val_ptr;
    if (table_get_cstr(&vm->globals, name, &val_ptr)) {
        Value val = *(Value*)val_ptr;
        if (IS_CLOSURE(val)) {
            return AS_CLOSURE(val);
        }
    }
    return NULL;
}
// LCOV_EXCL_STOP

// Helper to look up a scene-prefixed callback (e.g., "menu_on_start")
static ObjClosure* lookup_scene_callback(VM* vm, const char* scene, const char* callback) {
    if (scene[0] == '\0') {
        // Default scene: use standard callback names
        return lookup_callback(vm, callback);
    }

    // Build scene-prefixed name: "scene_callback"
    char name[128];
    snprintf(name, sizeof(name), "%s_%s", scene, callback);
    return lookup_callback(vm, name);
}

static void engine_detect_scene_callbacks(Engine* engine, const char* scene) {
    if (!engine || !engine->vm) return;

    engine->on_start = lookup_scene_callback(engine->vm, scene, "on_start");
    engine->on_update = lookup_scene_callback(engine->vm, scene, "on_update");
    engine->on_draw = lookup_scene_callback(engine->vm, scene, "on_draw");

    // Input callbacks
    engine->on_key_down = lookup_scene_callback(engine->vm, scene, "on_key_down");
    engine->on_key_up = lookup_scene_callback(engine->vm, scene, "on_key_up");
    engine->on_mouse_click = lookup_scene_callback(engine->vm, scene, "on_mouse_click");
    engine->on_mouse_move = lookup_scene_callback(engine->vm, scene, "on_mouse_move");
}

void engine_detect_callbacks(Engine* engine) {
    engine_detect_scene_callbacks(engine, engine->current_scene);
}

bool engine_has_callbacks(Engine* engine) {
    if (!engine) return false;
    return engine->on_start != NULL ||
           engine->on_update != NULL ||
           engine->on_draw != NULL ||
           engine->on_key_down != NULL ||
           engine->on_key_up != NULL ||
           engine->on_mouse_click != NULL ||
           engine->on_mouse_move != NULL;
}

// ============================================================================
// Scene Management
// ============================================================================

void engine_load_scene(Engine* engine, const char* scene_name) {
    if (!engine) return;

    // Copy scene name to next_scene buffer
    if (scene_name) {
        strncpy(engine->next_scene, scene_name, ENGINE_MAX_SCENE_NAME - 1);
        engine->next_scene[ENGINE_MAX_SCENE_NAME - 1] = '\0';
    } else {
        engine->next_scene[0] = '\0';
    }
    engine->scene_changed = true;
}

const char* engine_get_scene(Engine* engine) {
    if (!engine) return "";
    return engine->current_scene;
}

// Handle scene transition at the start of a frame
static void engine_handle_scene_transition(Engine* engine) {
    if (!engine->scene_changed) return;

    // Copy next scene to current
    strncpy(engine->current_scene, engine->next_scene, ENGINE_MAX_SCENE_NAME);
    engine->scene_changed = false;

    // Clear UI elements on scene change
    if (engine->ui) {
        ui_clear(engine->ui);
    }

    // Detect callbacks for the new scene
    engine_detect_scene_callbacks(engine, engine->current_scene);

    // LCOV_EXCL_START - scene on_start requires game code callbacks
    // Call the new scene's on_start
    if (engine->on_start) {
        vm_call_closure(engine->vm, engine->on_start, 0, NULL);
    }
    // LCOV_EXCL_STOP
}

// ============================================================================
// Physics Update
// ============================================================================

#ifndef __EMSCRIPTEN__
// Calculate frame position from frame index
static void calculate_frame_position(ObjAnimation* anim, int frame_index, int* frame_x, int* frame_y) {
    if (!anim || !anim->image || anim->frame_width <= 0) {
        *frame_x = 0;
        *frame_y = 0;
        return;
    }

    int frames_per_row = anim->image->width / anim->frame_width;
    if (frames_per_row <= 0) frames_per_row = 1;

    int row = frame_index / frames_per_row;
    int col = frame_index % frames_per_row;

    *frame_x = col * anim->frame_width;
    *frame_y = row * anim->frame_height;
}

// Update animations for all sprites
static void engine_update_animations(Engine* engine, double dt) {
    if (!engine || !engine->vm) return;

    Object* object = engine->vm->objects;
    while (object != NULL) {
        if (object->type == OBJ_SPRITE) {
            ObjSprite* sprite = (ObjSprite*)object;
            if (sprite->animation && sprite->animation->playing) {
                ObjAnimation* anim = sprite->animation;
                bool completed = animation_update(anim, dt);

                // Apply current frame to sprite
                if (anim->frame_count > 0 && anim->current_frame < anim->frame_count) {
                    int frame_index = anim->frames[anim->current_frame];
                    calculate_frame_position(anim, frame_index,
                                           &sprite->frame_x, &sprite->frame_y);
                }

                // LCOV_EXCL_START - animation callback requires game code
                // Call on_complete callback if animation finished
                if (completed && anim->on_complete) {
                    vm_call_closure(engine->vm, anim->on_complete, 0, NULL);
                }
                // LCOV_EXCL_STOP
            }
        }
        object = object->next;
    }
}

// Update physics for all sprites in the VM
static void engine_update_physics(Engine* engine, double dt) {
    if (!engine || !engine->vm) return;

    // Iterate through all objects and update sprite physics
    Object* object = engine->vm->objects;
    while (object != NULL) {
        if (object->type == OBJ_SPRITE) {
            ObjSprite* sprite = (ObjSprite*)object;
            physics_update_sprite(sprite, dt);
        }
        object = object->next;
    }
}

// Update all particle emitters
static void engine_update_particles(Engine* engine, double dt) {
    if (!engine || !engine->vm) return;

    Object* object = engine->vm->objects;
    while (object != NULL) {
        if (object->type == OBJ_PARTICLE_EMITTER) {
            ObjParticleEmitter* emitter = (ObjParticleEmitter*)object;
            particle_emitter_update(emitter, dt);
        }
        object = object->next;
    }
}
#endif

// ============================================================================
// Game Loop
// ============================================================================

void engine_stop(Engine* engine) {
    if (engine) {
        engine->running = false;
    }
}

#ifndef __EMSCRIPTEN__
// LCOV_EXCL_START - input callbacks require actual keyboard/mouse input
// Helper to fire input callbacks after polling events
static void engine_fire_input_callbacks(Engine* engine) {
    // Fire keyboard callbacks
    if (engine->on_key_down || engine->on_key_up) {
        for (int key = 0; key < PAL_KEY_COUNT; key++) {
            if (engine->on_key_down && pal_key_pressed((PalKey)key)) {
                Value args[1] = { NUMBER_VAL((double)key) };
                vm_call_closure(engine->vm, engine->on_key_down, 1, args);
            }
            if (engine->on_key_up && pal_key_released((PalKey)key)) {
                Value args[1] = { NUMBER_VAL((double)key) };
                vm_call_closure(engine->vm, engine->on_key_up, 1, args);
            }
        }
    }

    // Get current mouse position
    int mouse_x = 0, mouse_y = 0;
    pal_mouse_position(&mouse_x, &mouse_y);

    // Fire mouse click callback
    if (engine->on_mouse_click) {
        for (int button = PAL_MOUSE_LEFT; button <= PAL_MOUSE_RIGHT; button++) {
            if (pal_mouse_pressed((PalMouseButton)button)) {
                Value args[3] = {
                    NUMBER_VAL((double)mouse_x),
                    NUMBER_VAL((double)mouse_y),
                    NUMBER_VAL((double)button)
                };
                vm_call_closure(engine->vm, engine->on_mouse_click, 3, args);
            }
        }
    }

    // Fire mouse move callback if position changed
    if (engine->on_mouse_move) {
        if (mouse_x != engine->last_mouse_x || mouse_y != engine->last_mouse_y) {
            Value args[2] = {
                NUMBER_VAL((double)mouse_x),
                NUMBER_VAL((double)mouse_y)
            };
            vm_call_closure(engine->vm, engine->on_mouse_move, 2, args);
        }
    }

    // Update last mouse position
    engine->last_mouse_x = mouse_x;
    engine->last_mouse_y = mouse_y;
}
// LCOV_EXCL_STOP
#endif

// Single frame tick - called every frame by the game loop
static void engine_frame_tick(Engine* engine) {
    if (!engine || !engine->running) return;

    // Check for quit
    if (pal_should_quit()) {
        engine->running = false;
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
        return;
    }

    double frame_start = pal_time();
#ifndef __EMSCRIPTEN__
    double target_frame_time = 1.0 / engine->target_fps;
#endif

    // Calculate delta time
    engine->delta_time = frame_start - engine->last_time;
    engine->last_time = frame_start;

    // Cap delta time to avoid huge jumps (especially on first frame)
    if (engine->delta_time > 0.1) {
        engine->delta_time = 0.016667; // ~60fps
    }
    if (engine->delta_time < 0.0) {
        engine->delta_time = 0.016667;
    }

    engine->time += engine->delta_time;

    // Handle scene transitions at the start of the frame
    engine_handle_scene_transition(engine);

    // Poll input events
    pal_poll_events();

#ifndef __EMSCRIPTEN__
    // Fire input callbacks (disabled for WASM - needs investigation)
    engine_fire_input_callbacks(engine);
#endif

    // Update camera (follow target, shake, etc.)
    if (engine->camera) {
        camera_update(engine->camera, engine->delta_time);
    }

#ifndef __EMSCRIPTEN__
    // Update animations for all sprites
    engine_update_animations(engine, engine->delta_time);

    // Update physics for all sprites
    engine_update_physics(engine, engine->delta_time);

    // Update particle emitters
    engine_update_particles(engine, engine->delta_time);
#endif

    // Update UI system (before user callbacks so UI can consume input)
    if (engine->ui) {
        ui_update(engine->ui, engine->vm, engine->delta_time);
    }

    // LCOV_EXCL_START - game callbacks require compiled game code
    // Call on_update with delta time
    if (engine->on_update) {
        Value dt = NUMBER_VAL(engine->delta_time);
        vm_call_closure(engine->vm, engine->on_update, 1, &dt);
    }

    // Call on_draw
    if (engine->on_draw) {
        vm_call_closure(engine->vm, engine->on_draw, 0, NULL);
    }
    // LCOV_EXCL_STOP

    // Draw UI (after user draw callback for overlay behavior)
    if (engine->ui) {
        ui_draw(engine->ui);
    }

    // Present frame
    if (engine->window) {
        pal_window_present(engine->window);
    }

#ifndef __EMSCRIPTEN__
    // Frame rate limiting (native only - Emscripten handles this via requestAnimationFrame)
    double frame_time = pal_time() - frame_start;
    if (frame_time < target_frame_time) {
        pal_sleep(target_frame_time - frame_time);
    }
#endif
}

#ifdef __EMSCRIPTEN__
// Emscripten main loop callback wrapper
static void engine_emscripten_loop(void* arg) {
    Engine* engine = (Engine*)arg;
    engine_frame_tick(engine);
}
#endif

void engine_run(Engine* engine) {
    if (!engine || !engine->vm) return;

    // LCOV_EXCL_START - game callbacks require compiled game code
    // Call on_start first - it may create a window
    if (engine->on_start) {
        vm_call_closure(engine->vm, engine->on_start, 0, NULL);
    }
    // LCOV_EXCL_STOP

    // Auto-create window if not created by on_start
    // For Emscripten, we want to avoid destroying and recreating windows
    // as this can cause canvas attachment issues
    if (!engine->window_created) {
        engine_create_window(engine, ENGINE_DEFAULT_TITLE,
                            ENGINE_DEFAULT_WIDTH, ENGINE_DEFAULT_HEIGHT);
    }

    engine->running = true;
    engine->time = 0.0;
    engine->last_time = pal_time();

    // Initialize last mouse position
    pal_mouse_position(&engine->last_mouse_x, &engine->last_mouse_y);

#ifdef __EMSCRIPTEN__
    // Use Emscripten's main loop which integrates with the browser's requestAnimationFrame
    // 0 = use requestAnimationFrame (vsync), 1 = simulate infinite loop
    emscripten_set_main_loop_arg(engine_emscripten_loop, engine, 0, 1);
#else
    // LCOV_EXCL_START - main game loop runs in production, not unit tests
    // Native: traditional while loop
    while (engine->running && !pal_should_quit()) {
        engine_frame_tick(engine);
    }
    // LCOV_EXCL_STOP
#endif
}

// ============================================================================
// Test Wrappers (expose internal functions for unit testing)
// ============================================================================

#ifndef __EMSCRIPTEN__
void engine_frame_tick_test(Engine* engine) {
    engine_frame_tick(engine);
}

void engine_fire_input_callbacks_test(Engine* engine) {
    if (!engine) return;
    engine_fire_input_callbacks(engine);
}

void engine_update_animations_test(Engine* engine, double dt) {
    engine_update_animations(engine, dt);
}

void engine_update_physics_test(Engine* engine, double dt) {
    engine_update_physics(engine, dt);
}

void engine_update_particles_test(Engine* engine, double dt) {
    engine_update_particles(engine, dt);
}

void calculate_frame_position_test(ObjAnimation* anim, int frame_index, int* frame_x, int* frame_y) {
    calculate_frame_position(anim, frame_index, frame_x, frame_y);
}
#endif
