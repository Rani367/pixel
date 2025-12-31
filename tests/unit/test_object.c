#include "../test_framework.h"
#include "vm/object.h"
#include "vm/gc.h"
#include "vm/value.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Setup/Teardown
// ============================================================================

static void setup(void) {
    gc_init();
    strings_init();
}

static void teardown(void) {
    strings_free();
    gc_free_all();
}

// ============================================================================
// String Interning Tests
// ============================================================================

TEST(strings_init_and_free) {
    // This is covered by setup/teardown in other tests
    gc_init();
    strings_init();
    strings_free();
    gc_free_all();
}

TEST(string_intern_new) {
    setup();

    ObjString* str = string_copy("hello", 5);
    ASSERT_NOT_NULL(str);
    ASSERT_EQ(str->length, 5);
    ASSERT_STR_EQ(str->chars, "hello");

    teardown();
}

TEST(string_intern_returns_existing) {
    setup();

    ObjString* str1 = string_copy("test", 4);
    ObjString* str2 = string_copy("test", 4);

    // Should return same pointer due to interning
    ASSERT_EQ(str1, str2);

    teardown();
}

// ============================================================================
// String Operations Tests
// ============================================================================

TEST(string_copy_basic) {
    setup();

    ObjString* str = string_copy("world", 5);
    ASSERT_NOT_NULL(str);
    ASSERT_EQ(str->length, 5);
    ASSERT_STR_EQ(str->chars, "world");

    teardown();
}

TEST(string_copy_empty) {
    setup();

    ObjString* str = string_copy("", 0);
    ASSERT_NOT_NULL(str);
    ASSERT_EQ(str->length, 0);
    ASSERT_STR_EQ(str->chars, "");

    teardown();
}

TEST(string_concat_basic) {
    setup();

    ObjString* a = string_copy("hello", 5);
    ObjString* b = string_copy("world", 5);
    ObjString* result = string_concat(a, b);

    ASSERT_NOT_NULL(result);
    ASSERT_EQ(result->length, 10);
    ASSERT_STR_EQ(result->chars, "helloworld");

    teardown();
}

TEST(string_concat_with_empty) {
    setup();

    ObjString* str = string_copy("test", 4);
    ObjString* empty = string_copy("", 0);

    ObjString* result1 = string_concat(str, empty);
    ASSERT_STR_EQ(result1->chars, "test");

    ObjString* result2 = string_concat(empty, str);
    ASSERT_STR_EQ(result2->chars, "test");

    teardown();
}

TEST(string_hash_consistency) {
    uint32_t hash1 = string_hash("test", 4);
    uint32_t hash2 = string_hash("test", 4);
    ASSERT_EQ(hash1, hash2);
}

TEST(string_hash_different_strings) {
    uint32_t hash1 = string_hash("hello", 5);
    uint32_t hash2 = string_hash("world", 5);
    ASSERT_NE(hash1, hash2);
}

// ============================================================================
// Function Objects Tests
// ============================================================================

TEST(function_new_defaults) {
    setup();

    ObjFunction* func = function_new();
    ASSERT_NOT_NULL(func);
    ASSERT_EQ(func->arity, 0);
    ASSERT_EQ(func->upvalue_count, 0);
    ASSERT_NULL(func->name);

    teardown();
}

// ============================================================================
// Closure Objects Tests
// ============================================================================

TEST(closure_new_basic) {
    setup();

    ObjFunction* func = function_new();
    ObjClosure* closure = closure_new(func);

    ASSERT_NOT_NULL(closure);
    ASSERT_EQ(closure->function, func);

    teardown();
}

TEST(closure_new_with_upvalues) {
    setup();

    ObjFunction* func = function_new();
    func->upvalue_count = 3;

    ObjClosure* closure = closure_new(func);

    ASSERT_NOT_NULL(closure);
    ASSERT_EQ(closure->upvalue_count, 3);
    ASSERT_NOT_NULL(closure->upvalues);

    // All upvalues should be NULL initially
    for (int i = 0; i < 3; i++) {
        ASSERT_NULL(closure->upvalues[i]);
    }

    teardown();
}

// ============================================================================
// Upvalue Objects Tests
// ============================================================================

