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
   