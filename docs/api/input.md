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
|
| `KEY_A` - `KEY_Z` | A-Z keys |

### Number Keys
| Constant | Key |
|----------|-----|
| `KEY_0` - `KEY_9` | 0-9 keys |

### Arrow Keys
| Constant | Key |
|----------|-----|
| `KEY_UP` | Up arrow |
| `KEY_DOWN` | Down arrow |
| `KEY_LEFT` | Left arrow |
| `KEY_RIGHT` | Right arrow |

### Common Keys
| Constant | Key |
|----------|-----|
| `KEY_SPACE` | Spacebar |
| `KEY_ENTER` | Enter/Return |
| `KEY_TAB` | Tab |
| `KEY_BACKSPACE` | Backspace |
| `KEY_ESCAPE` | Escape |

### Modifier Keys
| Constant | Key |
|----------|-----|
| `KEY_SHIFT` | Shift (either) |
| `KEY_CTRL` | Control (either) |
| `KEY_ALT` | Alt/Option (either) |

### Function Keys
| Constant | Key |
|----------|-----|
| `KEY_F1` - `KEY_F12` | Function keys |

## Mouse Input

### mouse_x()
Returns the mouse X position in window coordinates.

### mouse_y()
Returns the mouse Y position in window coordinates.

```pixel
function on_update(dt) {
    crosshair_x = mouse_x()
    crosshair_y = mouse_y()
}
```

### mouse_down(button)
Returns `true` while the mouse button is held.

```pixel
if mouse_down(MOUSE_LEFT) {
    // Continuous fire
    shoot()
}
```

### mouse_pressed(button)
Returns `true` only on the frame the button was pressed.

```pixel
if mouse_pressed(MOUSE_LEFT) {
    // Single click
    select_item()
}
```

### mouse_released(button)
Returns `true` only on the frame the button was released.

```pixel
if mouse_released(MOUSE_LEFT) {
    drop_item()
}
```

## Mouse Button Constants

| Constant | Button |
|----------|--------|
| `MOUSE_LEFT` | Left button |
| `MOUSE_RIGHT` | Right button |
| `MOUSE_MIDDLE` | Middle button |

## Input Callbacks

For event-driven input handling, you can define callback functions.

### on_key_down(key)
Called when a key is pressed.

```pixel
function on_key_down(key) {
    if key == KEY_P {
        toggle_pause()
    }
}
```

### on_key_up(key)
Called when a key is released.

```pixel
function on_key_up(key) {
    if key == KEY_SPACE {
        release_arrow()
    }
}
```

### on_mouse_click(button, x, y)
Calle