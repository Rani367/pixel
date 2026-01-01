---
title: "Control Flow in Pixel"
description: "Master if statements, while loops, break, and continue in Pixel. Learn conditional logic and iteration patterns for game programming."
keywords: ["if statements", "while loops", "control flow", "conditionals", "game logic programming"]
---

# Control Flow

## If Statements

```pixel
if health > 0 {
    println("Alive!")
}
```

### Else

```pixel
if health > 0 {
    println("Alive!")
} else {
    println("Game Over")
}
```

### Else If

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
if health > 0 and ammo > 0 {
    can_fight = true
}

if is_paused or game_over {
    return
}
```

## For Loops

The `for` loop iterates over items in a list or range.

### Basic For Loop

```pixel
items = ["sword", "shield", "potion"]
for item in items {
    println(item)
}
```

### For Loop with Range

Use `range()` to generate a sequence of numbers:

```pixel
// 0 to 9
for i in range(10) {
    println(i)
}

// 5 to 9
for i in range(5, 10) {
    println(i)
}

// 10 down to 1 (step of -1)
for i in range(10, 0, -1) {
    println(i)
}

// Even numbers: 0, 2, 4, 6, 8
for i in range(0, 10, 2) {
    println(i)
}
```

### Iterating with Index

```pixel
enemies = [enemy1, enemy2, enemy3]
for i in range(len(enemies)) {
    enemies[i].update()
}
```

## While Loops

Use `while` for loops that depend on a condition rather than iteration count.

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
    if found_target {
        break  // Exit the loop
    }
    i = i + 1
}
```

## Continue

Skip to the next iteration:

```pixel
i = 0
while i < 10 {
    i = i + 1
    if i == 5 {
        continue  // Skip printing 5
    }
    println(i)
}
```

## Common Patterns

### Counting Loop

```pixel
// Preferred: use a for loop with range
for i in range(10) {
    // Do something with i (0 to 9)
}

// When you need more control
i = 0
while i < 10 {
    // Do something with i
    i = i + 1
}
```

### Iterating a List

```pixel
// Preferred: use a for loop
items = ["sword", "shield", "potion"]
for item in items {
    println(item)
}

// With index access
for i in range(len(items)) {
    println(items[i])
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
// Using for loop with negative step
for i in range(10, 0, -1) {
    println(i)
}
println("Go!")

// Using while loop
countdown = 10
while countdown > 0 {
    println(countdown)
    countdown = countdown - 1
}
println("Go!")
```
