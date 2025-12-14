    case OBJ_MUSIC: {
            ObjMusic* music = (ObjMusic*)object;
            music_destroy_handle(music);
            break;
        }
        case OBJ_CAMERA:
            // Camera has no external resources to free
            break;
        case OBJ_ANIMATION: {
            ObjAnimation* anim = (ObjAnimation*)object;
            if (anim->frames) {
                PH_FREE(anim->frames);
            }
            break;
        }
        case OBJ_PARTICLE_EMITTER:
            // Particle emitter has no external resources to free
            // (particles are stored in fixed-size array inside the object)
            break;
    }

    PH_FREE(object);
}

// ============================================================================
// String Interning - Weak Reference Support for GC
// ============================================================================

// Tombstone marker (must match table.c)
#define TOMBSTONE_KEY ((const char*)1)

void strings_remove_white(void) {
    // Remove unmarked strings from the intern table
    // This is called during GC before the sweep phase
    for (int i = 0; i < strings.capacity; i++) {
        TableEntry* entry = &strings.entries[i];
        if (entry->key != NULL && entry->key != TOMBSTONE_KEY) {
            // The value is an ObjString*
            ObjString* string = (ObjString*)entry->value;
            if (!string->obj.marked) {
                // String is unmarked (will be freed), remove from table
                entry->key = TOMBSTONE_KEY;
                entry->key_length = 0;
                entry->value = NULL;
            }
        }
    }
}
=====================================================================

ObjList* list_new(void) {
    ObjList* list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
    return list;
}

void list_append(ObjList* list, Value value) {
    if (list->count >= list->capacity) {
        int new_capacity = PH_GROW_CAPACITY(list->capacity);
        list->items = PH_REALLOC(list->items, sizeof(Value) * new_capacity);
        list->capacity = new_capacity;
    }
    list->items[list->count++] = value;
}

Value list_get(ObjList* list, int index) {
    if (index < 0 || index >= list->count) {
        return NONE_VAL;  // Or could return an error value
    }
    return list->items[index];
}

void list_set(ObjList* list, int index, Value value) {
    if (index >= 0 && index < list->count) {
        list->items[index] = value;
    }
}

int list_length(ObjList* list) {
    return list->count;
}

// ============================================================================
// Native Function Objects
// ============================================================================

ObjNative* native_new(NativeFn function, ObjString* name, int arity) {
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    native->name = name;
    native->arity = arity;
    return native;
}

// ============================================================================
// Vec2 Objects
// ============================================================================

ObjVec2* vec2_new(double x, double y) {
    ObjVec2* vec = ALLOCATE_OBJ(ObjVec2, OBJ_VEC2);
    vec->x = x;
    vec->y = y;
    return vec;
}

ObjVec2* vec2_add(ObjVec2* a, ObjVec2* b) {
    return vec2_new(a->x + b->x, a->y + b->y);
}

ObjVec2* vec2_sub(ObjVec2* a, ObjVec2* b) {
    return vec2_new(a->x - b->x, a->y - b->y);
}

ObjVec2* vec2_mul(ObjVec2* a, ObjVec2* b) {
    return vec2_new(a->x * b->x, a->y * b->y);
}

ObjVec2* vec2_scale(ObjVec2* v, double s) {
    return vec2_new(v->x * s, v->y * s);
}

double vec2_dot(ObjVec2* a, ObjVec2* b) {
    return a->x * b->x + a->y * b->y;
}

double vec2_length_squared(ObjVec2* v) {
    return v->x * v->x + v->y * v->y;
}

double vec2_length(ObjVec2* v) {
    return sqrt(vec2_length_squared(v));
}

ObjVec2* vec2_normalize(ObjVec2* v) {
    double len = vec2_length(v);
    if (len == 0) return vec2_new(0, 0);
    return vec2_new(v->x / len, v->y / len);
}

double vec2_distance(ObjVec2* a, ObjVec2* b) {
    double dx = b->x - a->x;
    double dy = b->y - a->y;
    return sqrt(dx * dx + dy * dy);
}

// ============================================================================
// Image Objects
// ============================================================================

// Callback for texture destruction (set by engine)
static TextureDestroyFn texture_destructor = NULL;

void image_set_texture_destructor(TextureDestroyFn fn) {
    texture_destructor = fn;
}

