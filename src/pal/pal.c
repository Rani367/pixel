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

void pal_sound_play