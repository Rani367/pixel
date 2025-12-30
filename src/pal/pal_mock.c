// Mock PAL backend for testing
// Records all function calls and allows simulating input

#define PAL_MOCK_ENABLED
#include "pal/pal.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

// -----------------------------------------------------------------------------
// Call recording
// -----------------------------------------------------------------------------

#define MAX_MOCK_CALLS 1024

static PalMockCall mock_calls[MAX_MOCK_CALLS];
static int mock_call_count = 0;

static void record_call(const char* function) {
    if (mock_call_count < MAX_MOCK_CALLS) {
        mock_calls[mock_call_count].function = function;
        mock_call_count++;
    }
}

const PalMockCall* pal_mock_get_calls(int* count) {
    *count = mock_call_count;
    return mock_calls;
}

void pal_mock_clear_calls(void) {
    mock_call_count = 0;
}

// -----------------------------------------------------------------------------
// Mock state
// -----------------------------------------------------------------------------

static bool mock_initialized = false;
static bool mock_quit_requested = false;
static double mock_start_time = 0;

// Input state
static bool mock_keys_down[PAL_KEY_COUNT];
static bool mock_keys_prev[PAL_KEY_COUNT];
static bool mock_mouse_down[4];  // 0 unused, 1=left, 2=middle, 3=right
static bool mock_mouse_prev[4];
static int mock_mouse_x = 0;
static int mock_mouse_y = 0;

// -----------------------------------------------------------------------------
// Mock window
// -----------------------------------------------------------------------------

struct PalWindow {
    char title[256];
    int width;
    int height;
    uint8_t clear_r, clear_g, clear_b;
};

// -----------------------------------------------------------------------------
// Mock texture
// -----------------------------------------------------------------------------

struct PalTexture {
    char path[256];
    int width;
    int height;
};

// -----------------------------------------------------------------------------
// Mock audio
// -----------------------------------------------------------------------------

struct PalSound {
    char path[256];
};

struct PalMusic {
    char path[256];
};

struct PalFont {
    char path[256];
    int size;
    bool is_default;
};

static PalMusic* mock_current_music = NULL;
static bool mock_music_playing = false;
static bool mock_music_paused = false;
static float mock_music_volume = 1.0f;
static float mock_master_volume = 1.0f;

// -----------------------------------------------------------------------------
// Backend initialization
// -----------------------------------------------------------------------------

bool pal_mock_init(void) {
    record_call("pal_init");

    mock_initialized = true;
    mock_quit_requested = false;
    mock_start_time = (double)clock() / CLOCKS_PER_SEC;

    memset(mock_keys_down, 0, sizeof(mock_keys_down));
    memset(mock_keys_prev, 0, sizeof(mock_keys_prev));
    memset(mock_mouse_down, 0, sizeof(mock_mouse_down));
    memset(mock_mouse_prev, 0, sizeof(mock_mouse_prev));
    mock_mouse_x = 0;
    mock_mouse_y = 0;

    mock_current_music = NULL;
    mock_music_playing = false;
    mock_music_paused = false;
    mock_music_volume = 1.0f;
    mock_master_volume = 1.0f;

    return true;
}

void pal_mock_quit(void) {
    record_call("pal_quit");
    mock_initialized = false;
}

// -----------------------------------------------------------------------------
// Window management
// -----------------------------------------------------------------------------

PalWindow* pal_mock_window_create(const char* title, int width, int height) {
    record_call("pal_window_create");

    PalWindow* window = malloc(sizeof(PalWindow));
    if (!window) return NULL;

    strncpy(window->title, title ? title : "", sizeof(window->title) - 1);
    window->title[sizeof(window->title) - 1] = '\0';
    window->width = width;
    window->height = height;
    window->clear_r = window->clear_g = window->clear_b = 0;

    return window;
}

void pal_mock_window_destroy(PalWindow* window) {
    record_call("pal_window_destroy");
    free(window);
}

void pal_mock_window_present(PalWindow* window) {
    (void)window;
    record_call("pal_window_present");
}

void pal_mock_window_clear(PalWindow* window, uint8_t r, uint8_t g, uint8_t b) {
    record_call("pal_window_clear");
    if (window) {
        window->clear_r = r;
        window->clear_g = g;
        window->clear_b = b;
    }
}

void pal_mock_window_set_title(PalWindow* window, const char* title) {
    record_call("pal_window_set_title");
    if (window && title) {
        strncpy(window->title, title, sizeof(window->title) - 1);
        window->title[sizeof(window->title) - 1] = '\0';
    }
}

void pal_mock_window_get_size(PalWindow* window, int* width, int* height) {
    record_call("pal_window_get_size");
    if (window) {
        if (width) *width = window->width;
        if (height) *height = window->height;
    }
}

// -----------------------------------------------------------------------------
// Rendering primitives
// -----------------------------------------------------------------------------

void pal_mock_draw_rect(PalWindow* window, int x, int y, int width, int height,
                        uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    (void)window; (void)x; (void)y; (void)width; (void)height;
    (void)r; (void)g; (void)b; (void)a;
    record_call("pal_draw_rect");
}

