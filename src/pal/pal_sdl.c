int8_t a) {
    if (!window || !window->sdl_renderer) return;

#ifdef __EMSCRIPTEN__
    // For Emscripten, SDL_RenderFillRect doesn't work properly
    // Use horizontal lines to fill the rectangle instead
    SDL_SetRenderDrawColor(window->sdl_renderer, r, g, b, a);
    for (int dy = 0; dy < h; dy++) {
        SDL_RenderDrawLine(window->sdl_renderer, x, y + dy, x + w - 1, y + dy);
    }
#else
    // Set blend mode based on alpha
    if (a == 255) {
        SDL_SetRenderDrawBlendMode(window->sdl_renderer, SDL_BLENDMODE_NONE);
    } else {
        SDL_SetRenderDrawBlendMode(window->sdl_renderer, SDL_BLENDMODE_BLEND);
    }
    SDL_SetRenderDrawColor(window->sdl_renderer, r, g, b, a);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderFillRect(window->sdl_renderer, &rect);
#endif
}

void pal_sdl_draw_rect_outline(PalWindow* window, int x, int y, int w, int h,
                               uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!window || !window->sdl_renderer) return;

    SDL_SetRenderDrawColor(window->sdl_renderer, r, g, b, a);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderDrawRect(window->sdl_renderer, &rect);
}

void pal_sdl_draw_line(PalWindow* window, int x1, int y1, int x2, int y2,
                       uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!window || !window->sdl_renderer) return;

    SDL_SetRenderDrawColor(window->sdl_renderer, r, g, b, a);
    SDL_RenderDrawLine(window->sdl_renderer, x1, y1, x2, y2);
}

// Simple circle drawing - draw multiple horizontal lines
void pal_sdl_draw_circle(PalWindow* window, int cx, int cy, int radius,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!window || !window->sdl_renderer) return;

#ifdef __EMSCRIPTEN__
    // For Emscripten, draw using lines instead of filled rect
    SDL_SetRenderDrawColor(window->sdl_renderer, r, g, b, a);

    // Draw horizontal lines to fill the circle area
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)SDL_sqrt((double)(radius * radius - dy * dy));
        SDL_RenderDrawLine(window->sdl_renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
#else
    // For native, use filled rectangle as approximation
    pal_sdl_draw_rect(window, cx - radius, cy - radius, radius * 2, radius * 2, r, g, b, a);
#endif
}

void pal_sdl_draw_circle_outline(PalWindow* window, int cx, int cy, int radius,
                                 uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!window || !window->sdl_renderer) return;

    SDL_SetRenderDrawColor(window->sdl_renderer, r, g, b, a);

    // Midpoint circle algorithm
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        SDL_RenderDrawPoint(window->sdl_renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(window->sdl_renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(window->sdl_renderer, cx - y, cy + x);
        SDL_RenderDrawPoint(window->sdl_renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(window->sdl_renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(window->sdl_renderer, cx - y, cy - x);
        SDL_RenderDrawPoint(window->sdl_renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(window->sdl_renderer, cx + x, cy - y);

        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

// -----------------------------------------------------------------------------
// Textures
// -----------------------------------------------------------------------------

PalTexture* pal_sdl_texture_load(PalWindow* window, const char* path) {
    if (!window || !window->sdl_renderer || !path) return NULL;

    SDL_Surface* surface = IMG_Load(path);
    if (!surface) return NULL;

    SDL_Texture* sdl_texture = SDL_CreateTextureFromSurface(window->sdl_renderer, surface);
    int w = surface->w;
    int h = surface->h;
    SDL_FreeSurface(surface);

    if (!sdl_texture) return NULL;

    PalTexture* texture = malloc(sizeof(PalTexture));
    if (!texture) {
        SDL_DestroyTexture(sdl_texture);
        return NULL;
    }

    texture->sdl_texture = sdl_texture;
    texture->width = w;
    texture->height = h;

    return texture;
}

void pal_sdl_texture_destroy(PalTexture* texture) {
    if (!texture) return;
    if (texture->sdl_texture) SDL_DestroyTexture(texture->sdl_texture);
    free(texture);
}

void pal_sdl_texture_get_size(PalTexture* texture, int* width, int* height) {
    if (texture) {
        if (width) *width = texture->width;
        if (height) *height = texture->height;
    } else {
        if (width) *width = 0;
        if (height) *height = 0;
    }
}

void pal_sdl_draw_texture(PalWindow* window, PalTexture* texture,
                          int x, int y, int width, int height) {
    if (!window || !window->sdl_renderer || !texture || !texture->sdl_texture) return;

    SDL_Rect dst = { x, y, width, height };
    SDL_RenderCopy(window->sdl_renderer, texture->sdl_texture, NULL, &dst);
}

void pal_sdl_draw_texture_ex(PalWindow* window, PalTexture* texture,
                             int x, int y, int width, int height,
                             double rotation, int origin_x, int origin_y,
                             bool flip_h, bool flip_v) {
    if (!window || !window->sdl_renderer || !texture || !texture->sdl_texture) return;

    SDL_Rect dst = { x, y, width, height };
    SDL_Point center = { origin_x, origin_y };
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (flip_h) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
    if (flip_v) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);

    SDL_RenderCopyEx(window->sdl_renderer, texture->sdl_texture, NULL, &dst,
                     rotation, &center, flip);
}

void pal_sdl_draw_texture_region(PalWindow* window, PalTexture* texture,
                                 int src_x, int src_y, int src_w, int src_h,
                                 int dst_x, int dst_y, int dst_w, int dst_h) {
    if (!window || !window->sdl_renderer || !texture || !texture->sdl_texture) return;

    SDL_Rect src = { src_x, src_y, src_w, src_h };
    SDL_Rect dst = { dst_x, dst_y, dst_w, dst_h };
    SDL_RenderCopy(window->sdl_renderer, texture->sdl_texture, &src, &dst);
}

// -----------------------------------------------------------------------------
// Input - Keyboard
// -----------------------------------------------------------------------------

void pal_sdl_poll_events(void) {
#ifdef __EMSCRIPTEN__
    // Minimal event polling for Emscripten
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            sdl_quit_requested = true;
        }
    }
#else
    // Refresh keyboard state pointer
    sdl_keyboard_state = SDL_GetKeyboardState(NULL);

    // Save previous state
    if (sdl_keyboard_state) {
        for (int i = 0; i < PAL_KEY_COUNT; i++) {
            sdl_keys_prev[i] = sdl_keyboard_state[i];
        }
    }
    sdl_mouse_prev = sdl_mouse_state;

    // Process events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            sdl_quit_requested = true;
        }
    }

    // Update mouse state
    sdl_mouse_state = SDL_GetMouseState(&sdl_mouse_x, &sdl_mouse_y);
#endif
}