ObjImage* image_new(PalTexture* texture, int width, int height, ObjString* path) {
    ObjImage* image = ALLOCATE_OBJ(ObjImage, OBJ_IMAGE);
    image->texture = texture;
    image->width = width;
    image->height = height;
    image->path = path;
    return image;
}

void image_destroy_texture(ObjImage* image) {
    if (image && image->texture && texture_destructor) {
        texture_destructor(image->texture);
        image->texture = NULL;
    }
}

// ============================================================================
// Sprite Objects
// ============================================================================

ObjSprite* sprite_new(ObjImage* image) {
    ObjSprite* sprite = ALLOCATE_OBJ(ObjSprite, OBJ_SPRITE);
    sprite->image = image;
    sprite->x = 0;
    sprite->y = 0;
    sprite->width = 0;   // 0 means use image size
    sprite->height = 0;
    sprite->rotation = 0;
    sprite->scale_x = 1.0;
    sprite->scale_y = 1.0;
    sprite->origin_x = 0.0;  // Top-left by default
    sprite->origin_y = 0.0;
    sprite->visible = true;
    sprite->flip_x = false;
    sprite->flip_y = false;
    sprite->frame_x = 0;
    sprite->frame_y = 0;
    sprite->frame_width = 0;  // 0 means use full image
    sprite->frame_height = 0;
    // Physics properties
    sprite->velocity_x = 0;
    sprite->velocity_y = 0;
    sprite->acceleration_x = 0;
    sprite->acceleration_y = 0;
    sprite->friction = 1.0;       // No friction by default
    sprite->gravity_scale = 1.0;  // Full gravity by default
    sprite->grounded = false;
    // Animation
    sprite->animation = NULL;
    return sprite;
}

// ============================================================================
// Font Objects
// ============================================================================

// Callback for font destruction (set by engine)
static FontDestroyFn font_destructor = NULL;

void font_set_destructor(FontDestroyFn fn) {
    font_destructor = fn;
}

ObjFont* font_new(PalFont* font, int size, bool is_default) {
    ObjFont* obj = ALLOCATE_OBJ(ObjFont, OBJ_FONT);
    obj->font = font;
    obj->size = size;
    obj->is_default = is_default;
    return obj;
}

void font_destroy_handle(ObjFont* font) {
    if (font && font->font && font_destructor) {
        font_destructor(font->font);
        font->font = NULL;
    }
}

// ============================================================================
// Sound Objects
// ============================================================================

// Callback for sound destruction (set by engine)
static SoundDestroyFn sound_destructor = NULL;

void sound_set_destructor(SoundDestroyFn fn) {
    sound_destructor = fn;
}

ObjSound* sound_new(PalSound* sound, ObjString* path) {
    ObjSound* obj = ALLOCATE_OBJ(ObjSound, OBJ_SOUND);
    obj->sound = sound;
    obj->path = path;
    return obj;
}

void sound_destroy_handle(ObjSound* sound) {
    if (sound && sound->sound && sound_destructor) {
        sound_destructor(sound->sound);
        sound->sound = NULL;
    }
}

// ============================================================================
// Music Objects
// ============================================================================

// Callback for music destruction (set by engine)
static MusicDestroyFn music_destructor = NULL;

void music_set_destructor(MusicDestroyFn fn) {
    music_destructor = fn;
}

ObjMusic* music_new(PalMusic* music, ObjString* path) {
    ObjMusic* obj = ALLOCATE_OBJ(ObjMusic, OBJ_MUSIC);
    obj->music = music;
    obj->path = path;
    return obj;
}

void music_destroy_handle(ObjMusic* music) {
    if (music && music->music && music_destructor) {
        music_destructor(music->music);
        music->music = NULL;
    }
}

// ============================================================================
// Camera Objects
// ============================================================================

ObjCamera* camera_new(void) {
    ObjCamera* camera = ALLOCATE_OBJ(ObjCamera, OBJ_CAMERA);
    camera->x = 0;
    camera->y = 0;
    camera->zoom = 1.0;
    camera->rotation = 0;
    camera->target = NULL;
    camera->follow_lerp = 0.1;  // Smooth follow by default
    camera->shake_intensity = 0;
    camera->shake_duration = 0;
    camera->shake_time = 0;
    camera->shake_offset_x = 0;
    camera->shake_offset_y = 0;
    return camera;
}

