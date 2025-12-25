ys
    PAL_KEY_RETURN = 40,
    PAL_KEY_ESCAPE = 41,
    PAL_KEY_BACKSPACE = 42,
    PAL_KEY_TAB = 43,
    PAL_KEY_SPACE = 44,
    PAL_KEY_LCTRL = 224,
    PAL_KEY_LSHIFT = 225,
    PAL_KEY_LALT = 226,
    PAL_KEY_RCTRL = 228,
    PAL_KEY_RSHIFT = 229,
    PAL_KEY_RALT = 230,

    PAL_KEY_COUNT = 256
} PalKey;

// Poll events (call once per frame)
void pal_poll_events(void);

// Check if window should close
bool pal_should_quit(void);

// Key state queries
bool pal_key_down(PalKey key);      // Currently held down
bool pal_key_pressed(PalKey key);   // Just pressed this frame
bool pal_key_released(PalKey key);  // Just released this frame

// -----------------------------------------------------------------------------
// Input - Mouse
// -----------------------------------------------------------------------------

typedef enum {
    PAL_MOUSE_LEFT = 1,
    PAL_MOUSE_MIDDLE = 2,
    PAL_MOUSE_RIGHT = 3,
} PalMouseButton;

void pal_mouse_position(int* x, int* y);
bool pal_mouse_down(PalMouseButton button);
bool pal_mouse_pressed(PalMouseButton button);
bool pal_mouse_released(PalMouseButton button);

// -----------------------------------------------------------------------------
// Audio - Sound effects
// -----------------------------------------------------------------------------

PalSound* pal_sound_load(const char* path);
void pal_sound_destroy(PalSound* sound);
void pal_sound_play(PalSound* sound);
void pal_sound_play_volume(PalSound* sound, float volume);  // volume: 0.0 - 1.0

// -----------------------------------------------------------------------------
// Audio - Music
// -----------------------------------------------------------------------------

PalMusic* pal_music_load(const char* path);
void pal_music_destroy(PalMusic* music);
void pal_music_play(PalMusic* music, bool loop);
void pal_music_stop(void);
void pal_music_pause(void);
void pal_music_resume(void);
void pal_music_set_volume(float volume);  // volume: 0.0 - 1.0
bool pal_music_is_playing(void);

// -----------------------------------------------------------------------------
// Audio - Master volume
// -----------------------------------------------------------------------------

void pal_set_master_volume(float volume);  // volume: 0.0 - 1.0, affects all audio

// -----------------------------------------------------------------------------
// Time
// -----------------------------------------------------------------------------

// Get time since PAL init in seconds
double pal_time(void);

// Sleep for a duration in seconds
void pal_sleep(double seconds);

// -----------------------------------------------------------------------------
// Mock backend support (for testing)
// -----------------------------------------------------------------------------

#ifdef PAL_MOCK_ENABLED

// Call recording for verification
typedef struct {
    const char* function;
    // Additional parameters could be stored here
} PalMockCall;

// Get recorded calls
const PalMockCall* pal_mock_get_calls(int* count);

// Clear recorded calls
void pal_mock_clear_calls(void);

// Simulate input
void pal_mock_set_key(PalKey key, bool down);
void pal_mock_set_mouse_button(PalMouseButton button, bool down);
void pal_mock_set_mouse_position(int x, int y);
void pal_mock_set_quit(bool quit);

#endif // PAL_MOCK_ENABLED

#endif // PLACEHOLDER_PAL_H
// Platform Abstraction Layer
// Provides a clean interface for platform-specific code (window, input, audio, etc.)

#ifndef PLACEHOLDER_PAL_H
#define PLACEHOLDER_PAL_H

#include <stdbool.h>
#include <stdint.h>

// Forward declarations
typedef struct PalWindow PalWindow;
typedef struct PalTexture PalTexture;
typedef struct PalSound PalSound;
typedef struct PalMusic PalMusic;
typedef struct PalFont PalFont;

// -----------------------------------------------------------------------------
// Backend selection
// -----------------------------------------------------------------------------

typedef enum {
    PAL_BACKEND_SDL2,
    PAL_BACKEND_MOCK,
} PalBackend;

// Initialize the PAL with a specific backend
bool pal_init(PalBackend backend);

// Shutdown the PAL
void pal_quit(void);

// Get the current backend
PalBackend pal_get_backend(void);

// -----------------------------------------------------------------------------
// Window management
// -----------------------------------------------------------------------------

