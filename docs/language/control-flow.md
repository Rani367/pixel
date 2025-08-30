lth > 0 and ammo > 0 {
    can_fight = true
}

if is_paused or game_over {
    return
}
```

## While Loops

```pixel
i = 0
while i < 10 {
    println(i)
    i = i + 1
}
```

### Infinite Loops

```pixel
while true {
    update_game()
    if should_quit {
        break
    }
}
```

## Break

Exit a loop immediately:

```pixel
i = 0
while i < 100 {
   i < len(items) {
    println(items[i])
    i = i + 1
}
```

### Game Loop Check

```pixel
function on_update(dt) {
    if game_over {
        if key_pressed(KEY_SPACE) {
            restart_game()
        }
        return  // Skip rest of update
    }

    // Normal game update
    update_player(dt)
    update_enemies(dt)
}
```

### Search Loop

```pixel
found = false
i = 0
while i < len(enemies) and not found {
    if enemies[i].health <= 0 {
        found = true
    }
    i = i + 1
}
```

### Countdown

```pixel
countdown = 10
while countdown > 0 {
    println(countdown)
    countdown = countdown - 1
}
println("Go!")
```
If

```pixel
if score >= 100 {
    println("A")
} else if score >= 70 {
    println("B")
} else if score >= 50 {
    println("C")
} else {
    println("F")
}
```

### Nested Conditions

```pixel
if is_alive {
    if has_key {
        open_door()
    }
}
```

### Compound Conditions

```pixel
if hea