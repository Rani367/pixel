_volume(PalSound* sound, float volume) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_sound_play_volume(sound, volume);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_sound_play_volume(sound, volume);
            break;
    }
}

// -----------------------------------------------------------------------------
// Audio - Music
// -----------------------------------------------------------------------------

PalMusic* pal_music_load(const char* path) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_music_load(path);
#else
            return NULL;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_music_load(path);
    }
    return NULL;
}

void pal_music_destroy(PalMusic* music) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_music_destroy(music);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_music_destroy(music);
            break;
    }
}

void pal_music_play(PalMusic* music, bool loop) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_music_play(music, loop);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_music_play(music, loop);
            break;
    }
}

void pal_music_stop(void) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_music_stop();
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_music_stop();
            break;
    }
}

void pal_music_pause(void) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_music_pause();
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_music_pause();
            break;
    }
}

void pal_music_resume(void) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_music_resume();
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_music_resume();
            break;
    }
}

void pal_music_set_volume(float volume) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_music_set_volume(volume);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_music_set_volume(volume);
            break;
    }
}

bool pal_music_is_playing(void) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_music_is_playing();
#else
            return false;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_music_is_playing();
    }
    return false;
}

// -----------------------------------------------------------------------------
// Audio - Master volume
// -----------------------------------------------------------------------------

void pal_set_master_volume(float volume) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_set_master_volume(volume);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_set_master_volume(volume);
            break;
    }
}

// -----------------------------------------------------------------------------
// Time
// -----------------------------------------------------------------------------

double pal_time(void) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_time();
#else
            return 0.0;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_time();
    }
    return 0.0;
}

void pal_sleep(double seconds) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_sleep(seconds);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_sleep(seconds);
            break;
    }
}

// -----------------------------------------------------------------------------
// Fonts and Text
// -----------------------------------------------------------------------------

PalFont* pal_font_load(const char* path, int size) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_font_load(path, size);
#else
            return NULL;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_font_load(path, size);
    }
    return NULL;
}

PalFont* pal_font_default(int size) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_font_default(size);
#else
            return NULL;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_font_default(size);
    }
    return NULL;
}

void pal_font_destroy(PalFont* font) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_font_destroy(font);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_font_destroy(font);
            break;
    }
}

void pal_draw_text(PalWindow* window, PalFont* font, const char* text,
                   int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_draw_text(window, font, text, x, y, r, g, b, a);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_draw_text(window, font, text, x, y, r, g, b, a);
            break;
    }
}

void pal_text_size(PalFont* font, const char* text, int* width, int* height) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_text_size(font, text, width, height);
#else
            if (width) *width = 0;
            if (height) *height = 0;
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_text_size(font, text, width, height);
            break;
    }
}
          break;
        case PAL_BACKEND_MOCK:
            pal_mock_draw_circle_outline(window, cx, cy, radius, r, g, b, a);
            break;
    }
}

// -----------------------------------------------------------------------------
// Textures
// -----------------------------------------------------------------------------

PalTexture* pal_texture_load(PalWindow* window, const char* path) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_texture_load(window, path);
#else
            return NULL;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_texture_load(window, path);
    }
    return NULL;
}

void pal_texture_destroy(PalTexture* texture) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_texture_destroy(texture);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_texture_destroy(texture);
            break;
    }
}

void pal_texture_get_size(PalTexture* texture, int* width, int* height) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_texture_get_size(texture, width, height);
#else
            if (width) *width = 0;
            if (height) *height = 0;
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_texture_get_size(texture, width, height);
            break;
    }
}

void pal_draw_texture(PalWindow* window, PalTexture* texture,
                      int x, int y, int width, int height) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_draw_texture(window, texture, x, y, width, height);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_draw_texture(window, texture, x, y, width, height);
            break;
    }
}

void pal_draw_texture_ex(PalWindow* window, PalTexture* texture,
                         int x, int y, int width, int height,
                         double rotation, int origin_x, int origin_y,
                         bool flip_h, bool flip_v) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_draw_texture_ex(window, texture, x, y, width, height,
                                    rotation, origin_x, origin_y, flip_h, flip_v);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_draw_texture_ex(window, texture, x, y, width, height,
                                     rotation, origin_x, origin_y, flip_h, flip_v);
            break;
    }
}