TEST(upvalue_new_basic) {
    setup();

    Value slot = NUMBER_VAL(42.0);
    ObjUpvalue* upvalue = upvalue_new(&slot);

    ASSERT_NOT_NULL(upvalue);
    ASSERT_EQ(upvalue->location, &slot);
    ASSERT_NULL(upvalue->next);

    teardown();
}

// ============================================================================
// Struct Objects Tests
// ============================================================================

TEST(struct_def_new_basic) {
    setup();

    ObjString* name = string_copy("Point", 5);
    ObjStructDef* def = struct_def_new(name, 2);

    ASSERT_NOT_NULL(def);
    ASSERT_EQ(def->name, name);
    ASSERT_EQ(def->field_count, 2);
    ASSERT_NOT_NULL(def->fields);

    teardown();
}

TEST(instance_new_fields_initialized) {
    setup();

    ObjString* name = string_copy("Point", 5);
    ObjStructDef* def = struct_def_new(name, 2);
    ObjInstance* instance = instance_new(def);

    ASSERT_NOT_NULL(instance);
    ASSERT_EQ(instance->struct_def, def);
    ASSERT_NOT_NULL(instance->fields);

    // Fields should be initialized to NONE
    for (int i = 0; i < 2; i++) {
        ASSERT(IS_NONE(instance->fields[i]));
    }

    teardown();
}

// ============================================================================
// List Operations Tests
// ============================================================================

TEST(list_new_empty) {
    setup();

    ObjList* list = list_new();
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(list->count, 0);
    ASSERT_EQ(list->capacity, 0);
    ASSERT_NULL(list->items);

    teardown();
}

TEST(list_append_single) {
    setup();

    ObjList* list = list_new();
    list_append(list, NUMBER_VAL(42.0));

    ASSERT_EQ(list->count, 1);
    ASSERT_EQ(AS_NUMBER(list->items[0]), 42.0);

    teardown();
}

TEST(list_append_triggers_growth) {
    setup();

    ObjList* list = list_new();

    // Append many items to trigger growth
    for (int i = 0; i < 20; i++) {
        list_append(list, NUMBER_VAL((double)i));
    }

    ASSERT_EQ(list->count, 20);
    ASSERT_GT(list->capacity, 20);

    // Verify all values
    for (int i = 0; i < 20; i++) {
        ASSERT_EQ(AS_NUMBER(list->items[i]), (double)i);
    }

    teardown();
}

TEST(list_get_valid_index) {
    setup();

    ObjList* list = list_new();
    list_append(list, NUMBER_VAL(10.0));
    list_append(list, NUMBER_VAL(20.0));
    list_append(list, NUMBER_VAL(30.0));

    Value v0 = list_get(list, 0);
    Value v1 = list_get(list, 1);
    Value v2 = list_get(list, 2);

    ASSERT_EQ(AS_NUMBER(v0), 10.0);
    ASSERT_EQ(AS_NUMBER(v1), 20.0);
    ASSERT_EQ(AS_NUMBER(v2), 30.0);

    teardown();
}

TEST(list_get_invalid_returns_none) {
    setup();

    ObjList* list = list_new();
    list_append(list, NUMBER_VAL(10.0));

    Value v = list_get(list, 5);  // Out of bounds
    ASSERT(IS_NONE(v));

    teardown();
}

TEST(list_set_valid_index) {
    setup();

    ObjList* list = list_new();
    list_append(list, NUMBER_VAL(10.0));

    list_set(list, 0, NUMBER_VAL(99.0));

    Value v = list_get(list, 0);
    ASSERT_EQ(AS_NUMBER(v), 99.0);

    teardown();
}

TEST(list_set_invalid_noop) {
    setup();

    ObjList* list = list_new();
    list_append(list, NUMBER_VAL(10.0));

    // Setting out of bounds should be no-op
    list_set(list, 5, NUMBER_VAL(99.0));

    ASSERT_EQ(list->count, 1);

    teardown();
}

TEST(list_length_correct) {
    setup();

    ObjList* list = list_new();
    ASSERT_EQ(list_length(list), 0);

    list_append(list, NUMBER_VAL(1.0));
    ASSERT_EQ(list_length(list), 1);

    list_append(list, NUMBER_VAL(2.0));
    list_append(list, NUMBER_VAL(3.0));
    ASSERT_EQ(list_length(list), 3);

    teardown();
}

// ============================================================================
// Vec2 Operations Tests
// ============================================================================