// Simple pseudo-random for shake (deterministic based on time)
static double camera_random(double seed) {
    uint32_t n = (uint32_t)(seed * 1000000.0);
    n = (n << 13) ^ n;
    n = n * (n * n * 15731 + 789221) + 1376312589;
    return (double)(n & 0x7fffffff) / (double)0x7fffffff * 2.0 - 1.0;
}

void camera_update(ObjCamera* camera, double dt) {
    if (!camera) return;

    // Update follow target
    if (camera->target) {
        double target_x = camera->target->x;
        double target_y = camera->target->y;

        // Lerp toward target
        camera->x += (target_x - camera->x) * camera->follow_lerp;
        camera->y += (target_y - camera->y) * camera->follow_lerp;
    }

    // Update shake
    if (camera->shake_duration > 0) {
        camera->shake_time += dt;
        camera->shake_duration -= dt;

        if (camera->shake_duration > 0) {
            // Calculate shake offset based on intensity and time
            double progress = camera->shake_time;
            camera->shake_offset_x = camera_random(progress) * camera->shake_intensity;
            camera->shake_offset_y = camera_random(progress + 100.0) * camera->shake_intensity;
        } else {
            // Shake ended
            camera->shake_offset_x = 0;
            camera->shake_offset_y = 0;
            camera->shake_intensity = 0;
        }
    }
}

// ============================================================================
// Animation Objects
// ============================================================================

ObjAnimation* animation_new(ObjImage* image, int frame_width, int frame_height) {
    ObjAnimation* anim = ALLOCATE_OBJ(ObjAnimation, OBJ_ANIMATION);
    anim->image = image;
    anim->frame_width = frame_width;
    anim->frame_height = frame_height;
    anim->frames = NULL;
    anim->frame_count = 0;
    anim->frame_time = 0.1;  // 10 FPS default
    anim->current_time = 0;
    anim->current_frame = 0;
    anim->playing = false;
    anim->looping = true;
    anim->on_complete = NULL;
    return anim;
}

void animation_set_frames(ObjAnimation* anim, int* frames, int count, double frame_time) {
    if (!anim) return;

    // Free old frames
    if (anim->frames) {
        PH_FREE(anim->frames);
    }

    // Allocate and copy new frames
    anim->frames = PH_ALLOC(sizeof(int) * count);
    for (int i = 0; i < count; i++) {
        anim->frames[i] = frames[i];
    }
    anim->frame_count = count;
    anim->frame_time = frame_time;
    anim->current_frame = 0;
    anim->current_time = 0;
}

bool animation_update(ObjAnimation* anim, double dt) {
    if (!anim || !anim->playing || anim->frame_count == 0) {
        return false;
    }

    anim->current_time += dt;

    // Check if we need to advance frame
    if (anim->current_time >= anim->frame_time) {
        anim->current_time -= anim->frame_time;
        anim->current_frame++;

        // Check if animation completed
        if (anim->current_frame >= anim->frame_count) {
            if (anim->looping) {
                anim->current_frame = 0;
            } else {
                anim->current_frame = anim->frame_count - 1;
                anim->playing = false;
                return true;  // Animation completed
            }
        }
    }

    return false;
}

// ============================================================================
// Particle Emitter Objects
// ============================================================================

// Simple random number generator for particles
static uint32_t particle_seed = 12345;

static double particle_random(void) {
    particle_seed = particle_seed * 1103515245 + 12345;
    return (double)(particle_seed & 0x7fffffff) / (double)0x7fffffff;
}

static double particle_random_range(double min, double max) {
    return min + particle_random() * (max - min);
}

ObjParticleEmitter* particle_emitter_new(double x, double y) {
    ObjParticleEmitter* emitter = ALLOCATE_OBJ(ObjParticleEmitter, OBJ_PARTICLE_EMITTER);
    emitter->x = x;
    emitter->y = y;
    // Default particle properties
    emitter->speed_min = 50;
    emitter->speed_max = 100;
    emitter->angle_min = 0;
    emitter->angle_max = 360;
    emitter->life_min = 0.5;
    emitter->life_max = 1.0;
    emitter->size_min = 4;
    emitter->size_max = 8;
    emitter->color = 0xFFFFFFFF;  // White
    emitter->fade = true;
    emitter->gravity = 0;
    // Emission settings
    emitter->rate = 0;
    emitter->emit_timer = 0;
    emitter->active = true;
    // Particle storage
    emitter->particle_count = 0;
    return emitter;
}