void pal_draw_texture_region(PalWindow* window, PalTexture* texture,
                             int src_x, int src_y, int src_w, int src_h,
                             int dst_x, int dst_y, int dst_w, int dst_h) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_draw_texture_region(window, texture, src_x, src_y, src_w, src_h,
                                        dst_x, dst_y, dst_w, dst_h);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_draw_texture_region(window, texture, src_x, src_y, src_w, src_h,
                                         dst_x, dst_y, dst_w, dst_h);
            break;
    }
}

// -----------------------------------------------------------------------------
// Input - Keyboard
// -----------------------------------------------------------------------------

void pal_poll_events(void) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_poll_events();
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_poll_events();
            break;
    }
}

bool pal_should_quit(void) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_should_quit();
#else
            return true;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_should_quit();
    }
    return true;
}

bool pal_key_down(PalKey key) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_key_down(key);
#else
            return false;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_key_down(key);
    }
    return false;
}

bool pal_key_pressed(PalKey key) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_key_pressed(key);
#else
            return false;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_key_pressed(key);
    }
    return false;
}

bool pal_key_released(PalKey key) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_key_released(key);
#else
            return false;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_key_released(key);
    }
    return false;
}

// -----------------------------------------------------------------------------
// Input - Mouse
// -----------------------------------------------------------------------------

void pal_mouse_position(int* x, int* y) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_mouse_position(x, y);
#else
            if (x) *x = 0;
            if (y) *y = 0;
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_mouse_position(x, y);
            break;
    }
}

bool pal_mouse_down(PalMouseButton button) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_mouse_down(button);
#else
            return false;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_mouse_down(button);
    }
    return false;
}

bool pal_mouse_pressed(PalMouseButton button) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_mouse_pressed(button);
#else
            return false;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_mouse_pressed(button);
    }
    return false;
}

bool pal_mouse_released(PalMouseButton button) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_mouse_released(button);
#else
            return false;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_mouse_released(button);
    }
    return false;
}

// -----------------------------------------------------------------------------
// Audio - Sound effects
// -----------------------------------------------------------------------------

PalSound* pal_sound_load(const char* path) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_sound_load(path);
#else
            return NULL;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_sound_load(path);
    }
    return NULL;
}

void pal_sound_destroy(PalSound* sound) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_sound_destroy(sound);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_sound_destroy(sound);
            break;
    }
}

void pal_sound_play(PalSound* sound) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_sound_play(sound);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_sound_play(sound);
            break;
    }
}

void pal_sound_play// Platform Abstraction Layer - Backend dispatcher
// Routes all PAL calls to the active backend (SDL2 or Mock)

#include "pal/pal.h"

#include <stdlib.h>

// -----------------------------------------------------------------------------
// Backend state
// -----------------------------------------------------------------------------

static PalBackend current_backend = PAL_BACKEND_MOCK;
static bool pal_initialized = false;

// -----------------------------------------------------------------------------
// Forward declarations for backend functions
// -----------------------------------------------------------------------------