TEST(vec2_new_basic) {
    setup();

    ObjVec2* v = vec2_new(3.0, 4.0);
    ASSERT_NOT_NULL(v);
    ASSERT_FLOAT_EQ(v->x, 3.0);
    ASSERT_FLOAT_EQ(v->y, 4.0);

    teardown();
}

TEST(vec2_add) {
    setup();

    ObjVec2* a = vec2_new(1.0, 2.0);
    ObjVec2* b = vec2_new(3.0, 4.0);
    ObjVec2* result = vec2_add(a, b);

    ASSERT_FLOAT_EQ(result->x, 4.0);
    ASSERT_FLOAT_EQ(result->y, 6.0);

    teardown();
}

TEST(vec2_sub) {
    setup();

    ObjVec2* a = vec2_new(5.0, 7.0);
    ObjVec2* b = vec2_new(2.0, 3.0);
    ObjVec2* result = vec2_sub(a, b);

    ASSERT_FLOAT_EQ(result->x, 3.0);
    ASSERT_FLOAT_EQ(result->y, 4.0);

    teardown();
}

TEST(vec2_mul) {
    setup();

    ObjVec2* a = vec2_new(2.0, 3.0);
    ObjVec2* b = vec2_new(4.0, 5.0);
    ObjVec2* result = vec2_mul(a, b);

    ASSERT_FLOAT_EQ(result->x, 8.0);
    ASSERT_FLOAT_EQ(result->y, 15.0);

    teardown();
}

TEST(vec2_scale) {
    setup();

    ObjVec2* v = vec2_new(2.0, 3.0);
    ObjVec2* result = vec2_scale(v, 2.5);

    ASSERT_FLOAT_EQ(result->x, 5.0);
    ASSERT_FLOAT_EQ(result->y, 7.5);

    teardown();
}

TEST(vec2_dot) {
    setup();

    ObjVec2* a = vec2_new(1.0, 2.0);
    ObjVec2* b = vec2_new(3.0, 4.0);
    double dot = vec2_dot(a, b);

    ASSERT_FLOAT_EQ(dot, 11.0);  // 1*3 + 2*4 = 11

    teardown();
}

TEST(vec2_length) {
    setup();

    ObjVec2* v = vec2_new(3.0, 4.0);
    double len = vec2_length(v);

    ASSERT_FLOAT_EQ(len, 5.0);  // 3-4-5 triangle

    teardown();
}

TEST(vec2_length_squared) {
    setup();

    ObjVec2* v = vec2_new(3.0, 4.0);
    double len_sq = vec2_length_squared(v);

    ASSERT_FLOAT_EQ(len_sq, 25.0);  // 9 + 16 = 25

    teardown();
}

TEST(vec2_normalize) {
    setup();

    ObjVec2* v = vec2_new(3.0, 4.0);
    ObjVec2* normalized = vec2_normalize(v);

    ASSERT_FLOAT_EQ(normalized->x, 0.6);  // 3/5
    ASSERT_FLOAT_EQ(normalized->y, 0.8);  // 4/5

    // Length should be 1
    double len = vec2_length(normalized);
    ASSERT_FLOAT_EQ(len, 1.0);

    teardown();
}

TEST(vec2_normalize_zero_vector) {
    setup();

    ObjVec2* v = vec2_new(0.0, 0.0);
    ObjVec2* normalized = vec2_normalize(v);

    // Normalizing zero vector should return zero (not crash)
    ASSERT_FLOAT_EQ(normalized->x, 0.0);
    ASSERT_FLOAT_EQ(normalized->y, 0.0);

    teardown();
}

TEST(vec2_distance) {
    setup();

    ObjVec2* a = vec2_new(0.0, 0.0);
    ObjVec2* b = vec2_new(3.0, 4.0);
    double dist = vec2_distance(a, b);

    ASSERT_FLOAT_EQ(dist, 5.0);

    teardown();
}

// ============================================================================
// Camera Objects Tests
// ============================================================================

TEST(camera_new_defaults) {
    setup();

    ObjCamera* cam = camera_new();
    ASSERT_NOT_NULL(cam);
    ASSERT_FLOAT_EQ(cam->x, 0.0);
    ASSERT_FLOAT_EQ(cam->y, 0.0);
    ASSERT_FLOAT_EQ(cam->zoom, 1.0);
    ASSERT_NULL(cam->target);

    teardown();
}

