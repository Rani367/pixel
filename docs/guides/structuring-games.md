---
title: "Structuring Larger Games"
description: "Organize your Pixel code for larger projects. Learn patterns for managing game entities, state, and complexity as your game grows."
keywords: ["game architecture", "code organization", "entity management", "game state", "best practices"]
---

# Structuring Larger Games

As your Pixel games grow beyond simple prototypes, good organization becomes important. This guide covers patterns for keeping your code manageable.

## Game State Object

Instead of scattered global variables, group related state into a struct:

```pixel
// Instead of this:
player_x = 0
player_y = 0
player_health = 100
score = 0
level = 1
game_over = false

// Use this:
game = {}

function on_start() {
    create_window(800, 600, "My Game")
    init_game()
}

function init_game() {
    game.player = {x: 400, y: 300, health: 100}
    game.score = 0
    game.level = 1
    game.over = false
    game.enemies = []
    game.bullets = []
}
```

Benefits:
- Easy to reset the entire game state
- Clear what belongs to the game
- Reduces global namespace clutter

## Entity Pattern

For managing multiple similar objects (enemies, bullets, coins), use a list of structs:

```pixel
enemies = []

function create_enemy(x, y, enemy_type) {
    enemy = {}
    enemy.x = x
    enemy.y = y
    enemy.type = enemy_type
    enemy.health = 30
    enemy.speed = 100
    enemy.active = true

    if enemy_type == "fast" {
        enemy.speed = 200
        enemy.health = 15
    } else if enemy_type == "tank" {
        enemy.speed = 50
        enemy.health = 100
    }

    push(enemies, enemy)
    return enemy
}

function update_enemies(dt) {
    // Iterate backwards for safe removal
    i = len(enemies) - 1
    while i >= 0 {
        enemy = enemies[i]

        if enemy.active {
            update_enemy(enemy, dt)
        }

        if not enemy.active or enemy.health <= 0 {
            remove(enemies, i)
        }

        i -= 1
    }
}

function update_enemy(enemy, dt) {
    // Move toward player
    dx = game.player.x - enemy.x
    dy = game.player.y - enemy.y
    dist = sqrt(dx * dx + dy * dy)

    if dist > 0 {
        enemy.x += (dx / dist) * enemy.speed * dt
        enemy.y += (dy / dist) * enemy.speed * dt
    }
}
```

## Factory Functions

Create helper functions that set up entities with consistent properties:

```pixel
function create_bullet(x, y, angle, speed) {
    bullet = {}
    bullet.x = x
    bullet.y = y
    bullet.vel_x = cos(angle) * speed
    bullet.vel_y = sin(angle) * speed
    bullet.damage = 10
    bullet.lifetime = 2.0
    bullet.active = true
    push(game.bullets, bullet)
    return bullet
}

function create_particle_burst(x, y, color, count) {
    emitter = create_emitter(x, y)
    emitter_set_color(emitter, color)
    emitter_set_size(emitter, 2, 5)
    emitter_set_speed(emitter, 50, 150)
    emitter_set_lifetime(emitter, 0.3, 0.6)
    emitter_set_angle(emitter, 0, 360)
    emitter_emit(emitter, count)
    return emitter
}
```

## Update and Draw Separation

Organize your main callbacks to delegate to focused functions:

```pixel
function on_update(dt) {
    if game.over {
        update_game_over(dt)
        return
    }

    update_player(dt)
    update_enemies(dt)
    update_bullets(dt)
    update_collisions()
    update_spawning(dt)
}

function on_draw() {
    clear(rgb(20, 20, 40))

    draw_background()
    draw_bullets()
    draw_enemies()
    draw_player()
    draw_particles()
    draw_ui()

    if game.over {
        draw_game_over()
    }
}
```

## State Machine for Game Flow

Use a state machine for managing different game states:

```pixel
game_state = "menu"

function on_update(dt) {
    if game_state == "menu" {
        update_menu(dt)
    } else if game_state == "playing" {
        update_playing(dt)
    } else if game_state == "paused" {
        update_paused(dt)
    } else if game_state == "game_over" {
        update_game_over(dt)
    }
}

function update_menu(dt) {
    if key_pressed(KEY_RETURN) {
        game_state = "playing"
        init_game()
    }
}

function update_playing(dt) {
    if key_pressed(KEY_ESCAPE) {
        game_state = "paused"
        return
    }

    // Normal game update...
    update_player(dt)
    update_enemies(dt)

    if game.player.health <= 0 {
        game_state = "game_over"
    }
}

function update_paused(dt) {
    if key_pressed(KEY_ESCAPE) {
        game_state = "playing"
    }
    if key_pressed(KEY_Q) {
        game_state = "menu"
    }
}
```