void particle_emitter_emit(ObjParticleEmitter* emitter, int count) {
    if (!emitter) return;

    for (int i = 0; i < count && emitter->particle_count < PARTICLE_MAX; i++) {
        Particle* p = &emitter->particles[emitter->particle_count++];

        // Position at emitter
        p->x = emitter->x;
        p->y = emitter->y;

        // Random velocity within angle/speed range
        double angle = particle_random_range(emitter->angle_min, emitter->angle_max);
        double speed = particle_random_range(emitter->speed_min, emitter->speed_max);
        double rad = angle * 3.14159265358979 / 180.0;
        p->vx = cos(rad) * speed;
        p->vy = sin(rad) * speed;

        // Random lifetime and size
        p->life = particle_random_range(emitter->life_min, emitter->life_max);
        p->max_life = p->life;
        p->size = particle_random_range(emitter->size_min, emitter->size_max);
        p->color = emitter->color;
    }
}

void particle_emitter_update(ObjParticleEmitter* emitter, double dt) {
    if (!emitter || !emitter->active) return;

    // Rate-based emission
    if (emitter->rate > 0) {
        emitter->emit_timer += dt;
        double interval = 1.0 / emitter->rate;
        while (emitter->emit_timer >= interval) {
            particle_emitter_emit(emitter, 1);
            emitter->emit_timer -= interval;
        }
    }

    // Update all particles
    int alive = 0;
    for (int i = 0; i < emitter->particle_count; i++) {
        Particle* p = &emitter->particles[i];

        // Update lifetime
        p->life -= dt;
        if (p->life <= 0) {
            continue;  // Particle is dead
        }

        // Apply gravity
        p->vy += emitter->gravity * dt;

        // Update position
        p->x += p->vx * dt;
        p->y += p->vy * dt;

        // Apply fade
        if (emitter->fade) {
            double alpha = (p->life / p->max_life) * 255.0;
            p->color = (emitter->color & 0x00FFFFFF) | ((uint32_t)alpha << 24);
        }

        // Keep alive particles at the front
        if (alive != i) {
            emitter->particles[alive] = *p;
        }
        alive++;
    }
    emitter->particle_count = alive;
}

// ============================================================================
// Object Utilities
// ============================================================================

void object_print(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
        case OBJ_FUNCTION: {
            ObjFunction* fn = AS_FUNCTION(value);
            if (fn->name == NULL) {
                printf("<fn>");
            } else {
                printf("<fn %s>", fn->name->chars);
            }
            break;
        }
        case OBJ_CLOSURE: {
            ObjClosure* closure = AS_CLOSURE(value);
            if (closure->function->name == NULL) {
                printf("<fn>");
            } else {
                printf("<fn %s>", closure->function->name->chars);
            }
            break;
        }
        case OBJ_UPVALUE:
            printf("<upvalue>");
            break;
        case OBJ_STRUCT_DEF: {
            ObjStructDef* def = AS_STRUCT_DEF(value);
            printf("<struct %s>", def->name->chars);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance* instance = AS_INSTANCE(value);
            printf("<%s instance>", instance->struct_def->name->chars);
            break;
        }
        case OBJ_LIST: {
            ObjList* list = AS_LIST(value);
            printf("[");
            for (int i = 0; i < list->count; i++) {
                if (i > 0) printf(", ");
                value_print(list->items[i]);
            }
            printf("]");
            break;
        }
        case OBJ_NATIVE: {
            ObjNative* native = AS_NATIVE(value);
            if (native->name == NULL) {
                printf("<native fn>");
            } else {
                printf("<native fn %s>", native->name->chars);
            }
            break;
        }
        case OBJ_VEC2: {
            ObjVec2* vec = AS_VEC2(value);
            printf("vec2(%g, %g)", vec->x, vec->y);
            break;
        }
        case OBJ_IMAGE: {
            ObjImage* image = AS_IMAGE(value);
            if (image->path) {
                printf("<image %s>", image->path->chars);
            } else {
                printf("<image %dx%d>", image->width, image->height);
            }
            break;
        }
        case OBJ_SPRITE: {
            ObjSprite* sprite = AS_SPRITE(value);
            printf("<sprite at (%.1f, %.1f)>", sprite->x, sprite->y);
            break;
        }
        case OBJ_FONT: {
            ObjFont* font = AS_FONT(value);
            if (font->is_default) {
                printf("<font default %d>", font->size);
            } else {
                printf("<font %d>", font->size);
            }
            break;
        }
        case OBJ_SOUND: {
            ObjSound* sound = AS_SOUND(value);
            if (sound->path) {
                printf("<sound %s>", sound->path->chars);
            } else {
                printf("<sound>");
            }
            break;
        }
        case OBJ_MUSIC: {
            ObjMusic* music = AS_MUSIC(value);
            if (music->path) {
                printf("<music %s>", music->path->chars);
            } else {
                printf("<music>");
            }
            break;
        }
        case OBJ_CAMERA: {
            ObjCamera* camera = AS_CAMERA(value);
            printf("<camera at (%.1f, %.1f) zoom=%.1f>", camera->x, camera->y, camera->zoom);
            break;
        }
        case OBJ_ANIMATION: {
            ObjAnimation* anim = AS_ANIMATION(value);
            printf("<animation %d frames @ %.1f fps>", anim->frame_count, 1.0 / anim->frame_time);
            break;
        }
        case OBJ_PARTICLE_EMITTER: {
            ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(value);
            printf("<particle_emitter at (%.1f, %.1f) %d particles>", emitter->x, emitter->y, emitter->particle_count);
            break;
        }
    }
}