PalWindow* pal_window_create(const char* title, int width, int height);
void pal_window_destroy(PalWindow* window);
void pal_window_present(PalWindow* window);
void pal_window_clear(PalWindow* window, uint8_t r, uint8_t g, uint8_t b);
void pal_window_set_title(PalWindow* window, const char* title);
void pal_window_get_size(PalWindow* window, int* width, int* height);

// -----------------------------------------------------------------------------
// Rendering primitives
// -----------------------------------------------------------------------------

void pal_draw_rect(PalWindow* window, int x, int y, int width, int height,
                   uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void pal_draw_rect_outline(PalWindow* window, int x, int y, int width, int height,
                           uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void pal_draw_line(PalWindow* window, int x1, int y1, int x2, int y2,
                   uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void pal_draw_circle(PalWindow* window, int cx, int cy, int radius,
                     uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void pal_draw_circle_outline(PalWindow* window, int cx, int cy, int radius,
                             uint8_t r, uint8_t g, uint8_t b, uint8_t a);

// -----------------------------------------------------------------------------
// Textures
// -----------------------------------------------------------------------------

PalTexture* pal_texture_load(PalWindow* window, const char* path);
void pal_texture_destroy(PalTexture* texture);
void pal_texture_get_size(PalTexture* texture, int* width, int* height);

void pal_draw_texture(PalWindow* window, PalTexture* texture,
                      int x, int y, int width, int height);

void pal_draw_texture_ex(PalWindow* window, PalTexture* texture,
                         int x, int y, int width, int height,
                         double rotation, int origin_x, int origin_y,
                         bool flip_h, bool flip_v);

// Source rectangle for sprite sheets
void pal_draw_texture_region(PalWindow* window, PalTexture* texture,
                             int src_x, int src_y, int src_w, int src_h,
                             int dst_x, int dst_y, int dst_w, int dst_h);

// -----------------------------------------------------------------------------
// Fonts and Text
// -----------------------------------------------------------------------------

PalFont* pal_font_load(const char* path, int size);
void pal_font_destroy(PalFont* font);
void pal_draw_text(PalWindow* window, PalFont* font, const char* text,
                   int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void pal_text_size(PalFont* font, const char* text, int* width, int* height);

// Default embedded font (simple pixel font)
PalFont* pal_font_default(int size);

// -----------------------------------------------------------------------------
// Input - Keyboard
// -----------------------------------------------------------------------------

// Key codes (subset matching common game keys)
typedef enum {
    PAL_KEY_UNKNOWN = 0,

    // Letters
    PAL_KEY_A = 4,
    PAL_KEY_B = 5,
    PAL_KEY_C = 6,
    PAL_KEY_D = 7,
    PAL_KEY_E = 8,
    PAL_KEY_F = 9,
    PAL_KEY_G = 10,
    PAL_KEY_H = 11,
    PAL_KEY_I = 12,
    PAL_KEY_J = 13,
    PAL_KEY_K = 14,
    PAL_KEY_L = 15,
    PAL_KEY_M = 16,
    PAL_KEY_N = 17,
    PAL_KEY_O = 18,
    PAL_KEY_P = 19,
    PAL_KEY_Q = 20,
    PAL_KEY_R = 21,
    PAL_KEY_S = 22,
    PAL_KEY_T = 23,
    PAL_KEY_U = 24,
    PAL_KEY_V = 25,
    PAL_KEY_W = 26,
    PAL_KEY_X = 27,
    PAL_KEY_Y = 28,
    PAL_KEY_Z = 29,

    // Numbers
    PAL_KEY_0 = 39,
    PAL_KEY_1 = 30,
    PAL_KEY_2 = 31,
    PAL_KEY_3 = 32,
    PAL_KEY_4 = 33,
    PAL_KEY_5 = 34,
    PAL_KEY_6 = 35,
    PAL_KEY_7 = 36,
    PAL_KEY_8 = 37,
    PAL_KEY_9 = 38,

    // Function keys
    PAL_KEY_F1 = 58,
    PAL_KEY_F2 = 59,
    PAL_KEY_F3 = 60,
    PAL_KEY_F4 = 61,
    PAL_KEY_F5 = 62,
    PAL_KEY_F6 = 63,
    PAL_KEY_F7 = 64,
    PAL_KEY_F8 = 65,
    PAL_KEY_F9 = 66,
    PAL_KEY_F10 = 67,
    PAL_KEY_F11 = 68,
    PAL_KEY_F12 = 69,

    // Arrow keys
    PAL_KEY_RIGHT = 79,
    PAL_KEY_LEFT = 80,
    PAL_KEY_DOWN = 81,
    PAL_KEY_UP = 82,

    // Special ke