// Mock backend (always available)
bool pal_mock_init(void);
void pal_mock_quit(void);
PalWindow* pal_mock_window_create(const char* title, int width, int height);
void pal_mock_window_destroy(PalWindow* window);
void pal_mock_window_present(PalWindow* window);
void pal_mock_window_clear(PalWindow* window, uint8_t r, uint8_t g, uint8_t b);
void pal_mock_window_set_title(PalWindow* window, const char* title);
void pal_mock_window_get_size(PalWindow* window, int* width, int* height);
void pal_mock_draw_rect(PalWindow* window, int x, int y, int width, int height,
                        uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void pal_mock_draw_rect_outline(PalWindow* window, int x, int y, int width, int height,
                                uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void pal_mock_draw_line(PalWindow* window, int x1, int y1, int x2, int y2,
                        uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void pal_mock_draw_circle(PalWindow* window, int cx, int cy, int radius,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void pal_mock_draw_circle_outline(PalWindow* window, int cx, int cy, int radius,
                                  uint8_t r, uint8_t g, uint8_t b, uint8_t a);
PalTexture* pal_mock_texture_load(PalWindow* window, const char* path);
void pal_mock_texture_destroy(PalTexture* texture);
void pal_mock_texture_get_size(PalTexture* texture, int* width, int* height);
void pal_mock_draw_texture(PalWindow* window, PalTexture* texture,
                           int x, int y, int width, int height);
void pal_mock_draw_texture_ex(PalWindow* window, PalTexture* texture,
                              int x, int y, int width, int height,
                              double rotation, int origin_x, int origin_y,
                              bool flip_h, bool flip_v);
void pal_mock_draw_texture_region(PalWindow* window, PalTexture* texture,
                                  int src_x, int src_y, int src_w, int src_h,
                                  int dst_x, int dst_y, int dst_w, int dst_h);
void pal_mock_poll_events(void);
bool pal_mock_should_quit(void);
bool pal_mock_key_down(PalKey key);
bool pal_mock_key_pressed(PalKey key);
bool pal_mock_key_released(PalKey key);
void pal_mock_mouse_position(int* x, int* y);
bool pal_mock_mouse_down(PalMouseButton button);
bool pal_mock_mouse_pressed(PalMouseButton button);
bool pal_mock_mouse_released(PalMouseButton button);
PalSound* pal_mock_sound_load(const char* path);
void pal_mock_sound_destroy(PalSound* sound);
void pal_mock_sound_play(PalSound* sound);
void pal_mock_sound_play_volume(PalSound* sound, float volume);
PalMusic* pal_mock_music_load(const char* path);
void pal_mock_music_destroy(PalMusic* music);
void pal_mock_music_play(PalMusic* music, bool loop);
void pal_mock_music_stop(void);
void pal_mock_music_pause(void);
void pal_mock_music_resume(void);
void pal_mock_music_set_volume(float volume);
bool pal_mock_music_is_playing(void);
void pal_mock_set_master_volume(float volume);
double pal_mock_time(void);
void pal_mock_sleep(double seconds);
PalFont* pal_mock_font_load(const char* path, int size);
PalFont* pal_mock_font_default(int size);
void pal_mock_font_destroy(PalFont* font);
void pal_mock_draw_text(PalWindow* window, PalFont* font, const char* text,
                        int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void pal_mock_text_size(PalFont* font, const char* text, int* width, int* height);

#ifdef PAL_USE_SDL2
// SDL2 backend (conditionally available)
bool pal_sdl_init(void);
void pal_sdl_quit(void);
PalWindow* pal_sdl_window_create(const char* title, int width, int height);
void pal_sdl_window_destroy(PalWindow* window);
void pal_sdl_window_present(PalWindow* window);
void pal_sdl_window_clear(PalWindow* window, uint8_t r, uint8_t g, uint8_t b);
void pal_sdl_window_set_title(PalWindow* window, const char* title);
void pal_sdl_window_get_size(PalWindow* window, int* width, int* height);
void pal_sdl_draw_rect(PalWindow* window, int x, int y, int width, int height,
                       uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void pal_sdl_draw_rect_outline(PalWindow* window, int x, int y, int width, int height,
                               uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void pal_sdl_draw_line(PalWindow* window, int x1, int y1, int x2, int y2,
                       uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void pal_sdl_draw_circle(PalWindow* window, int cx, int cy, int radius,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void pal_sdl_draw_circle_outline(PalWindow* window, int cx, int cy, int radius,
                                 uint8_t r, uint8_t g, uint8_t b, uint8_t a);
PalTexture* pal_sdl_texture_load(PalWindow* window, const char* path);
void pal_sdl_texture_destroy(PalTexture* texture);
void pal_sdl_texture_get_size(PalTexture* texture, int* width, int* height);
void pal_sdl_draw_texture(PalWindow* window, PalTexture* texture,
                          int x, int y, int width, int height);
void pal_sdl_draw_texture_ex(PalWindow* window, PalTexture* texture,
                             int x, int y, int width, int height,
                             double rotation, int origin_x, int origin_y,
                             bool flip_h, bool flip_v);
void pal_sdl_draw_texture_region(PalWindow* window, PalTexture* texture,
                                 int src_x, int src_y, int src_w, int src_h,
                                 int dst_x, int dst_y, int dst_w, int dst_h);
void pal_sdl_poll_events(void);
bool pal_sdl_should_quit(void);
bool pal_sdl_key_down(PalKey key);
bool pal_sdl_key_pressed(PalKey key);
bool pal_sdl_key_released(PalKey key);
void pal_sdl_mouse_position(int* x, int* y);
bool pal_sdl_mouse_down(PalMouseButton button);
bool pal_sdl_mouse_pressed(PalMouseButton button);
bool pal_sdl_mouse_released(PalMouseButton button);
PalSound* pal_sdl_sound_load(const char* path);
void pal_sdl_sound_destroy(PalSound* sound);
void pal_sdl_sound_play(PalSound* sound);
void pal_sdl_sound_play_volume(PalSound* sound, float volume);
PalMusic* pal_sdl_music_load(const char* path);
void pal_sdl_music_destroy(PalMusic* music);
void pal_sdl_music_play(PalMusic* music, bool loop);
void pal_sdl_music_stop(void);
void pal_sdl_music_pause(void);
void pal_sdl_music_resume(void);
void pal_sdl_music_set_volume(float volume);
bool pal_sdl_music_is_playing(void);
void pal_sdl_set_master_volume(float volume);
double pal_sdl_time(void);
void pal_sdl_sleep(double seconds);
PalFont* pal_sdl_font_load(const char* path, int size);
PalFont* pal_sdl_font_default(int size);
void pal_sdl_font_destroy(PalFont* font);
void pal_sdl_draw_text(PalWindow* window, PalFont* font, const char* text,
                       int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void pal_sdl_text_size(PalFont* font, const char* text, int* width, int* height);
#endif

// -----------------------------------------------------------------------------
// Backend selection
// -----------------------------------------------------------------------------

bool pal_init(PalBackend backend) {
    if (pal_initialized) {
        pal_quit();
    }

    current_backend = backend;

    bool result = false;
    switch (backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            result = pal_sdl_init();
#else
            result = false;
#endif
            break;
        case PAL_BACKEND_MOCK:
            result = pal_mock_init();
            break;
    }

    pal_initialized = result;
    return result;
}

void pal_quit(void) {
    if (!pal_initialized) return;

    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_quit();
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_quit();
            break;
    }

    pal_initialized = false;
}

PalBackend pal_get_backend(void) {
    return current_backend;
}

// -----------------------------------------------------------------------------
// Window management
// -----------------------------------------------------------------------------

PalWindow* pal_window_create(const char* title, int width, int height) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            return pal_sdl_window_create(title, width, height);
#else
            return NULL;
#endif
        case PAL_BACKEND_MOCK:
            return pal_mock_window_create(title, width, height);
    }
    return NULL;
}

void pal_window_destroy(PalWindow* window) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_window_destroy(window);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_window_destroy(window);
            break;
    }
}

void pal_window_present(PalWindow* window) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_window_present(window);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_window_present(window);
            break;
    }
}

void pal_window_clear(PalWindow* window, uint8_t r, uint8_t g, uint8_t b) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_window_clear(window, r, g, b);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_window_clear(window, r, g, b);
            break;
    }
}

void pal_window_set_title(PalWindow* window, const char* title) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_window_set_title(window, title);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_window_set_title(window, title);
            break;
    }
}

