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
