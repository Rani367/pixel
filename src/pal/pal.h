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
// -------------------------------------------------------