TEST(camera_update_no_target) {
    setup();

    ObjCamera* cam = camera_new();
    cam->x = 100.0;
    cam->y = 100.0;

    camera_update(cam, 0.016);  // ~60fps frame

    // Without target, position shouldn't change
    ASSERT_FLOAT_EQ(cam->x, 100.0);
    ASSERT_FLOAT_EQ(cam->y, 100.0);

    teardown();
}

TEST(camera_update_with_target) {
    setup();

    ObjCamera* cam = camera_new();
    cam->x = 0.0;
    cam->y = 0.0;
    cam->follow_lerp = 1.0;  // Instant follow

    ObjSprite* sprite = sprite_new(NULL);
    sprite->x = 100.0;
    sprite->y = 200.0;

    cam->target = sprite;

    camera_update(cam, 0.016);

    // With lerp=1.0, camera should be at target
    ASSERT_FLOAT_EQ(cam->x, 100.0);
    ASSERT_FLOAT_EQ(cam->y, 200.0);

    teardown();
}

TEST(camera_shake_decay) {
    setup();

    ObjCamera* cam = camera_new();
    cam->shake_intensity = 10.0;
    cam->shake_duration = 0.5;
    cam->shake_time = 0.0;

    // Update for 0.25 seconds (half duration)
    camera_update(cam, 0.25);

    ASSERT_FLOAT_EQ(cam->shake_time, 0.25);
    ASSERT(cam->shake_duration > 0);  // Still shaking

    // Update past duration
    camera_update(cam, 0.30);

    // Shake should have stopped (duration <= 0)
    ASSERT(cam->shake_duration <= 0);
    ASSERT_FLOAT_EQ(cam->shake_offset_x, 0.0);
    ASSERT_FLOAT_EQ(cam->shake_offset_y, 0.0);

    teardown();
}

// ============================================================================
// Animation Objects Tests
// ============================================================================

TEST(animation_new_basic) {
    setup();

    ObjAnimation* anim = animation_new(NULL, 16, 16);
    ASSERT_NOT_NULL(anim);
    ASSERT_EQ(anim->frame_width, 16);
    ASSERT_EQ(anim->frame_height, 16);
    ASSERT_EQ(anim->playing, false);
    ASSERT_EQ(anim->looping, true);
    ASSERT_NULL(anim->frames);
    ASSERT_EQ(anim->frame_count, 0);

    teardown();
}

TEST(animation_set_frames) {
    setup();

    ObjAnimation* anim = animation_new(NULL, 16, 16);

    int frames[] = {0, 1, 2, 3};
    animation_set_frames(anim, frames, 4, 0.1);

    ASSERT_EQ(anim->frame_count, 4);
    ASSERT_FLOAT_EQ(anim->frame_time, 0.1);
    ASSERT_EQ(anim->frames[0], 0);
    ASSERT_EQ(anim->frames[1], 1);
    ASSERT_EQ(anim->frames[2], 2);
    ASSERT_EQ(anim->frames[3], 3);

    teardown();
}

TEST(animation_update_loop) {
    setup();

    ObjAnimation* anim = animation_new(NULL, 16, 16);
    int frames[] = {0, 1, 2};
    animation_set_frames(anim, frames, 3, 0.1);

    anim->playing = true;
    anim->looping = true;
    anim->current_frame = 0;
    anim->current_time = 0.0;

    // Update to next frame
    bool completed = animation_update(anim, 0.15);
    ASSERT_EQ(completed, false);
    ASSERT_EQ(anim->current_frame, 1);

    // Update to third frame
    completed = animation_update(anim, 0.15);
    ASSERT_EQ(completed, false);
    ASSERT_EQ(anim->current_frame, 2);

    // Loop back to first frame
    completed = animation_update(anim, 0.15);
    ASSERT_EQ(completed, false);  // Looping doesn't complete
    ASSERT_EQ(anim->current_frame, 0);

    teardown();
}

TEST(animation_update_oneshot) {
    setup();

    ObjAnimation* anim = animation_new(NULL, 16, 16);
    int frames[] = {0, 1};
    animation_set_frames(anim, frames, 2, 0.1);

    anim->playing = true;
    anim->looping = false;  // One-shot
    anim->current_frame = 0;
    anim->current_time = 0.0;

    // First update
    bool completed = animation_update(anim, 0.15);
    ASSERT_EQ(completed, false);
    ASSERT_EQ(anim->current_frame, 1);

    // Second update completes animation
    completed = animation_update(anim, 0.15);
    ASSERT_EQ(completed, true);
    ASSERT_EQ(anim->playing, false);

    teardown();
}

