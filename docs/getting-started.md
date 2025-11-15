e` - size multiplier
- `flip_x`, `flip_y` - mirror the sprite

## Adding Collision

Pixel provides simple collision detection using sprites:

```pixel
player = none
coin = none
score = 0

function on_start() {
    create_window(800, 600, "Collision Demo")

    // Create sprites (can be used without images for collision)
    player = create_sprite(none)
    player.x = 100
    player.y = 300
    player.width = 40
    player.height = 40

    coin = create_sprite(none)
    coin.x = 600
    coin.y = 300
    coin.width = 30
    coin.height = 30
}

function on_update(dt) {
    // Move player
    if key_down(KEY_RIGHT) {
        player.x = player.x + 200 * dt
    }

    // Check collision between sprites
    if collides(player, coin) {
        score = score + 1
        coin.x = random_range(50, 750)
        coin.y = random_range(50, 550)
    }
}

function on_draw() {
    clear(rgb(20, 20, 40))
    draw_rect(player.x, player.y, player.width, player.height, BLUE)
    draw_circle(coin.x + 15, coin.y + 15, 15, YELLOW)
    draw_text("Score: " + to_string(score), 20, 20, default_font(24), WHITE)
}
```

## Core Callbacks

Pixel games use three main callback functions:

| Callback | Purpose | Called |
|----------|---------|--------|
| `on_start()` | Initialize game state | Once at startup |
| `on_update(dt)` | Update game logic | Every frame |
| `on_draw()` | Render graphics | Every frame, after update |

## Next Steps

- Check out the [example games](https://github.com/Rani367/pixel/tree/main/examples/games) for complete working games
- Read the [API Reference](/pixel/docs/api/core) for all available functions
- Read the [Language Guide](/pixel/docs/language/basics) to learn more about Pixel syntax

## Quick Reference

### Input
```pixel
key_down(KEY_SPACE)      // Key is held
key_pressed(KEY_SPACE)   // Key just pressed this frame
mouse_x(), mouse_y()     // Mouse position
mouse_down(MOUSE_LEFT)   // Mouse button held
```

### Drawing
```pixel
clear(color)
draw_rect(x, y, w, h, color)
draw_circle(x, y, radius, color)
draw_line(x1, y1, x2, y2, color)
draw_text(text, x, y, font, color)
```

### Colors
```pixel
rgb(255, 0, 0)           // Red
rgba(0, 255, 0, 128)     // Semi-transparent green
WHITE, BLACK, RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, ORANGE, GRAY
```

### Math
```pixel
random()                 // 0.0 to 1.0
random_range(10, 20)     // 10.0 to 20.0
sin(angle), cos(angle)   // Angle in radians
sqrt(n), pow(n, exp)
min(a, b), max(a, b), clamp(val, lo, hi)
```
