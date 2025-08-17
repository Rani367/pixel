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
