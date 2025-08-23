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
   