---
title: "Understanding the Game Loop"
description: "Learn how Pixel's game loop works: on_start, on_update, and on_draw callbacks. Understand delta time for smooth movement and frame-independent animation."
keywords: ["game loop", "on_start", "on_update", "on_draw", "delta time", "frame rate"]
---

# Understanding the Game Loop

Every Pixel game runs on a **game loop** - a cycle that repeats every frame, updating game logic and drawing graphics. Pixel handles the loop automatically; you just define what happens at each stage.

## The Three Callbacks

Pixel uses three main callback functions:

| Callback | When Called | Purpose |
|----------|-------------|---------|
| `on_start()` | Once at startup | Initialize game state |
| `on_update(dt)` | Every frame | Update game logic |
| `on_draw()` | Every frame, after update | Render graphics |

```pixel
function on_start() {
    create_window(800, 600, "My Game")
    // Initialize your game here
}

function on_update(dt) {
    // Update game logic here
}

function on_draw() {
    clear(BLACK)
    // Draw graphics here
}
```

## on_start()

Called **once** when your game first runs. Use it to:

- Create the game window
- Load images, sounds, and fonts
- Initialize game variables
- Set up initial game state

```pixel
player = none
player_image = none
score = 0

function on_start() {
    create_window(800, 600, "Space Shooter")

    // Load assets
    player_image = load_image("player.png")
    background_music = load_music("music.ogg")

    // Create game objects
    player = create_sprite(player_image)
    player.x = 400
    player.y = 500

    // Start background music
    play_music_loop(background_music)
}
```

## on_update(dt)

Called **every frame** before drawing. Use it for:

- Reading player input
- Moving game objects
- Checking collisions
- Updating game state

The `dt` parameter is **delta time** - the time elapsed since the last frame, in seconds.

```pixel
function on_update(dt) {
    // Move player based on input
    if key_down(KEY_LEFT) {
        player.x = player.x - 200 * dt
    }
    if key_down(KEY_RIGHT) {
        player.x = player.x + 200 * dt
    }

    // Check for collisions
    for enemy in enemies {
        if collides(player, enemy) {
            game_over = true
        }
    }
}
```

## on_draw()

Called **every frame** after `on_update`. Use it to:

- Clear the screen
- Draw backgrounds
- Draw game objects
- Draw UI elements (score, health, etc.)

```pixel
function on_draw() {
    // Always clear first
    clear(rgb(20, 20, 40))

    // Draw game world
    draw_sprite(player)
    for enemy in enemies {
        draw_sprite(enemy)
    }

    // Draw UI on top
    draw_text("Score: " + to_string(score), 20, 20, ui_font, WHITE)
}
```

## Understanding Delta Time (dt)

**Delta time** is crucial for smooth, consistent movement across different computers and frame rates.

### The Problem Without Delta Time

```pixel
// BAD: Movement depends on frame rate
function on_update(dt) {
    player.x = player.x + 5  // Moves 5 pixels per frame
}
```

At 60 FPS, the player moves 300 pixels/second. At 30 FPS, only 150 pixels/second. The game runs differently on different computers.

### The Solution: Multiply by dt

```pixel
// GOOD: Consistent movement regardless of frame rate
function on_update(dt) {
    player.x = player.x + 300 * dt  // Moves 300 pixels per second
}
```

Now the player always moves 300 pixels per second, whether the game runs at 30, 60, or 120 FPS.

### Practical Example

```pixel
speed = 200  // Pixels per second

function on_update(dt) {
    if key_down(KEY_RIGHT) {
        player.x = player.x + speed * dt
    }
    if key_down(KEY_LEFT) {
        player.x = player.x - speed * dt
    }
}
```

Think of it this way:
- `speed` = how fast (pixels per second)
- `dt` = how long this frame lasted (fraction of a second)
- `speed * dt` = how far to move this frame

## Frame Rate

Pixel targets 60 frames per second by default. This means:
- `on_update` runs ~60 times per second
- `on_draw` runs ~60 times per second
- `dt` is approximately 0.0167 (1/60th of a second)

The actual frame rate may vary depending on:
- Computer performance
- Game complexity
- Other running programs

This is why using `dt` is important - it handles these variations automatically.

## Execution Order

Each frame follows this order:

1. **Input Processing** - Pixel updates keyboard/mouse state
2. **on_update(dt)** - Your game logic runs
3. **Scene on_update(dt)** - Current scene's update runs (if using scenes)
4. **on_draw()** - Your drawing code runs
5. **Scene on_draw()** - Current scene's draw runs (if using scenes)
6. **Display** - The frame appears on screen

## Common Patterns

### Timer

```pixel
spawn_timer = 0
spawn_interval = 2.0  // Seconds

function on_update(dt) {
    spawn_timer = spawn_timer + dt

    if spawn_timer >= spawn_interval {
        spawn_enemy()
        spawn_timer = 0
    }
}
```

### Animation Based on Time

```pixel
function on_update(dt) {
    // Bobbing motion using game time
    bob_offset = sin(game_time() * 3) * 10
}

function on_draw() {
    draw_sprite_at(player, player.x, player.y + bob_offset)
}
```

### State-Based Updates

```pixel
game_state = "playing"

function on_update(dt) {
    if game_state == "playing" {
        update_gameplay(dt)
    } else if game_state == "paused" {
        if key_pressed(KEY_P) {
            game_state = "playing"
        }
    } else if game_state == "gameover" {
        if key_pressed(KEY_RETURN) {
            restart_game()
        }
    }
}
```

## Tips

1. **Always use dt for movement** - Don't use fixed values for position changes

2. **Clear at the start of on_draw** - Always call `clear()` first

3. **Don't draw in on_update** - Keep drawing in `on_draw()`

4. **Don't update state in on_draw** - Keep logic in `on_update()`

5. **Load assets in on_start** - Loading files during gameplay causes stutters

## See Also

- [Getting Started](/pixel/docs/getting-started) - Create your first game
- [Scenes API](/pixel/docs/api/scenes) - Scene-specific callbacks
