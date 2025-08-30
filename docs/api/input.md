n on_mouse_move(x, y) {
    cursor_x = x
    cursor_y = y
}
```

## Common Patterns

### WASD + Arrow Key Movement

```pixel
function on_update(dt) {
    dx = 0
    dy = 0

    if key_down(KEY_W) or key_down(KEY_UP) {
        dy = -1
    }
    if key_down(KEY_S) or key_down(KEY_DOWN) {
        dy = 1
    }
    if key_down(KEY_A) or key_down(KEY_LEFT) {
        dx = -1
    }
    if key_down(KEY_D) or key_down(KEY_RIGHT) {
        dx = 1
    }

    player.x = player.x + dx * speed * dt
    player.y = player.y + dy * speed * dt
}
```

### Mouse Aiming

```pixel
function on_update(dt) {
    // Calculate angle from player to mouse
    dx = mouse_x() - player.x
    dy = mouse_y() - player.y
    player.rotation = atan2(dy, dx) * 180 / 3.14159
}
```

### Click to Move

```pixel
target_x = 0
target_y = 0
moving = false

function on_mouse_click(button, x, y) {
    if button == MOUSE_LEFT {
        target_x = x
        target_y = y
        moving = true
    }
}

function on_update(dt) {
    if moving {
        player.x = move_toward(player.x, target_x, speed * dt)
        player.y = move_toward(player.y, target_y, speed * dt)

        if abs(player.x - target_x) < 1 and abs(player.y - target_y) < 1 {
            moving = false
        }
    }
}
```
