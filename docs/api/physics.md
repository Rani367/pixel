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