TEST(animation_update_not_playing) {
    setup();

    ObjAnimation* anim = animation_new(NULL, 16, 16);
    anim->playing = false;

    bool completed = animation_update(anim, 0.5);
    ASSERT_EQ(completed, false);

    teardown();
}

// ============================================================================
// Sprite Objects Tests
// ============================================================================

TEST(sprite_new_basic) {
    setup();

    ObjSprite* sprite = sprite_new(NULL);
    ASSERT_NOT_NULL(sprite);
    ASSERT_NULL(sprite->image);
    ASSERT_FLOAT_EQ(sprite->x, 0.0);
    ASSERT_FLOAT_EQ(sprite->y, 0.0);
    ASSERT_EQ(sprite->visible, true);
    ASSERT_FLOAT_EQ(sprite->scale_x, 1.0);
    ASSERT_FLOAT_EQ(sprite->scale_y, 1.0);
    ASSERT_FLOAT_EQ(sprite->friction, 1.0);  // No friction by default
    ASSERT_FLOAT_EQ(sprite->gravity_scale, 1.0);

    teardown();
}

TEST(sprite_with_animation) {
    setup();

    ObjSprite* sprite = sprite_new(NULL);
    ObjAnimation* anim = animation_new(NULL, 32, 32);

    sprite->animation = anim;
    sprite->frame_width = 32;
    sprite->frame_height = 32;

    ASSERT_NOT_NULL(sprite->animation);
    ASSERT_EQ(sprite->frame_width, 32);

    teardown();
}

// ============================================================================
// Particle Emitter Tests
// ============================================================================

TEST(emitter_new_basic) {
    setup();

    ObjParticleEmitter* emitter = particle_emitter_new(100.0, 200.0);
    ASSERT_NOT_NULL(emitter);
    ASSERT_FLOAT_EQ(emitter->x, 100.0);
    ASSERT_FLOAT_EQ(emitter->y, 200.0);
    ASSERT_EQ(emitter->particle_count, 0);
    ASSERT_EQ(emitter->active, true);  // Active by default

    teardown();
}

TEST(emitter_emit_particles) {
    setup();

    ObjParticleEmitter* emitter = particle_emitter_new(0.0, 0.0);
    emitter->speed_min = 50.0;
    emitter->speed_max = 100.0;
    emitter->life_min = 0.5;
    emitter->life_max = 1.0;
    emitter->size_min = 2.0;
    emitter->size_max = 4.0;

    particle_emitter_emit(emitter, 5);

    ASSERT_EQ(emitter->particle_count, 5);

    // Check particles are initialized
    for (int i = 0; i < 5; i++) {
        Particle* p = &emitter->particles[i];
        ASSERT(p->life > 0);
        ASSERT(p->max_life > 0);
    }

    teardown();
}

TEST(emitter_update_with_gravity) {
    setup();

    ObjParticleEmitter* emitter = particle_emitter_new(0.0, 0.0);
    emitter->gravity = 100.0;
    emitter->life_min = 1.0;
    emitter->life_max = 1.0;

    particle_emitter_emit(emitter, 1);

    Particle* p = &emitter->particles[0];
    double initial_vy = p->vy;

    particle_emitter_update(emitter, 0.1);

    // Velocity should have increased due to gravity
    ASSERT(p->vy > initial_vy);

    teardown();
}

TEST(emitter_particle_death) {
    setup();

    ObjParticleEmitter* emitter = particle_emitter_new(0.0, 0.0);
    emitter->life_min = 0.1;
    emitter->life_max = 0.1;

    particle_emitter_emit(emitter, 3);
    ASSERT_EQ(emitter->particle_count, 3);

    // Update past lifetime
    particle_emitter_update(emitter, 0.2);

    // All particles should be dead
    ASSERT_EQ(emitter->particle_count, 0);

    teardown();
}