void pal_mock_draw_rect_outline(PalWindow* window, int x, int y, int width, int height,
                                uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    (void)window; (void)x; (void)y; (void)width; (void)height;
    (void)r; (void)g; (void)b; (void)a;
    record_call("pal_draw_rect_outline");
}

void pal_mock_draw_line(PalWindow* window, int x1, int y1, int x2, int y2,
                        uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    (void)window; (void)x1; (void)y1; (void)x2; (void)y2;
    (void)r; (void)g; (void)b; (void)a;
    record_call("pal_draw_line");
}

void pal_mock_draw_circle(PalWindow* window, int cx, int cy, int radius,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    (void)window; (void)cx; (void)cy; (void)radius;
    (void)r; (void)g; (void)b; (void)a;
    record_call("pal_draw_circle");
}

void pal_mock_draw_circle_outline(PalWindow* window, int cx, int cy, int radius,
                                  uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    (void)window; (void)cx; (void)cy; (void)radius;
    (void)r; (void)g; (void)b; (void)a;
    record_call("pal_draw_circle_outline");
}

// -----------------------------------------------------------------------------
// Textures
// -----------------------------------------------------------------------------

PalTexture* pal_mock_texture_load(PalWindow* window, const char* path) {
    (void)window;
    record_call("pal_texture_load");

    PalTexture* texture = malloc(sizeof(PalTexture));
    if (!texture) return NULL;

    strncpy(texture->path, path ? path : "", sizeof(texture->path) - 1);
    texture->path[sizeof(texture->path) - 1] = '\0';
    // Default mock texture size
    texture->width = 64;
    texture->height = 64;

    return texture;
}

void pal_mock_texture_destroy(PalTexture* texture) {
    record_call("pal_texture_destroy");
    free(texture);
}

void pal_mock_texture_get_size(PalTexture* texture, int* width, int* height) {
    record_call("pal_texture_get_size");
    if (texture) {
        if (width) *width = texture->width;
        if (height) *height = texture->height;
    }
}

void pal_mock_draw_texture(PalWindow* window, PalTexture* texture,
                           int x, int y, int width, int height) {
    (void)window; (void)texture; (void)x; (void)y; (void)width; (void)height;
    record_call("pal_draw_texture");
}

void pal_mock_draw_texture_ex(PalWindow* window, PalTexture* texture,
                              int x, int y, int width, int height,
                              double rotation, int origin_x, int origin_y,
                              bool flip_h, bool flip_v) {
    (void)window; (void)texture; (void)x; (void)y; (void)width; (void)height;
    (void)rotation; (void)origin_x; (void)origin_y; (void)flip_h; (void)flip_v;
    record_call("pal_draw_texture_ex");
}

void pal_mock_draw_texture_region(PalWindow* window, PalTexture* texture,
                                  int src_x, int src_y, int src_w, int src_h,
                                  int dst_x, int dst_y, int dst_w, int dst_h) {
    (void)window; (void)texture;
    (void)src_x; (void)src_y; (void)src_w; (void)src_h;
    (void)dst_x; (void)dst_y; (void)dst_w; (void)dst_h;
    record_call("pal_draw_texture_region");
}

// -----------------------------------------------------------------------------
// Input - Keyboard
// -----------------------------------------------------------------------------

void pal_mock_poll_events(void) {
    record_call("pal_poll_events");
    // Copy current state to previous state for pressed/released detection
    memcpy(mock_keys_prev, mock_keys_down, sizeof(mock_keys_prev));
    memcpy(mock_mouse_prev, mock_mouse_down, sizeof(mock_mouse_prev));
}

bool pal_mock_should_quit(void) {
    record_call("pal_should_quit");
    return mock_quit_requested;
}

bool pal_mock_key_down(PalKey key) {
    if (key < 0 || key >= PAL_KEY_COUNT) return false;
    return mock_keys_down[key];
}

bool pal_mock_key_pressed(PalKey key) {
    if (key < 0 || key >= PAL_KEY_COUNT) return false;
    return mock_keys_down[key] && !mock_keys_prev[key];
}

bool pal_mock_key_released(PalKey key) {
    if (key < 0 || key >= PAL_KEY_COUNT) return false;
    return !mock_keys_down[key] && mock_keys_prev[key];
}

// -----------------------------------------------------------------------------
// Input - Mouse
// -----------------------------------------------------------------------------

void pal_mock_mouse_position(int* x, int* y) {
    if (x) *x = mock_mouse_x;
    if (y) *y = mock_mouse_y;
}

bool pal_mock_mouse_down(PalMouseButton button) {
    if (button < 1 || button > 3) return false;
    return mock_mouse_down[button];
}

bool pal_mock_mouse_pressed(PalMouseButton button) {
    if (button < 1 || button > 3) return false;
    return mock_mouse_down[button] && !mock_mouse_prev[button];
}

bool pal_mock_mouse_released(PalMouseButton button) {
    if (button < 1 || button > 3) return false;
    return !mock_mouse_down[button] && mock_mouse_prev[button];
}