uint32_t object_hash(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            return AS_STRING(value)->hash;
        case OBJ_VEC2: {
            // Hash based on x and y
            ObjVec2* vec = AS_VEC2(value);
            uint64_t x_bits, y_bits;
            memcpy(&x_bits, &vec->x, sizeof(x_bits));
            memcpy(&y_bits, &vec->y, sizeof(y_bits));
            uint32_t hash = (uint32_t)(x_bits ^ (x_bits >> 32));
            hash ^= (uint32_t)(y_bits ^ (y_bits >> 32));
            return hash;
        }
        default:
            // Use pointer as hash for other object types
            return (uint32_t)(uintptr_t)AS_OBJECT(value);
    }
}

const char* object_type_name(ObjectType type) {
    switch (type) {
        case OBJ_STRING:     return "string";
        case OBJ_FUNCTION:   return "function";
        case OBJ_CLOSURE:    return "closure";
        case OBJ_UPVALUE:    return "upvalue";
        case OBJ_STRUCT_DEF: return "struct";
        case OBJ_INSTANCE:   return "instance";
        case OBJ_LIST:       return "list";
        case OBJ_NATIVE:     return "native";
        case OBJ_VEC2:       return "vec2";
        case OBJ_IMAGE:      return "image";
        case OBJ_SPRITE:     return "sprite";
        case OBJ_FONT:       return "font";
        case OBJ_SOUND:      return "sound";
        case OBJ_MUSIC:      return "music";
        case OBJ_CAMERA:     return "camera";
        case OBJ_ANIMATION:  return "animation";
        case OBJ_PARTICLE_EMITTER: return "particle_emitter";
        default:             return "unknown";
    }
}

void object_free(Object* object) {
    switch (object->type) {
        case OBJ_STRING:
            // Remove from intern table
            // (The string is freed with the object)
            break;
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            if (function->chunk != NULL) {
                chunk_free(function->chunk);
                PH_FREE(function->chunk);
            }
            break;
        }
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)object;
            PH_FREE(closure->upvalues);
            break;
        }
        case OBJ_UPVALUE:
            // Nothing extra to free
            break;
        case OBJ_STRUCT_DEF: {
            ObjStructDef* def = (ObjStructDef*)object;
            PH_FREE(def->fields);
            table_free(&def->methods);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*)object;
            PH_FREE(instance->fields);
            break;
        }
        case OBJ_LIST: {
            ObjList* list = (ObjList*)object;
            PH_FREE(list->items);
            break;
        }
        case OBJ_NATIVE:
            // Nothing extra to free
            break;
        case OBJ_VEC2:
            // Nothing extra to free
            break;
        case OBJ_IMAGE: {
            ObjImage* image = (ObjImage*)object;
            image_destroy_texture(image);
            break;
        }
        case OBJ_SPRITE:
            // Sprite doesn't own its image; image is a reference
            break;
        case OBJ_FONT: {
            ObjFont* font = (ObjFont*)object;
            font_destroy_handle(font);
            break;
        }
        case OBJ_SOUND: {
            ObjSound* sound = (ObjSound*)object;
            sound_destroy_handle(sound);
            break;
        }
    #include "vm/object.h"