bool pal_sdl_should_quit(void) {
    return sdl_quit_requested;
}

bool pal_sdl_key_down(PalKey key) {
    if (!sdl_keyboard_state || key < 0 || key >= PAL_KEY_COUNT) return false;
    return sdl_keyboard_state[key] != 0;
}

bool pal_sdl_key_pressed(PalKey key) {
    if (!sdl_keyboard_state || key < 0 || key >= PAL_KEY_COUNT) return false;
    return sdl_keyboard_state[key] && !sdl_keys_prev[key];
}

bool pal_sdl_key_released(PalKey key) {
    if (!sdl_keyboard_state || key < 0 || key >= PAL_KEY_COUNT) return false;
    return !sdl_keyboard_state[key] && sdl_keys_prev[key];
}

// -----------------------------------------------------------------------------
// Input - Mouse
// -----------------------------------------------------------------------------

void pal_sdl_mouse_position(int* x, int* y) {
    if (x) *x = sdl_mouse_x;
    if (y) *y = sdl_mouse_y;
}

static Uint32 get_sdl_mouse_button(PalMouseButton button) {
    switch (button) {
        case PAL_MOUSE_LEFT: return SDL_BUTTON_LMASK;
        case PAL_MOUSE_MIDDLE: return SDL_BUTTON_MMASK;
        case PAL_MOUSE_RIGHT: return SDL_BUTTON_RMASK;
        default: return 0;
    }
}

bool pal_sdl_mouse_down(PalMouseButton button) {
    return (sdl_mouse_state & get_sdl_mouse_button(button)) != 0;
}

bool pal_sdl_mouse_pressed(PalMouseButton button) {
    Uint32 mask = get_sdl_mouse_button(button);
    return (sdl_mouse_state & mask) && !(sdl_mouse_prev & mask);
}

bool pal_sdl_mouse_released(PalMouseButton button) {
    Uint32 mask = get_sdl_mouse_button(button);
    return !(sdl_mouse_state & mask) && (sdl_mouse_prev & mask);
}

// -----------------------------------------------------------------------------
// Audio - Sound effects
// -----------------------------------------------------------------------------

PalSound* pal_sdl_sound_load(const char* path) {
    if (!path) return NULL;

    Mix_Chunk* chunk = Mix_LoadWAV(path);
    if (!chunk) return NULL;

    PalSound* sound = malloc(sizeof(PalSound));
    if (!sound) {
        Mix_FreeChunk(chunk);
        return NULL;
    }

    sound->chunk = chunk;
    return sound;
}