// -----------------------------------------------------------------------------
// Audio - Sound effects
// -----------------------------------------------------------------------------

PalSound* pal_mock_sound_load(const char* path) {
    record_call("pal_sound_load");

    PalSound* sound = malloc(sizeof(PalSound));
    if (!sound) return NULL;

    strncpy(sound->path, path ? path : "", sizeof(sound->path) - 1);
    sound->path[sizeof(sound->path) - 1] = '\0';

    return sound;
}

void pal_mock_sound_destroy(PalSound* sound) {
    record_call("pal_sound_destroy");
    free(sound);
}

void pal_mock_sound_play(PalSound* sound) {
    (void)sound;
    record_call("pal_sound_play");
}

void pal_mock_sound_play_volume(PalSound* sound, float volume) {
    (void)sound; (void)volume;
    record_call("pal_sound_play_volume");
}

// -----------------------------------------------------------------------------
// Audio - Music
// -----------------------------------------------------------------------------

PalMusic* pal_mock_music_load(const char* path) {
    record_call("pal_music_load");

    PalMusic* music = malloc(sizeof(PalMusic));
    if (!music) return NULL;

    strncpy(music->path, path ? path : "", sizeof(music->path) - 1);
    music->path[sizeof(music->path) - 1] = '\0';

    return music;
}

void pal_mock_music_destroy(PalMusic* music) {
    record_call("pal_music_destroy");
    free(music);
}

void pal_mock_music_play(PalMusic* music, bool loop) {
    (void)loop;
    record_call("pal_music_play");
    mock_current_music = music;
    mock_music_playing = true;
    mock_music_paused = false;
}

void pal_mock_music_stop(void) {
    record_call("pal_music_stop");
    mock_music_playing = false;
    mock_music_paused = false;
}

void pal_mock_music_pause(void) {
    record_call("pal_music_pause");
    if (mock_music_playing) {
        mock_music_paused = true;
    }
}

void pal_mock_music_resume(void) {
    record_call("pal_music_resume");
    mock_music_paused = false;
}

void pal_mock_music_set_volume(float volume) {
    (void)volume;
    record_call("pal_music_set_volume");
    mock_music_volume = volume;
}

bool pal_mock_music_is_playing(void) {
    return mock_music_playing && !mock_music_paused;
}

// -----------------------------------------------------------------------------
// Audio - Master volume
// -----------------------------------------------------------------------------

void pal_mock_set_master_volume(float volume) {
    record_call("pal_set_master_volume");
    mock_master_volume = volume;
}

// -----------------------------------------------------------------------------
// Time
// -----------------------------------------------------------------------------

double pal_mock_time(void) {
    return (double)clock() / CLOCKS_PER_SEC - mock_start_time;
}

void pal_mock_sleep(double seconds) {
    (void)seconds;
    record_call("pal_sleep");
    // No actual sleep in mock - tests should run fast
}

// -----------------------------------------------------------------------------
// Mock input simulation
// -----------------------------------------------------------------------------

void pal_mock_set_key(PalKey key, bool down) {
    if (key >= 0 && key < PAL_KEY_COUNT) {
        mock_keys_down[key] = down;
    }
}

void pal_mock_set_mouse_button(PalMouseButton button, bool down) {
    if (button >= 1 && button <= 3) {
        mock_mouse_down[button] = down;
    }
}

void pal_mock_set_mouse_position(int x, int y) {
    mock_mouse_x = x;
    mock_mouse_y = y;
}

void pal_mock_set_quit(bool quit) {
    mock_quit_requested = quit;
}

// -----------------------------------------------------------------------------
// Fonts and Text
// -----------------------------------------------------------------------------

PalFont* pal_mock_font_load(const char* path, int size) {
    record_call("pal_font_load");

    PalFont* font = malloc(sizeof(PalFont));
    if (!font) return NULL;

    strncpy(font->path, path ? path : "", sizeof(font->path) - 1);
    font->path[sizeof(font->path) - 1] = '\0';
    font->size = size;
    font->is_default = false;

    return font;
}

PalFont* pal_mock_font_default(int size) {
    record_call("pal_font_default");

    PalFont* font = malloc(sizeof(PalFont));
    if (!font) return NULL;

    font->path[0] = '\0';
    font->size = size;
    font->is_default = true;

    return font;
}

void pal_mock_font_destroy(PalFont* font) {
    record_call("pal_font_destroy");
    free(font);
}

void pal_mock_draw_text(PalWindow* window, PalFont* font, const char* text,
                        int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    (void)window; (void)font; (void)text;
    (void)x; (void)y; (void)r; (void)g; (void)b; (void)a;
    record_call("pal_draw_text");
}

void pal_mock_text_size(PalFont* font, const char* text, int* width, int* height) {
    // Approximate text size: 8 pixels per character width, font size for height
    int len = text ? (int)strlen(text) : 0;
    int char_width = font ? (font->size / 2) : 8;
    int char_height = font ? font->size : 16;

    if (width) *width = len * char_width;
    if (height) *height = char_height;
}
