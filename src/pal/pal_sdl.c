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
 