TEST(emitter_max_particles) {
    setup();

    ObjParticleEmitter* emitter = particle_emitter_new(0.0, 0.0);
    emitter->life_min = 10.0;
    emitter->life_max = 10.0;

    // Try to emit more than max
    particle_emitter_emit(emitter, 300);  // PARTICLE_MAX is 256

    // Should cap at PARTICLE_MAX
    ASSERT_EQ(emitter->particle_count, PARTICLE_MAX);

    teardown();
}

TEST(emitter_rate_based_emission) {
    setup();

    ObjParticleEmitter* emitter = particle_emitter_new(0.0, 0.0);
    emitter->rate = 100.0;  // 100 particles/second
    emitter->active = true;
    emitter->life_min = 5.0;
    emitter->life_max = 5.0;

    // Update for 0.1 seconds = 10 particles expected
    particle_emitter_update(emitter, 0.1);

    ASSERT_EQ(emitter->particle_count, 10);

    teardown();
}

// ============================================================================
// Image Objects Tests
// ============================================================================

TEST(image_new_basic) {
    setup();

    ObjString* path = string_copy("test.png", 8);
    ObjImage* image = image_new(NULL, 64, 64, path);

    ASSERT_NOT_NULL(image);
    ASSERT_EQ(image->width, 64);
    ASSERT_EQ(image->height, 64);
    ASSERT_EQ(image->path, path);

    teardown();
}

// ============================================================================
// Font Objects Tests
// ============================================================================

TEST(font_new_basic) {
    setup();

    ObjFont* font = font_new(NULL, 24, false);
    ASSERT_NOT_NULL(font);
    ASSERT_EQ(font->size, 24);
    ASSERT_EQ(font->is_default, false);

    teardown();
}

TEST(font_new_default) {
    setup();

    ObjFont* font = font_new(NULL, 16, true);
    ASSERT_NOT_NULL(font);
    ASSERT_EQ(font->size, 16);
    ASSERT_EQ(font->is_default, true);

    teardown();
}

// ============================================================================
// Sound Objects Tests
// ============================================================================

TEST(sound_new_basic) {
    setup();

    ObjString* path = string_copy("test.wav", 8);
    ObjSound* sound = sound_new(NULL, path);

    ASSERT_NOT_NULL(sound);
    ASSERT_EQ(sound->path, path);

    teardown();
}

// ============================================================================
// Music Objects Tests
// ============================================================================

TEST(music_new_basic) {
    setup();

    ObjString* path = string_copy("test.ogg", 8);
    ObjMusic* music = music_new(NULL, path);

    ASSERT_NOT_NULL(music);
    ASSERT_EQ(music->path, path);

    teardown();
}

// ============================================================================
// Native Function Objects Tests
// ============================================================================

static Value dummy_native(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;
    return NONE_VAL;
}

TEST(native_new_basic) {
    setup();

    ObjString* name = string_copy("test_fn", 7);
    ObjNative* native = native_new(dummy_native, name, 2);

    ASSERT_NOT_NULL(native);
    ASSERT_EQ(native->function, dummy_native);
    ASSERT_EQ(native->name, name);
    ASSERT_EQ(native->arity, 2);

    teardown();
}

TEST(native_variadic) {
    setup();

    ObjString* name = string_copy("varargs", 7);
    ObjNative* native = native_new(dummy_native, name, -1);

    ASSERT_EQ(native->arity, -1);  // Variadic

    teardown();
}

// ============================================================================
// Object Utility Tests
// ============================================================================

TEST(object_type_name_all_types) {
    ASSERT_STR_EQ(object_type_name(OBJ_STRING), "string");
    ASSERT_STR_EQ(object_type_name(OBJ_FUNCTION), "function");
    ASSERT_STR_EQ(object_type_name(OBJ_CLOSURE), "closure");
    ASSERT_STR_EQ(object_type_name(OBJ_UPVALUE), "upvalue");
    ASSERT_STR_EQ(object_type_name(OBJ_STRUCT_DEF), "struct");
    ASSERT_STR_EQ(object_type_name(OBJ_INSTANCE), "instance");
    ASSERT_STR_EQ(object_type_name(OBJ_LIST), "list");
    ASSERT_STR_EQ(object_type_name(OBJ_NATIVE), "native");
    ASSERT_STR_EQ(object_type_name(OBJ_VEC2), "vec2");
    ASSERT_STR_EQ(object_type_name(OBJ_IMAGE), "image");
    ASSERT_STR_EQ(object_type_name(OBJ_SPRITE), "sprite");
    ASSERT_STR_EQ(object_type_name(OBJ_FONT), "font");
    ASSERT_STR_EQ(object_type_name(OBJ_SOUND), "sound");
    ASSERT_STR_EQ(object_type_name(OBJ_MUSIC), "music");
    ASSERT_STR_EQ(object_type_name(OBJ_CAMERA), "camera");
    ASSERT_STR_EQ(object_type_name(OBJ_ANIMATION), "animation");
    ASSERT_STR_EQ(object_type_name(OBJ_PARTICLE_EMITTER), "particle_emitter");
}

