chars, int length);

// Concatenate two strings
ObjString* string_concat(ObjString* a, ObjString* b);

// Hash a string (FNV-1a)
uint32_t string_hash(const char* chars, int length);

// String equality (pointer comparison due to interning)
#define STRING_EQUAL(a, b)  ((a) == (b))

// ============================================================================
// Function Object
// ============================================================================

typedef struct {
    Object obj;
    int arity;              // Number of parameters
    int upvalue_count;      // Number of captured variables
    Chunk* chunk;           // Bytecode (NULL until Phase 7)
    ObjString* name;        // Function name (NULL for anonymous)
} ObjFunction;

#define AS_FUNCTION(v)      ((ObjFunction*)AS_OBJECT(v))

ObjFunction* function_new(void);

// ============================================================================
// Closure Object (function + captured upvalues)
// ============================================================================

// Forward declaration
typedef struct ObjUpvalue ObjUpvalue;

typedef struct {
    Object obj;
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalue_count;
} ObjClosure;

#define AS_CLOSURE(v)       ((ObjClosure*)AS_OBJECT(v))

ObjClosure* closure_new(ObjFunction* function);

// ============================================================================
// Upvalue Object (captured variable)
// ============================================================================

struct ObjUpvalue {
    Object obj;
    Value* location;        // Pointer to the variable
    Value closed;           // Closed-over value (when hoisted to heap)
    struct ObjUpvalue* next; // For linked list of open upvalues
};

#define AS_UPVALUE(v)       ((ObjUpvalue*)AS_OBJECT(v))

ObjUpvalue* upvalue_new(Value* slot);

// ============================================================================
// Struct Definition Object
// ============================================================================

typedef struct {
    Object obj;
    ObjString* name;
    ObjString** fields;     // Array of field names
    int field_count;
    Table methods;          // Hash table of methods (ObjString* -> ObjClosure*)
} ObjStructDef;

#define AS_STRUCT_DEF(v)    ((ObjStructDef*)AS_OBJECT(v))

ObjStructDef* struct_def_new(ObjString* name, int field_count);

// ============================================================================
// Instance Object (instance of a struct)
// ============================================================================

typedef struct {
    Object obj;
    ObjStructDef* struct_def;
    Value* fields;          // Array of field values
} ObjInstance;

#define AS_INSTANCE(v)      ((ObjInstance*)AS_OBJECT(v))

ObjInstance* instance_new(ObjStructDef* struct_def);

// ============================================================================
// List Object
// ============================================================================

typedef struct {
    Object obj;
    Value* items;
    int count;
    int capacity;
} ObjList;

#define AS_LIST(v)          ((ObjList*)AS_OBJECT(v))

ObjList* list_new(void);
void list_append(ObjList* list, Value value);
Value list_get(ObjList* list, int index);
void list_set(ObjList* list, int index, Value value);
int list_length(ObjList* list);

// ============================================================================
// Native Function Object
// ============================================================================

typedef Value (*NativeFn)(int arg_count, Value* args);

typedef struct {
    Object obj;
    NativeFn function;
    ObjString* name;
    int arity;              // -1 for variadic
} ObjNative;

#define AS_NATIVE(v)        ((ObjNative*)AS_OBJECT(v))

ObjNative* native_new(NativeFn function, ObjString* name, int arity);

// ============================================================================
// Vec2 Object (2D vector for game math)
// ============================================================================

typedef struct {
    Object obj;
    double x;
    double y;
} ObjVec2;

#define AS_VEC2(v)          ((ObjVec2*)AS_OBJECT(v))

ObjVec2* vec2_new(double x, double y);
ObjVec2* vec2_add(ObjVec2* a, ObjVec2* b);
ObjVec2* vec2_sub(ObjVec2* a, ObjVec2* b);
ObjVec2* vec2_mul(ObjVec2* a, ObjVec2* b);
ObjVec2* vec2_scale(ObjVec2* v, double s);
double vec2_dot(ObjVec2* a, ObjVec2* b);
double vec2_length(ObjVec2* v);
double vec2_length_squared(ObjVec2* v);
ObjVec2* vec2_normalize(ObjVec2* v);
double vec2_distance(ObjVec2* a, ObjVec2* b);

