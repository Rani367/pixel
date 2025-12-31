// Internal engine functions exposed for testing
// DO NOT include this header in production code!

#ifndef PH_ENGINE_INTERNAL_H
#define PH_ENGINE_INTERNAL_H

#include "engine/engine.h"

// Forward declare internal functions for testing
// These are normally static in engine.c

// Frame tick function - executes one frame of the game loop
void engine_frame_tick_test(Engine* engine);

// Fire input callbacks based on current input state
void engine_fire_input_callbacks_test(Engine* engine);

// Update animations for all sprites
void engine_update_animations_test(Engine* engine, double dt);

// Update physics for all sprites
void engine_update_physics_test(Engine* engine, double dt);

// Update particle emitters
void engine_update_particles_test(Engine* engine, double dt);

// Calculate frame position for animation
void calculate_frame_position_test(ObjAnimation* anim, int frame_index, int* frame_x, int* frame_y);

#endif // PH_ENGINE_INTERNAL_H