## Using Scenes for Organization

For larger games, use Pixel's scene system:

```pixel
// game.pixel - Main file

function on_start() {
    create_window(800, 600, "My Game")
    load_assets()
    load_scene("menu")
}

function load_assets() {
    // Load all assets once at startup
    assets.player_img = load_image("player.png")
    assets.enemy_img = load_image("enemy.png")
    assets.font = default_font(24)
    assets.music = load_music("background.ogg")
}

// Menu scene
function menu_on_start() {
    play_music_loop(assets.music)
}

function menu_on_update(dt) {
    if key_pressed(KEY_RETURN) {
        load_scene("game")
    }
}

function menu_on_draw() {
    clear(rgb(20, 20, 60))
    draw_text("Press ENTER to Start", 250, 300, assets.font, WHITE)
}

// Game scene
function game_on_start() {
    init_game()
}

function game_on_update(dt) {
    update_gameplay(dt)
}

function game_on_draw() {
    draw_gameplay()
}
```

## Component-Style Organization

For complex entities, you can organize properties into logical groups:

```pixel
function create_player(x, y) {
    p = {}

    // Position component
    p.x = x
    p.y = y

    // Physics component
    p.vel_x = 0
    p.vel_y = 0
    p.speed = 200
    p.grounded = false

    // Combat component
    p.health = 100
    p.max_health = 100
    p.damage = 10
    p.invincible = false
    p.invincible_timer = 0

    // Animation component
    p.sprite = create_sprite(assets.player_img)
    p.facing_right = true

    return p
}

function update_player_physics(player, dt) {
    // Apply velocity
    player.x += player.vel_x * dt
    player.y += player.vel_y * dt

    // Apply gravity
    if not player.grounded {
        player.vel_y += GRAVITY * dt
    }
}

function update_player_combat(player, dt) {
    // Update invincibility
    if player.invincible {
        player.invincible_timer -= dt
        if player.invincible_timer <= 0 {
            player.invincible = false
        }
    }
}

function update_player(dt) {
    update_player_input(game.player, dt)
    update_player_physics(game.player, dt)
    update_player_combat(game.player, dt)
    update_player_animation(game.player, dt)
}
```

## Configuration Constants

Define constants at the top of your file for easy tuning:

```pixel
// Game configuration
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600

// Player settings
PLAYER_SPEED = 200
PLAYER_JUMP_FORCE = 400
PLAYER_MAX_HEALTH = 100

// Enemy settings
ENEMY_SPAWN_RATE = 2.0
ENEMY_BASE_SPEED = 100
MAX_ENEMIES = 20

// Physics
GRAVITY = 980

function on_start() {
    create_window(SCREEN_WIDTH, SCREEN_HEIGHT, "My Game")
    // ...
}
```

## Level Data Structure

For level-based games, store level data in structs:

```pixel
levels = []

function load_levels() {
    // Level 1
    level1 = {}
    level1.name = "Forest"
    level1.spawn_rate = 2.0
    level1.enemy_count = 10
    level1.enemy_types = ["basic"]
    level1.background_color = rgb(34, 139, 34)
    push(levels, level1)

    // Level 2
    level2 = {}
    level2.name = "Cave"
    level2.spawn_rate = 1.5
    level2.enemy_count = 15
    level2.enemy_types = ["basic", "fast"]
    level2.background_color = rgb(50, 50, 70)
    push(levels, level2)

    // Level 3
    level3 = {}
    level3.name = "Castle"
    level3.spawn_rate = 1.0
    level3.enemy_count = 20
    level3.enemy_types = ["basic", "fast", "tank"]
    level3.background_color = rgb(80, 60, 60)
    push(levels, level3)
}

function start_level(level_num) {
    level = levels[level_num]
    game.current_level = level_num
    game.spawn_rate = level.spawn_rate
    game.enemies_remaining = level.enemy_count
    game.bg_color = level.background_color
}
```

## Tips for Maintainable Code

1. **One job per function** - Each function should do one thing well

2. **Consistent naming** - Use patterns like `update_X()`, `draw_X()`, `create_X()`

3. **Group related code** - Keep player functions together, enemy functions together, etc.

4. **Comment sections** - Use comments to separate logical sections:
   ```pixel
   // ========== PLAYER ==========

   function update_player(dt) { }
   function draw_player() { }

   // ========== ENEMIES ==========

   function update_enemies(dt) { }
   function draw_enemies() { }
   ```

5. **Start simple** - Don't over-engineer. Add structure as needed.

## See Also

- [Scenes API](/pixel/docs/api/scenes) - Using scenes for organization
- [Game Loop Guide](/pixel/docs/guides/game-loop) - Understanding the game loop
