ngine->delta_time > 0.1) {
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

    // Call on_update with delta time
    if (engine->on_update) {
        Value dt = NUMBER_VAL(engine->delta_time);
        vm_call_closure(engine->vm, engine->on_update, 1, &dt);
    }

    // Call on_draw
    if (engine->on_draw) {
        vm_call_closure(engine->vm, engine->on_draw, 0, NULL);
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

    // Call on_start first - it may create a window
    if (engine->on_start) {
        vm_call_closure(engine->vm, engine->on_start, 0, NULL);
    }

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
    // Native: traditional while loop
    while (engine->running && !pal_should_quit()) {
        engine_frame_tick(engine);
    }
#endif
}
// Game Engine Core Implementation

#include "engine/engine.h"
#include "engine/physics.h"
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

    return engine;
}

void engine_free(Engine* engine) {
    if (!engine) return;

    if (engine->window) {
        pal_window_destroy(engine->window);
        engine->window = NULL;
    }

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

    if (engine->w