TEST(object_hash_string) {
    setup();

    ObjString* str = string_copy("test", 4);
    Value val = OBJECT_VAL((Object*)str);

    uint32_t hash = object_hash(val);
    ASSERT_EQ(hash, str->hash);  // Should return cached hash

    teardown();
}

TEST(object_hash_vec2) {
    setup();

    ObjVec2* v = vec2_new(1.0, 2.0);
    Value val = OBJECT_VAL((Object*)v);

    // Just verify it doesn't crash and returns something
    uint32_t hash = object_hash(val);
    (void)hash;  // Suppress unused warning

    teardown();
}

// ============================================================================
// Value Print Tests
// ============================================================================

TEST(value_print_closure) {
    setup();
    ObjFunction* fn = function_new();
    fn->name = string_copy("test_fn", 7);
    ObjClosure* closure = closure_new(fn);

    // Print to stdout (just verify no crash)
    printf("  Testing print: ");
    value_print(OBJECT_VAL(closure));
    printf("\n");

    teardown();
}

TEST(value_print_closure_anonymous) {
    setup();
    ObjFunction* fn = function_new();
    fn->name = NULL;  // Anonymous function
    ObjClosure* closure = closure_new(fn);

    printf("  Testing print: ");
    value_print(OBJECT_VAL(closure));
    printf("\n");

    teardown();
}

TEST(value_print_upvalue) {
    setup();
    ObjUpvalue* uv = upvalue_new(NULL);
    uv->closed = NUMBER_VAL(42);

    printf("  Testing print: ");
    value_print(OBJECT_VAL(uv));
    printf("\n");

    teardown();
}

TEST(value_print_struct_def) {
    setup();
    ObjString* name = string_copy("Point", 5);
    ObjStructDef* def = struct_def_new(name, 2);

    printf("  Testing print: ");
    value_print(OBJECT_VAL(def));
    printf("\n");

    teardown();
}

TEST(value_print_instance) {
    setup();
    ObjString* name = string_copy("Point", 5);
    ObjStructDef* def = struct_def_new(name, 2);
    ObjInstance* inst = instance_new(def);

    printf("  Testing print: ");
    value_print(OBJECT_VAL(inst));
    printf("\n");

    teardown();
}

TEST(value_print_list) {
    setup();
    ObjList* list = list_new();
    list_append(list, NUMBER_VAL(1));
    list_append(list, NUMBER_VAL(2));
    list_append(list, NUMBER_VAL(3));

    printf("  Testing print: ");
    value_print(OBJECT_VAL(list));
    printf("\n");

    teardown();
}

TEST(value_print_native) {
    setup();
    ObjString* name = string_copy("native_test", 11);
    ObjNative* native = native_new(NULL, name, 0);

    printf("  Testing print: ");
    value_print(OBJECT_VAL(native));
    printf("\n");

    teardown();
}

TEST(value_print_native_anonymous) {
    setup();
    ObjNative* native = native_new(NULL, NULL, 0);

    printf("  Testing print: ");
    value_print(OBJECT_VAL(native));
    printf("\n");

    teardown();
}

TEST(value_print_vec2) {
    setup();
    ObjVec2* v = vec2_new(1.5, 2.5);

    printf("  Testing print: ");
    value_print(OBJECT_VAL(v));
    printf("\n");

    teardown();
}

TEST(value_print_image) {
    setup();
    ObjString* path = string_copy("/test/image.png", 15);
    ObjImage* img = image_new(NULL, 100, 100, path);

    printf("  Testing print: ");
    value_print(OBJECT_VAL(img));
    printf("\n");

    teardown();
}

