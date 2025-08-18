layer.x, player.y)
}
```

### Bounce Off Walls

```pixel
function on_update(dt) {
    ball.x = ball.x + ball.vel_x * dt
    ball.y = ball.y + ball.vel_y * dt

    // Bounce off sides
    if ball.x <= 0 or ball.x >= window_width() - ball_size {
        ball.vel_x = -ball.vel_x
    }

    // Bounce off top/bottom
    if ball.y <= 0 or ball.y >= window_height() - ball_size {
        ball.vel_y = -ball.vel_y
    }
}
```
 movement
    if key_down(KEY_LEFT) {
        player.velocity_x = -MOVE_SPEED
    } else if key_down(KEY_RIGHT) {
        player.velocity_x = MOVE_SPEED
    } else {
        player.velocity_x = 0
    }

    // Jumping
    if key_pressed(KEY_SPACE) and player.grounded {
        player.velocity_y = -JUMP_FORCE
        player.grounded = false
    }

    // Apply gravity
    player.velocity_y = player.velocity_y + GRAVITY * dt

    // Move player
    player.x = player.x + player.velocity_x * dt
    player.y = player.y + player.velocity_y * dt

    // Ground collision (simple)
    if player.y > ground_y {
        player.y = ground_y
        player.velocity_y = 0
        player.grounded = true
    }
}
```

### Top-Down Movement

```pixel
function on_update(dt) {
    dx = 0
    dy = 0

    if key_down(KEY_W) { dy = -1 }
    if key_down(KEY_S) { dy = 1 }
    if key_down(KEY_A) { dx = -1 }
    if key_down(KEY_D) { dx = 1 }

    // Normalize diagonal movement
    if dx != 0 and dy != 0 {
        dx = dx * 0.707
        dy = dy * 0.707
    }

    player.x = player.x + dx * speed * dt
    player.y = player.y + dy * speed * dt
}
```

### Bullet/Projectile

```pixel
bullets = []

function spawn_bullet(x, y, angle) {
    bullet = {}
    bullet.x = x
    bullet.y = y
    bullet.vel_x = cos(angle) * BULLET_SPEED
    bullet.vel_y = sin(angle) * BULLET_SPEED
    push(bullets, bullet)
}

function update_bullets(dt) {
    i = len(bullets) - 1
    while i >= 0 {
        b = bullets[i]
        b.x = b.x + b.vel_x * dt
        b.y = b.y + b.vel_y * dt

        // Remove if off screen
        if b.x < 0 or b.x > window_width() or b.y < 0 or b.y > window_height() {
            remove(bullets, i)
        }
        i = i - 1
    }
}
```

### Simple Enemy AI

```pixel
function update_enemy(enemy, dt) {
    // Move toward player
    dx = player.x - enemy.x
    dy = player.y - enemy.y
    dist = sqrt(dx * dx + dy * dy)

    if dist > 0 {
        enemy.x = enemy.x + (dx / dist) * enemy.speed * dt
        enemy.y = enemy.y + (dy / dist) * enemy.speed * dt
    }

    // Face player
    look_at(enemy, p