// ============================================================================
// Image Object (loaded texture wrapper)
// ============================================================================

// Forward declaration for PAL texture (avoids including pal.h)
typedef struct PalTexture PalTexture;

// Callback type for texture destruction (set by engine)
typedef void (*TextureDestroyFn)(PalTexture* texture);

// Set the texture destructor callback (called by engine on init)
void image_set_texture_destructor(TextureDestroyFn fn);

typedef struct {
    Object obj;
    PalTexture* texture;      // PAL texture handle (not GC-managed)
    int width;
    int height;
    ObjString* path;          // Path for identification/debugging
} ObjImage;

#define AS_IMAGE(v)         ((ObjImage*)AS_OBJECT(v))

ObjImage* image_new(PalTexture* texture, int width, int height, ObjString* path);
void image_destroy_texture(ObjImage* image);  // Called when freeing the object

// ============================================================================
// Animation Object (frame-based animation)
// Must be defined before ObjSprite since ObjSprite contains ObjAnimation*
// ============================================================================

typedef struct ObjAnimation {
    Object obj;
    ObjImage* image;          // Source sprite sheet
    int frame_width;          // Width of each frame
    int frame_height;         // Height of each frame
    int* frames;              // Array of frame indices
    int frame_count;          // Number of frames
    double frame_time;        // Time per frame in seconds
    double current_time;      // Time into current frame
    int current_frame;        // Current frame index (into frames array)
    bool playing;             // Is animation playing?
    bool looping;             // Does animation loop?
    ObjClosure* on_complete;  // Callback when animation finishes (non-looping)
} ObjAnimation;

#define AS_ANIMATION(v)     ((ObjAnimation*)AS_OBJECT(v))

ObjAnimation* animation_new(ObjImage* image, int frame_width, int frame_height);
void animation_set_frames(ObjAnimation* anim, int* frames, int count, double frame_time);
bool animation_update(ObjAnimation* anim, double dt);  // Returns true if completed

// ============================================================================
// Sprite Object (drawable entity with position/transform)
// ============================================================================

typedef struct {
    Object obj;
    ObjImage* image;          // Source image
    double x, y;              // Position
    double width, height;     // Size (0 = use image size)
    double rotation;          // Rotation in degrees
    double scale_x, scale_y;  // Scale factors
    double origin_x, origin_y; // Origin as ratio (0.5 = center)
    bool visible;             // Whether to draw
    bool flip_x, flip_y;      // Flip flags
    // For sprite sheets
    int frame_x, frame_y;     // Current frame position in sheet
    int frame_width, frame_height; // Frame size (0 = use full image)
    // Physics properties
    double velocity_x, velocity_y;      // Current velocity
    double acceleration_x, acceleration_y; // Current acceleration
    double friction;                     // Friction coefficient (0-1, applied each frame)
    double gravity_scale;                // Per-sprite gravity multiplier
    bool grounded;                       // Is sprite on ground?
    // Animation
    ObjAnimation* animation;             // Current animation (NULL if none)
} ObjSprite;

#define AS_SPRITE(v)        ((ObjSprite*)AS_OBJECT(v))

ObjSprite* sprite_new(ObjImage* image);

// ============================================================================
// Font Object (loaded font wrapper)
// ============================================================================

// Forward declaration for PAL font
typedef struct PalFont PalFont;

// Callback type for font destruction (set by engine)
typedef void (*FontDestroyFn)(PalFont* font);

// Set the font destructor callback (called by engine on init)
void font_set_destructor(FontDestroyFn fn);

typedef struct {
    Object obj;
    PalFont* font;        // PAL font handle (not GC-managed)
    int size;             // Font size in pixels
    bool is_default;      // Whether this is the default embedded font
} ObjFont;