TEST(value_print_image_no_path) {
    setup();
    ObjImage* img = image_new(NULL, 64, 64, NULL);

    printf("  Testing print: ");
    value_print(OBJECT_VAL(img));
    printf("\n");

    teardown();
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("String Interning");
    RUN_TEST(strings_init_and_free);
    RUN_TEST(string_intern_new);
    RUN_TEST(string_intern_returns_existing);

    TEST_SUITE("String Operations");
    RUN_TEST(string_copy_basic);
    RUN_TEST(string_copy_empty);
    RUN_TEST(string_concat_basic);
    RUN_TEST(string_concat_with_empty);
    RUN_TEST(string_hash_consistency);
    RUN_TEST(string_hash_different_strings);

    TEST_SUITE("Function Objects");
    RUN_TEST(function_new_defaults);

    TEST_SUITE("Closure Objects");
    RUN_TEST(closure_new_basic);
    RUN_TEST(closure_new_with_upvalues);

    TEST_SUITE("Upvalue Objects");
    RUN_TEST(upvalue_new_basic);

    TEST_SUITE("Struct Objects");
    RUN_TEST(struct_def_new_basic);
    RUN_TEST(instance_new_fields_initialized);

    TEST_SUITE("List Operations");
    RUN_TEST(list_new_empty);
    RUN_TEST(list_append_single);
    RUN_TEST(list_append_triggers_growth);
    RUN_TEST(list_get_valid_index);
    RUN_TEST(list_get_invalid_returns_none);
    RUN_TEST(list_set_valid_index);
    RUN_TEST(list_set_invalid_noop);
    RUN_TEST(list_length_correct);

    TEST_SUITE("Vec2 Operations");
    RUN_TEST(vec2_new_basic);
    RUN_TEST(vec2_add);
    RUN_TEST(vec2_sub);
    RUN_TEST(vec2_mul);
    RUN_TEST(vec2_scale);
    RUN_TEST(vec2_dot);
    RUN_TEST(vec2_length);
    RUN_TEST(vec2_length_squared);
    RUN_TEST(vec2_normalize);
    RUN_TEST(vec2_normalize_zero_vector);
    RUN_TEST(vec2_distance);

    TEST_SUITE("Camera Objects");
    RUN_TEST(camera_new_defaults);
    RUN_TEST(camera_update_no_target);
    RUN_TEST(camera_update_with_target);
    RUN_TEST(camera_shake_decay);

    TEST_SUITE("Animation Objects");
    RUN_TEST(animation_new_basic);
    RUN_TEST(animation_set_frames);
    RUN_TEST(animation_update_loop);
    RUN_TEST(animation_update_oneshot);
    RUN_TEST(animation_update_not_playing);

    TEST_SUITE("Sprite Objects");
    RUN_TEST(sprite_new_basic);
    RUN_TEST(sprite_with_animation);

    TEST_SUITE("Particle Emitter");
    RUN_TEST(emitter_new_basic);
    RUN_TEST(emitter_emit_particles);
    RUN_TEST(emitter_update_with_gravity);
    RUN_TEST(emitter_particle_death);
    RUN_TEST(emitter_max_particles);
    RUN_TEST(emitter_rate_based_emission);

    TEST_SUITE("Image Objects");
    RUN_TEST(image_new_basic);

    TEST_SUITE("Font Objects");
    RUN_TEST(font_new_basic);
    RUN_TEST(font_new_default);

    TEST_SUITE("Sound Objects");
    RUN_TEST(sound_new_basic);

    TEST_SUITE("Music Objects");
    RUN_TEST(music_new_basic);

    TEST_SUITE("Native Function Objects");
    RUN_TEST(native_new_basic);
    RUN_TEST(native_variadic);

    TEST_SUITE("Object Utilities");
    RUN_TEST(object_type_name_all_types);
    RUN_TEST(object_hash_string);
    RUN_TEST(object_hash_vec2);

    TEST_SUITE("Value Print");
    RUN_TEST(value_print_closure);
    RUN_TEST(value_print_closure_anonymous);
    RUN_TEST(value_print_upvalue);
    RUN_TEST(value_print_struct_def);
    RUN_TEST(value_print_instance);
    RUN_TEST(value_print_list);
    RUN_TEST(value_print_native);
    RUN_TEST(value_print_native_anonymous);
    RUN_TEST(value_print_vec2);
    RUN_TEST(value_print_image);
    RUN_TEST(value_print_image_no_path);

    TEST_SUMMARY();
}