void pal_window_get_size(PalWindow* window, int* width, int* height) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_window_get_size(window, width, height);
#else
            if (width) *width = 0;
            if (height) *height = 0;
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_window_get_size(window, width, height);
            break;
    }
}

// -----------------------------------------------------------------------------
// Rendering primitives
// -----------------------------------------------------------------------------

void pal_draw_rect(PalWindow* window, int x, int y, int width, int height,
                   uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_draw_rect(window, x, y, width, height, r, g, b, a);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_draw_rect(window, x, y, width, height, r, g, b, a);
            break;
    }
}

void pal_draw_rect_outline(PalWindow* window, int x, int y, int width, int height,
                           uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_draw_rect_outline(window, x, y, width, height, r, g, b, a);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_draw_rect_outline(window, x, y, width, height, r, g, b, a);
            break;
    }
}

void pal_draw_line(PalWindow* window, int x1, int y1, int x2, int y2,
                   uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_draw_line(window, x1, y1, x2, y2, r, g, b, a);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_draw_line(window, x1, y1, x2, y2, r, g, b, a);
            break;
    }
}

void pal_draw_circle(PalWindow* window, int cx, int cy, int radius,
                     uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_draw_circle(window, cx, cy, radius, r, g, b, a);
#endif
            break;
        case PAL_BACKEND_MOCK:
            pal_mock_draw_circle(window, cx, cy, radius, r, g, b, a);
            break;
    }
}

void pal_draw_circle_outline(PalWindow* window, int cx, int cy, int radius,
                             uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    switch (current_backend) {
        case PAL_BACKEND_SDL2:
#ifdef PAL_USE_SDL2
            pal_sdl_draw_circle_outline(window, cx, cy, radius, r, g, b, a);
#endif
  