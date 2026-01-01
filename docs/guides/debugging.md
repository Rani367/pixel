---
title: "Debugging Guide"
description: "Debug your Pixel games effectively. Learn to understand error messages, find common mistakes, and use debugging techniques to fix issues."
keywords: ["debugging", "error messages", "troubleshooting", "common mistakes", "game debugging"]
---

# Debugging Guide

When something goes wrong in your Pixel game, this guide will help you find and fix the problem.

## Understanding Error Messages

Pixel provides helpful error messages that show exactly where the problem occurred.

### Error Format

```
Error at line 15, column 10 in game.pixel:
    player.x = player.x + speeed * dt
                          ^^^^^^
Error: Undefined variable 'speeed'. Did you mean 'speed'?
```

The error message includes:
- **Line and column number** - Where the error occurred
- **File name** - Which file has the error
- **Code snippet** - The problematic line with the issue highlighted
- **Error description** - What went wrong
- **Suggestion** - Sometimes a helpful hint (like "Did you mean 'speed'?")

## Common Errors and Solutions

### Undefined Variable

```
Error: Undefined variable 'playar'
```

**Cause**: You're using a variable that doesn't exist or is misspelled.

**Fix**: Check the spelling and make sure you defined the variable first.

```pixel
// Wrong
playar.x = 100

// Right
player.x = 100
```

### Undefined Function

```
Error: Undefined function 'crate_window'
```

**Cause**: The function name is misspelled or doesn't exist.

**Fix**: Check the function name against the API reference.

```pixel
// Wrong
crate_window(800, 600, "Game")

// Right
create_window(800, 600, "Game")
```

### Wrong Number of Arguments

```
Error: Expected 3 arguments but got 2
```

**Cause**: You called a function with the wrong number of parameters.

**Fix**: Check the function signature in the API reference.

```pixel
// Wrong - missing color argument
draw_rect(100, 100, 50, 50)

// Right
draw_rect(100, 100, 50, 50, RED)
```

### Cannot Read Property of None

```
Error: Cannot read property 'x' of none
```

**Cause**: You're trying to access a property on a `none` value.

**Fix**: Make sure the variable is initialized before using it.

```pixel
// Wrong - player is still none
player = none
player.x = 100  // Error!

// Right
player = create_sprite(none)
player.x = 100
```

### Type Errors

```
Error: Cannot perform '+' on string and number
```

**Cause**: You're trying to combine incompatible types.

**Fix**: Use `to_string()` or `to_number()` to convert.

```pixel
// Wrong
message = "Score: " + score

// Right
message = "Score: " + to_string(score)
```

## Debugging Techniques

### Using println() to Debug

The simplest debugging technique is printing values to see what's happening:

```pixel
function on_update(dt) {
    println("Player position: " + to_string(player.x) + ", " + to_string(player.y))
    println("Velocity: " + to_string(player.velocity_x))
    println("dt: " + to_string(dt))
}
```

### Checking Conditions

Print when specific events happen:

```pixel
function on_update(dt) {
    if key_pressed(KEY_SPACE) {
        println("Space pressed!")
        println("Can jump: " + to_string(player.grounded))
    }

    if collides(player, enemy) {
        println("Collision detected!")
    }
}
```

### Visualizing Collision Boxes

Draw debug rectangles to see hitboxes:

```pixel
DEBUG = true

function on_draw() {
    clear(BLACK)
    draw_sprite(player)

    if DEBUG {
        // Draw hitbox as outline
        draw_rect_outline(player.x, player.y, player.width, player.height, GREEN)

        for enemy in enemies {
            draw_rect_outline(enemy.x, enemy.y, enemy.width, enemy.height, RED)
        }
    }
}
```

### Tracking State Changes

```pixel
old_state = ""

function on_update(dt) {
    if game_state != old_state {
        println("State changed: " + old_state + " -> " + game_state)
        old_state = game_state
    }
}
```

### Pause and Step

Add a debug pause to examine state:

```pixel
debug_pause = false

function on_update(dt) {
    if key_pressed(KEY_P) {
        debug_pause = not debug_pause
        println("Debug pause: " + to_string(debug_pause))
    }

    if debug_pause {
        // Only advance one frame when pressing N
        if not key_pressed(KEY_N) {
            return
        }
        println("Stepping one frame...")
    }

    // Normal game logic here
}
```

## Common Mistakes

### Forgetting to Call create_window()

```pixel
// Wrong - no window created
function on_start() {
    player = create_sprite(none)
}

// Right
function on_start() {
    create_window(800, 600, "My Game")
    player = create_sprite(none)
}
```

### Not Using dt for Movement

```pixel
// Wrong - movement speed depends on frame rate
function on_update(dt) {
    player.x = player.x + 5
}

// Right - consistent speed
function on_update(dt) {
    player.x = player.x + 300 * dt
}
```

### Forgetting to Clear the Screen

```pixel
// Wrong - trails and visual glitches
function on_draw() {
    draw_sprite(player)
}

// Right
function on_draw() {
    clear(BLACK)
    draw_sprite(player)
}
```

### Modifying List During Iteration

```pixel
// Wrong - skips items or crashes
i = 0
while i < len(bullets) {
    if bullets[i].off_screen {
        remove(bullets, i)  // Messes up indices!
    }
    i = i + 1
}

// Right - iterate backwards
i = len(bullets) - 1
while i >= 0 {
    if bullets[i].off_screen {
        remove(bullets, i)
    }
    i = i - 1
}
```

### Using = Instead of ==

```pixel
// Wrong - assignment, not comparison
if player.health = 0 {
    game_over()
}

// Right
if player.health == 0 {
    game_over()
}
```

### Off-by-One in Loops

```pixel
// Wrong - misses last item
for i in range(len(items) - 1) {
    println(items[i])
}

// Right
for i in range(len(items)) {
    println(items[i])
}
```

### Not Initializing Variables

```pixel
// Wrong - score undefined in on_draw
function on_update(dt) {
    score = score + 1  // Error if score not defined!
}

// Right - define at top level
score = 0

function on_update(dt) {
    score = score + 1
}
```

## When Things Look Wrong

### Sprites Not Appearing

Check:
1. Is `visible` set to `true`?
2. Are x/y coordinates on screen?
3. Is `draw_sprite()` called in `on_draw()`?
4. Is the image loaded successfully?
5. Is `clear()` called after drawing?

```pixel
// Debug sprite visibility
function on_draw() {
    clear(BLACK)
    println("Drawing at: " + to_string(player.x) + ", " + to_string(player.y))
    println("Visible: " + to_string(player.visible))
    draw_sprite(player)
}
```

### Movement Not Working

Check:
1. Is the input being detected?
2. Is dt being used correctly?
3. Is the speed value reasonable?
4. Is something resetting the position?

```pixel
function on_update(dt) {
    if key_down(KEY_RIGHT) {
        println("Right key detected")
        old_x = player.x
        player.x = player.x + 200 * dt
        println("Moved from " + to_string(old_x) + " to " + to_string(player.x))
    }
}
```

### Collisions Not Working

Check:
1. Do sprites have width/height set?
2. Are positions correct (not off by sprite size)?
3. Is collision check in `on_update()`?

```pixel
println("Player bounds: " + to_string(player.x) + ", " + to_string(player.y) +
        " - " + to_string(player.x + player.width) + ", " + to_string(player.y + player.height))
println("Enemy bounds: " + to_string(enemy.x) + ", " + to_string(enemy.y) +
        " - " + to_string(enemy.x + enemy.width) + ", " + to_string(enemy.y + enemy.height))
```

## Getting Help

If you're still stuck:

1. **Simplify** - Remove code until it works, then add back piece by piece
2. **Check examples** - Compare with working example code in `examples/`
3. **Read the API** - Make sure you understand what each function does
4. **Take a break** - Fresh eyes often spot issues immediately

## See Also

- [Getting Started](/pixel/docs/getting-started) - Basic setup
- [API Reference](/pixel/docs/api) - Function documentation