#include "vm/chunk.h"
#include "vm/gc.h"
#include "core/table.h"
#include "core/strings.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// String intern table
static Table strings;

// ============================================================================
// String Objects
// ============================================================================

void strings_init(void) {
    table_init(&strings);
}

void strings_free(void) {
    table_free(&strings);
}

uint32_t string_hash(const char* chars, int length) {
    // FNV-1a hash
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)chars[i];
        hash *= 16777619;
    }
    return hash;
}

static ObjString* allocate_string(const char* chars, int length, uint32_t hash) {
    // Check if already interned
    const char* interned = table_find_string(&strings, chars, length, hash);
    if (interned != NULL) {
        // Find the ObjString that owns this interned string
        // The string chars are at a fixed offset from the ObjString
        return (ObjString*)(interned - offsetof(ObjString, chars));
    }

    // Allocate new string using GC
    ObjString* string = (ObjString*)gc_allocate_object(
        sizeof(ObjString) + length + 1, OBJ_STRING);
    string->length = length;
    string->hash = hash;
    memcpy(string->chars, chars, length);
    string->chars[length] = '\0';

    // Intern the string
    table_set(&strings, string->chars, length, string);

    return string;
}

ObjString* string_copy(const char* chars, int length) {
    uint32_t hash = string_hash(chars, length);
    return allocate_string(chars, length, hash);
}

ObjString* string_take(char* chars, int length) {
    uint32_t hash = string_hash(chars, length);

    // Check if already interned
    const char* interned = table_find_string(&strings, chars, length, hash);
    if (interned != NULL) {
        // Free the passed string and return the interned one
        PH_FREE(chars);
        return (ObjString*)(interned - offsetof(ObjString, chars));
    }

    // Create new string using GC (we still need to copy since ObjString uses flexible array)
    ObjString* string = (ObjString*)gc_allocate_object(
        sizeof(ObjString) + length + 1, OBJ_STRING);
    string->length = length;
    string->hash = hash;
    memcpy(string->chars, chars, length);
    string->chars[length] = '\0';

    // Free the original
    PH_FREE(chars);

    // Intern the string
    table_set(&strings, string->chars, length, string);

    return string;
}

ObjString* string_concat(ObjString* a, ObjString* b) {
    int length = a->length + b->length;
    char* chars = PH_ALLOC(length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    return string_take(chars, length);
}

ObjString* string_intern(const char* chars, int length) {
    return string_copy(chars, length);
}

// ============================================================================
// Function Objects
// ============================================================================

ObjFunction* function_new(void) {
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->upvalue_count = 0;
    function->chunk = NULL;  // Will be set in Phase 7
    function->name = NULL;
    return function;
}

// ============================================================================
// Closure Objects
// ============================================================================

ObjClosure* closure_new(ObjFunction* function) {
    ObjUpvalue** upvalues = PH_ALLOC(sizeof(ObjUpvalue*) * function->upvalue_count);
    for (int i = 0; i < function->upvalue_count; i++) {
        upvalues[i] = NULL;
    }

    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalue_count = function->upvalue_count;
    return closure;
}

// ============================================================================
// Upvalue Objects
// ============================================================================

ObjUpvalue* upvalue_new(Value* slot) {
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->location = slot;
    upvalue->closed = NONE_VAL;
    upvalue->next = NULL;
    return upvalue;
}

// ============================================================================
// Struct Definition Objects
// ============================================================================

ObjStructDef* struct_def_new(ObjString* name, int field_count) {
    ObjStructDef* def = ALLOCATE_OBJ(ObjStructDef, OBJ_STRUCT_DEF);
    def->name = name;
    def->field_count = field_count;
    def->fields = PH_ALLOC(sizeof(ObjString*) * field_count);
    for (int i = 0; i < field_count; i++) {
        def->fields[i] = NULL;
    }
    table_init(&def->methods);
    return def;
}

// ============================================================================
// Instance Objects
// ============================================================================

ObjInstance* instance_new(ObjStructDef* struct_def) {
    ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->struct_def = struct_def;
    instance->fields = PH_ALLOC(sizeof(Value) * struct_def->field_count);
    for (int i = 0; i < struct_def->field_count; i++) {
        instance->fields[i] = NONE_VAL;
    }
    return instance;
}

// ============================================================================
// List Objects
// =======