void pal_sdl_sound_destroy(PalSound* sound) {
    if (!sound) return;
    if (sound->chunk) Mix_FreeChunk(sound->chunk);
    free(sound);
}

void pal_sdl_sound_play(PalSound* sound) {
    if (sound && sound->chunk) {
        Mix_PlayChannel(-1, sound->chunk, 0);
    }
}

void pal_sdl_sound_play_volume(PalSound* sound, float volume) {
    if (sound && sound->chunk) {
        int v = (int)(volume * MIX_MAX_VOLUME);
        if (v < 0) v = 0;
        if (v > MIX_MAX_VOLUME) v = MIX_MAX_VOLUME;
        Mix_VolumeChunk(sound->chunk, v);
        Mix_PlayChannel(-1, sound->chunk, 0);
    }
}

// -----------------------------------------------------------------------------
// Audio - Music
// -----------------------------------------------------------------------------

PalMusic* pal_sdl_music_load(const char* path) {
    if (!path) return NULL;

    Mix_Music* music = Mix_LoadMUS(path);
    if (!music) return NULL;

    PalMusic* pal_music = malloc(sizeof(PalMusic));
    if (!pal_music) {
        Mix_FreeMusic(music);
        return NULL;
    }

    pal_music->music = music;
    return pal_music;
}

void pal_sdl_music_destroy(PalMusic* music) {
    if (!music) return;
    if (music->music) Mix_FreeMusic(music->music);
    free(music);
}

void pal_sdl_music_play(PalMusic* music, bool loop) {
    if (music && music->music) {
        Mix_PlayMusic(music->music, loop ? -1 : 1);
    }
}

void pal_sdl_music_stop(void) {
    Mix_HaltMusic();
}

void pal_sdl_music_pause(void) {
    Mix_PauseMusic();
}

void pal_sdl_music_resume(void) {
    Mix_ResumeMusic();
}

void pal_sdl_music_set_volume(float volume) {
    int v = (int)(volume * MIX_MAX_VOLUME);
    if (v < 0) v = 0;
    if (v > MIX_MAX_VOLUME) v = MIX_MAX_VOLUME;
    Mix_VolumeMusic(v);
}

bool pal_sdl_music_is_playing(void) {
    return Mix_PlayingMusic() != 0;
}

// -----------------------------------------------------------------------------
// Audio - Master volume
// -----------------------------------------------------------------------------

void pal_sdl_set_master_volume(float volume) {
    int v = (int)(volume * MIX_MAX_VOLUME);
    if (v < 0) v = 0;
    if (v > MIX_MAX_VOLUME) v = MIX_MAX_VOLUME;
    Mix_Volume(-1, v);    // All sound channels
    Mix_VolumeMusic(v);   // Music channel
}

// -----------------------------------------------------------------------------
// Time
// -----------------------------------------------------------------------------

double pal_sdl_time(void) {
    Uint64 now = SDL_GetPerformanceCounter();
    return (double)(now - sdl_start_time) / (double)sdl_frequency;
}

void pal_sdl_sleep(double seconds) {
    if (seconds > 0) {
        SDL_Delay((Uint32)(seconds * 1000.0));
    }
}

// -----------------------------------------------------------------------------
// Fonts and Text (stub implementation - SDL_ttf support to be added later)
// -----------------------------------------------------------------------------

struct PalFont {
    int size;
    bool is_default;
};

PalFont* pal_sdl_font_load(const char* path, int size) {
    (void)path;
    // TODO: Implement with SDL_ttf
    PalFont* font = malloc(sizeof(PalFont));
    if (!font) return NULL;
    font->size = size;
    font->is_default = false;
    return font;
}

PalFont* pal_sdl_font_default(int size) {
    // TODO: Implement with embedded font
    PalFont* font = malloc(sizeof(PalFont));
    if (!font) return NULL;
    font->size = size;
    font->is_default = true;
    return font;
}

void pal_sdl_font_destroy(PalFont* font) {
    free(font);
}

void pal_sdl_draw_text(PalWindow* window, PalFont* font, const char* text,
                       int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    (void)window; (void)font; (void)text;
    (void)x; (void)y; (void)r; (void)g; (void)b; (void)a;
    // TODO: Implement with SDL_ttf
    // For now, no-op since we don't have font rendering
}

void pal_sdl_text_size(PalFont* font, const char* text, int* width, int* height) {
    // Approximate text size without SDL_ttf
    int len = text ? (int)strlen(text) : 0;
    int char_width = font ? (font->size / 2) : 8;
    int char_height = font ? font->size : 16;

    if (width) *width = len * char_width;
    if (height) *height = char_height;
}

#endif // PAL_USE_SDL2
