# Audio API Reference

## Sound Effects

Sound effects are short audio clips that play immediately and can overlap.

### load_sound(path)
Loads a sound effect from file. Supports WAV, OGG, MP3.

```pixel
jump_sound = none
coin_sound = none

function on_start() {
    create_window(800, 600, "Audio Demo")
    jump_sound = load_sound("sounds/jump.wav")
    coin_sound = load_sound("sounds/coin.ogg")
}
```

### play_sound(sound)
Plays a sound effect at full volume.

```pixel
if key_pressed(KEY_SPACE) {
    play_sound(jump_sound)
}
```

### play_sound_volume(sound, volume)
Plays a sound effect at specified volume (0.0 to 1.0).

```pixel
play_sound_volume(explosion, 0.5)  // Half volume
```

## Music

Music is for longer background audio. Only one music track plays at a time.

### load_music(path)
Loads a music file. Supports WAV, OGG, MP3.

```pixel
background_music = load_music("music/level1.ogg")
```

### play_music(music)
Plays music once.

```pixel
play_music(victory_theme)
```

### play_music_loop(music)
Plays music in a loop.

```pixel
function on_start() {
    create_window(800, 600, "Game")
    bgm = load_music("music/background.ogg")
    play_music_loop(bgm)
}
```

### pause_music()
Pauses the current music.

```pixel
function pause_game() {
    paused = true
    pause_music()
}
```

### resume_music()
Resumes paused music.

```pixel
function unpause_game() {
    paused = false
    resume_music()
}
```

### stop_music()
Stops the current music completely.

```pixel
function game_over() {
    stop_music()
    play_music(gameover_theme)
}
```

### music_playing()
Returns `true` if music is currently playing.

```pixel
if not music_playing() {
    play_music_loop(next_track)
}
```

## Volume Control

### set_music_volume(volume)
Sets the music volume (0.0 to 1.0).

```pixel
set_music_volume(0.7)  // 70% volume
```

### set_master_volume(volume)
Sets the master volume for all audio (0.0 to 1.0).

```pixel
set_master_volume(0.5)  // Half volume for everything
```

## Common Patterns

### Audio Manager

```pixel
sfx_volume = 1.0
music_volume = 0.8

function play_sfx(sound) {
    play_sound_volume(sound, sfx_volume)
}

function set_sfx_volume(vol) {
    sfx_volume = vol
}

function set_bgm_volume(vol) {
    music_volume = vol
    set_music_volume(vol)
}
```

### Scene-Based Music

```pixel
current_music = none

function change_scene(name) {
    if name == "menu" {
        if current_music != menu_music {
            stop_music()
            current_music = menu_music
            play_music_loop(menu_music)
        }
    }
    if name == "game" {
        stop_music()
        current_music = game_music
        play_music_loop(game_music)
    }
    load_scene(name)
}
```

### Sound with Cooldown

```pixel
shoot_sound = none
shoot_cooldown = 0

function on_start() {
    shoot_sound = load_sound("sounds/shoot.wav")
}

function on_update(dt) {
    shoot_cooldown = shoot_cooldown - dt

    if mouse_down(MOUSE_LEFT) and shoot_cooldown <= 0 {
        shoot()
        play_sound(shoot_sound)
        shoot_cooldown = 0.1  // 100ms between sounds
    }
}
```

### Randomized Sounds

```pixel
hit_sounds = []

function on_start() {
    push(hit_sounds, load_sound("sounds/hit1.wav"))
    push(hit_sounds, load_sound("sounds/hit2.wav"))
    push(hit_sounds, load_sound("sounds/hit3.wav"))
}

function play_hit_sound() {
    index = floor(random() * len(hit_sounds))
    play_sound(hit_sounds[index])
}
```

## Supported Formats

| Format | Extension | Notes |
|--------|-----------|-------|
| WAV | `.wav` | Uncompressed, best for short sounds |
| OGG Vorbis | `.ogg` | Compressed, good for music |
| MP3 | `.mp3` | Compressed, widely supported |

## Best Practices

1. **Load audio in on_start()**: Loading files takes time, do it once at startup
2. **Use WAV for sound effects**: Lower latency for short sounds
3. **Use OGG for music**: Good compression for longer tracks
4. **Set reasonable volumes**: Start at 0.7-0.8 to leave headroom
5. **Handle missing files**: The game continues if audio fails to load