#define AS_FONT(v)          ((ObjFont*)AS_OBJECT(v))

ObjFont* font_new(PalFont* font, int size, bool is_default);
void font_destroy_handle(ObjFont* font);  // Called when freeing the object

// ============================================================================
// Sound Object (loaded sound effect wrapper)
// ============================================================================

// Forward declaration for PAL sound
typedef struct PalSound PalSound;

// Callback type for sound destruction (set by engine)
typedef void (*SoundDestroyFn)(PalSound* sound);

// Set the sound destructor callback (called by engine on init)
void sound_set_destructor(SoundDestroyFn fn);

typedef struct {
    Object obj;
    PalSound* sound;      // PAL sound handle (not GC-managed)
    ObjString* path;      // Path for identification/debugging
} ObjSound;

#define AS_SOUND(v)         ((ObjSound*)AS_OBJECT(v))

ObjSound* sound_new(PalSound* sound, ObjString* path);
void sound_destroy_handle(ObjSound* sound);  // Called when freeing the object

// ============================================================================
// Music Object (loaded music wrapper)
// ============================================================================

// Forward declaration for PAL music
typedef struct PalMusic PalMusic;

// Callback type for music destruction (set by engine)
typedef void (*MusicDestroyFn)(PalMusic* music);

// Set the music destructor callback (called by engine on init)
void music_set_destructor(MusicDestroyFn fn);

typedef struct {
    Object obj;
    PalMusic* music;      // PAL music handle (not GC-managed)
    ObjString* path;      // Path for identification/debugging
} ObjMusic;

#define AS_MUSIC(v)         ((ObjMusic*)AS_OBJECT(v))

ObjMusic* music_new(PalMusic* music, ObjString* path);
void music_destroy_handle(ObjMusic* music);  // Called when freeing the object

// ============================================================================
// Camera Object (viewport for the game world)
// ============================================================================

typedef struct {
    Object obj;
    double x, y;              // Camera position (world coordinates)
    double zoom;              // Zoom factor (1.0 = normal)
    double rotation;          // Rotation in degrees
    ObjSprite* target;        // Sprite to follow (NULL if not following)
    double follow_lerp;       // Follow smoothness (0-1, 1 = instant)
    // Camera shake
    double shake_intensity;   // Current shake intensity
    double shake_duration;    // Remaining shake duration
    double shake_time;        // Elapsed shake time
    double shake_offset_x;    // Current shake offset
    double shake_offset_y;
} ObjCamera;

#define AS_CAMERA(v)        ((ObjCamera*)AS_OBJECT(v))

ObjCamera* camera_new(void);
void camera_update(ObjCamera* camera, double dt);  // Update follow and shake

// ============================================================================
// Particle Emitter Object
// ============================================================================

// Maximum particles per emitter
#define PARTICLE_MAX 256

// Individual particle (stored inside emitter, not a separate object)
typedef struct {
    double x, y;          // Position
    double vx, vy;        // Velocity
    double life;          // Remaining lifetime
    double max_life;      // Maximum lifetime (for alpha calculation)
    double size;          // Current size
    uint32_t color;       // Particle color (RGBA)
} Particle;

// Particle emitter configuration
typedef struct {
    Object obj;
    double x, y;                  // Emitter position
    // Particle properties (ranges)
    double speed_min, speed_max;  // Speed range
    double angle_min, angle_max;  // Emission angle range (degrees)
    double life_min, life_max;    // Lifetime range (seconds)
    double size_min, size_max;    // Size range
    uint32_t color;               // Base color
    bool fade;                    // Fade alpha over lifetime
    double gravity;               // Gravity applied to particles
    // Emission settings
    double rate;                  // Particles per second (0 = manual emit only)
    double emit_timer;            // Time accumulator for rate-based emission
    bool active;                  // Is emitter active
    // Particle storage
    Particle particles[PARTICLE_MAX];
    int particle_count;           // Current number of live particles
} ObjParticleEmitter;

#define AS_PARTICLE_EMITTER(v)  ((ObjParticleEmitter*)AS_OBJECT(v))

