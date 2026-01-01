---
title: "Scene Management API Reference"
description: "Organize your Pixel game with scenes for menus, levels, and game states. Learn to switch between scenes and use scene-specific callbacks."
keywords: ["Pixel scenes", "game states", "menu system", "level management", "scene switching"]
---

# Scene Management API Reference

Scenes help organize different parts of your game - menus, levels, pause screens, etc. Each scene can have its own `on_start`, `on_update`, and `on_draw` callbacks.

## Scene Functions

### load_scene(name)
Loads a new scene by name. This calls `{name}_on_start()` if it exists.

```pixel
load_scene("menu")      // Calls menu_on_start()
load_scene("level1")    // Calls level1_on_start()
load_scene("gameover")  // Calls gameover_on_start()
```

### get_scene()
Returns the name of the current scene.

```pixel
if get_scene() == "game" {
    // We're in the game scene
}
```

## Scene Callbacks

Each scene can define its own callbacks by prefixing the function name with the scene name:

| Callback | When Called |
|----------|-------------|
| `{scene}_on_start()` | Once when scene loads |
| `{scene}_on_update(dt)` | Every frame |
| `{scene}_on_draw()` | Every frame after update |

```pixel
// Menu scene callbacks
function menu_on_start() {
    println("Menu loaded")
}

function menu_on_update(dt) {
    if key_pressed(KEY_RETURN) {
        load_scene("game")
    }
}

function menu_on_draw() {
    clear(rgb(20, 20, 40))
    draw_text("Press ENTER to Start", 250, 280, default_font(32), WHITE)
}

// Game scene callbacks
function game_on_start() {
    println("Game started")
    reset_game()
}

function game_on_update(dt) {
    update_player(dt)
    update_enemies(dt)
}

function game_on_draw() {
    clear(rgb(100, 150, 200))
    draw_game()
}
```

## Global vs Scene Callbacks

The global callbacks (`on_start`, `on_update`, `on_draw`) work alongside scene callbacks:

1. Global `on_start()` runs once when the game first starts
2. When a scene loads, `{scene}_on_start()` runs
3. Each frame: global `on_update(dt)`, then `{scene}_on_update(dt)`
4. Each frame: global `on_draw()`, then `{scene}_on_draw()`

```pixel
// This always runs, regardless of scene
function on_update(dt) {
    // Handle global pause key
    if key_pressed(KEY_ESCAPE) {
        if get_scene() == "game" {
            load_scene("pause")
        }
    }
}
```

## Complete Scene Example

```pixel
// ============ SHARED STATE ============
player_x = 400
player_y = 300
score = 0
high_score = 0

// ============ GLOBAL ============
function on_start() {
    create_window(800, 600, "Scene Demo")
    load_scene("menu")
}

// ============ MENU SCENE ============
function menu_on_update(dt) {
    if key_pressed(KEY_RETURN) {
        load_scene("game")
    }
}

function menu_on_draw() {
    clear(rgb(20, 20, 60))
    draw_text("MY GAME", 320, 200, default_font(48), WHITE)
    draw_text("Press ENTER to Play", 270, 300, default_font(24), GRAY)
    draw_text("High Score: " + to_string(high_score), 300, 400, default_font(24), YELLOW)
}

// ============ GAME SCENE ============
function game_on_start() {
    // Reset game state
    player_x = 400
    player_y = 300
    score = 0
}

function game_on_update(dt) {
    // Movement
    if key_down(KEY_LEFT)  { player_x -= 200 * dt }
    if key_down(KEY_RIGHT) { player_x += 200 * dt }
    if key_down(KEY_UP)    { player_y -= 200 * dt }
    if key_down(KEY_DOWN)  { player_y += 200 * dt }

    // Pause
    if key_pressed(KEY_ESCAPE) {
        load_scene("pause")
    }

    // Simulate game over
    if key_pressed(KEY_G) {
        if score > high_score {
            high_score = score
        }
        load_scene("gameover")
    }

    // Score
    score += 1
}

function game_on_draw() {
    clear(rgb(100, 150, 200))
    draw_circle(player_x, player_y, 20, CYAN)
    draw_text("Score: " + to_string(score), 20, 20, default_font(24), WHITE)
    draw_text("Press G for Game Over, ESC to Pause", 20, 560, default_font(16), GRAY)
}

// ============ PAUSE SCENE ============
function pause_on_update(dt) {
    if key_pressed(KEY_ESCAPE) {
        load_scene("game")
    }
    if key_pressed(KEY_Q) {
        load_scene("menu")
    }
}

function pause_on_draw() {
    // Draw game in background (frozen)
    clear(rgb(100, 150, 200))
    draw_circle(player_x, player_y, 20, CYAN)

    // Draw pause overlay
    draw_rect(0, 0, 800, 600, rgba(0, 0, 0, 150))
    draw_text("PAUSED", 330, 250, default_font(48), WHITE)
    draw_text("Press ESC to Resume", 290, 320, default_font(24), GRAY)
    draw_text("Press Q to Quit", 310, 360, default_font(24), GRAY)
}

// ============ GAME OVER SCENE ============
function gameover_on_update(dt) {
    if key_pressed(KEY_RETURN) {
        load_scene("game")
    }
    if key_pressed(KEY_ESCAPE) {
        load_scene("menu")
    }
}

function gameover_on_draw() {
    clear(rgb(60, 20, 20))
    draw_text("GAME OVER", 280, 200, default_font(48), RED)
    draw_text("Score: " + to_string(score), 340, 280, default_font(32), WHITE)

    if score >= high_score {
        draw_text("NEW HIGH SCORE!", 290, 340, default_font(24), YELLOW)
    }

    draw_text("Press ENTER to Retry", 280, 420, default_font(24), GRAY)
    draw_text("Press ESC for Menu", 290, 460, default_font(24), GRAY)
}
```

## Scene Best Practices

1. **Initialize state in scene's on_start** - Reset game variables when entering a scene

2. **Use global on_update for shared logic** - Things like global pause or volume controls

3. **Keep scene names simple** - Use lowercase names like `"menu"`, `"game"`, `"level1"`

4. **Persist data in global variables** - High scores, settings, etc. survive scene changes

5. **Call load_scene in on_update, not on_draw** - Scene loading should happen during logic, not rendering

## Common Scene Patterns

### Level Progression

```pixel
current_level = 1

function next_level() {
    current_level += 1
    load_scene("level" + to_string(current_level))
}

function level1_on_start() {
    setup_level(1)
}

function level2_on_start() {
    setup_level(2)
}
```

### Transition State

```pixel
transitioning = false
transition_timer = 0

function start_transition(target_scene) {
    transitioning = true
    transition_timer = 0.5
    next_scene = target_scene
}

function on_update(dt) {
    if transitioning {
        transition_timer -= dt
        if transition_timer <= 0 {
            transitioning = false
            load_scene(next_scene)
        }
    }
}
```

## See Also

- [Functions](/pixel/docs/language/functions) - Defining functions and callbacks
- [Engine API](/pixel/docs/api/engine) - Window and drawing