ObjParticleEmitter* particle_emitter_new(double x, double y);
void particle_emitter_emit(ObjParticleEmitter* emitter, int count);
void particle_emitter_update(ObjParticleEmitter* emitter, double dt);

// ============================================================================
// Object Utilities
// ============================================================================

// Print an object value
void object_print(Value value);

// Get the hash of an object
uint32_t object_hash(Value value);

// Get the type name of an object
const char* object_type_name(ObjectType type);

// Free an object's memory (used by GC)
void object_free(Object* object);

// ============================================================================
// Object Allocation & Tracking
// ============================================================================

// Allocate a new object of a given size and type
// Uses the GC system for tracking (see gc.h)
// Defined in gc.c
Object* gc_allocate_object(size_t size, int type);

#define ALLOCATE_OBJ(type, object_type) \
    (type*)gc_allocate_object(sizeof(type), object_type)

// ============================================================================
// String Interning
// ============================================================================

// Initialize the string intern table
void strings_init(void);

// Free the string intern table
void strings_free(void);

// Intern a string (returns existing or creates new)
ObjString* string_intern(const char* chars, int length);

#endif // PH_OBJECT_H
#ifndef PH_OBJECT_H
#define PH_OBJECT_H

#include "core/common.h"
#include "core/table.h"
#include "vm/value.h"

// Forward declaration for Chunk (defined in Phase 7)
typedef struct Chunk Chunk;

// Object types
typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_CLOSURE,
    OBJ_UPVALUE,
    OBJ_STRUCT_DEF,
    OBJ_INSTANCE,
    OBJ_LIST,
    OBJ_NATIVE,
    OBJ_VEC2,
    OBJ_IMAGE,
    OBJ_SPRITE,
    OBJ_FONT,
    OBJ_SOUND,
    OBJ_MUSIC,
    OBJ_CAMERA,
    OBJ_ANIMATION,
    OBJ_PARTICLE_EMITTER,
} ObjectType;

// Common header for all heap-allocated objects
struct Object {
    ObjectType type;
    bool marked;            // For garbage collection
    struct Object* next;    // Intrusive linked list for GC
};

// ============================================================================
// Object Type Checking
// ============================================================================

#define OBJ_TYPE(value)     (AS_OBJECT(value)->type)

#define IS_STRING(v)        (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_STRING)
#define IS_FUNCTION(v)      (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_FUNCTION)
#define IS_CLOSURE(v)       (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_CLOSURE)
#define IS_UPVALUE(v)       (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_UPVALUE)
#define IS_STRUCT_DEF(v)    (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_STRUCT_DEF)
#define IS_INSTANCE(v)      (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_INSTANCE)
#define IS_LIST(v)          (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_LIST)
#define IS_NATIVE(v)        (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_NATIVE)
#define IS_VEC2(v)          (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_VEC2)
#define IS_IMAGE(v)         (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_IMAGE)
#define IS_SPRITE(v)        (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_SPRITE)
#define IS_FONT(v)          (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_FONT)
#define IS_SOUND(v)         (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_SOUND)
#define IS_MUSIC(v)         (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_MUSIC)
#define IS_CAMERA(v)        (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_CAMERA)
#define IS_ANIMATION(v)     (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_ANIMATION)
#define IS_PARTICLE_EMITTER(v) (IS_OBJECT(v) && OBJ_TYPE(v) == OBJ_PARTICLE_EMITTER)

// ============================================================================
// String Object
// ============================================================================

typedef struct {
    Object obj;
    uint32_t length;
    uint32_t hash;
    char chars[];           // Flexible array member
} ObjString;

#define AS_STRING(v)        ((ObjString*)AS_OBJECT(v))
#define AS_CSTRING(v)       (((ObjString*)AS_OBJECT(v))->chars)

// Create a new string by copying characters
ObjString* string_copy(const char* chars, int length);

// Take ownership of a heap-allocated string
ObjString* string_take(char* 