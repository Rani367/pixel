---
title: "Data Structures in Pixel"
description: "Work with lists, structs, and vectors in Pixel. Learn to create, modify, and iterate over collections for managing game entities and state."
keywords: ["Pixel lists", "structs", "vec2", "game data structures", "arrays in Pixel"]
---

# Data Structures

## Lists

Lists are ordered collections that can hold any type of value.

### Creating Lists

```pixel
empty = []
numbers = [1, 2, 3, 4, 5]
mixed = [42, "hello", true, [1, 2]]
```

### Accessing Elements

Lists are zero-indexed:

```pixel
colors = ["red", "green", "blue"]
first = colors[0]   // "red"
second = colors[1]  // "green"
```

### Modifying Elements

```pixel
scores = [10, 20, 30]
scores[1] = 25  // [10, 25, 30]
```

### List Length

```pixel
items = [1, 2, 3, 4, 5]
count = len(items)  // 5
```

### Adding Elements

```pixel
items = []
push(items, "sword")   // ["sword"]
push(items, "shield")  // ["sword", "shield"]
```

### Removing Elements

```pixel
items = ["a", "b", "c"]
last = pop(items)      // Returns "c", items is now ["a", "b"]
remove(items, 0)       // Remove at index, items is now ["b"]
```

### Inserting Elements

```pixel
items = ["a", "c"]
insert(items, 1, "b")  // ["a", "b", "c"]
```

### Searching

```pixel
items = ["apple", "banana", "cherry"]

// Check if item exists
if contains(items, "banana") {
    println("Found it!")
}

// Get index of item
idx = index_of(items, "cherry")  // 2
```

### Iterating

```pixel
enemies = [enemy1, enemy2, enemy3]
i = 0
while i < len(enemies) {
    enemies[i].update()
    i = i + 1
}
```

### Reverse Iteration (for removal)

When removing items during iteration, go backwards:

```pixel
i = len(bullets) - 1
while i >= 0 {
    if bullets[i].off_screen {
        remove(bullets, i)
    }
    i = i - 1
}
```

## Structs

Structs are templates for creating objects with named fields.

### Defining Structs

First, define a struct with its field names:

```pixel
struct Player { x, y, health }
struct Enemy { x, y, health, speed }
```

### Creating Instances

Create instances by calling the struct name like a function:

```pixel
player = Player(100, 200, 100)
enemy = Enemy(300, 200, 50, 80)
```

### Accessing Properties

```pixel
println(player.x)       // 100
println(player.health)  // 100
```

### Modifying Properties

```pixel
player.x = player.x + 10
player.health = player.health - 25
```

### Structs with Methods

Structs can include methods:

```pixel
struct Vector {
    x, y,
    function length() {
        return sqrt(this.x * this.x + this.y * this.y)
    }
}

v = Vector(3, 4)
println(v.length())  // 5
```

### Structs in Lists

```pixel
struct Enemy { x, y, health }

enemies = []

push(enemies, Enemy(100, 100, 30))
push(enemies, Enemy(200, 150, 30))

// Update all enemies
i = 0
while i < len(enemies) {
    enemies[i].x = enemies[i].x + 1
    i = i + 1
}
```

## Vec2

Vec2 is a built-in type for 2D vectors.

### Creating Vectors

```pixel
position = vec2(100, 200)
velocity = vec2(5, -3)
```

### Accessing Components

```pixel
pos = vec2(100, 200)
println(pos.x)  // 100
println(pos.y)  // 200
```

### Vector Math

```pixel
a = vec2(1, 2)
b = vec2(3, 4)

// Add vectors
sum = vec2(a.x + b.x, a.y + b.y)

// Scale vector
scaled = vec2(a.x * 2, a.y * 2)

// Magnitude (length)
length = sqrt(a.x * a.x + a.y * a.y)

// Normalize (unit vector)
normalized = vec2(a.x / length, a.y / length)
```

## Common Patterns

### Entity List

```pixel
struct Bullet { x, y, vel_x, vel_y, active }

bullets = []

function spawn_bullet(x, y, dx, dy) {
    push(bullets, Bullet(x, y, dx, dy, true))
}

function update_bullets(dt) {
    i = len(bullets) - 1
    while i >= 0 {
        b = bullets[i]
        b.x = b.x + b.vel_x * dt
        b.y = b.y + b.vel_y * dt

        if b.x < 0 or b.x > 800 {
            remove(bullets, i)
        }
        i = i - 1
    }
}
```

### Game State

Use global variables to track game state:

```pixel
score = 0
lives = 3
level = 1
paused = false
enemies = []
bullets = []

function reset_game() {
    score = 0
    lives = 3
    level = 1
    paused = false
    enemies = []
    bullets = []
}
```

### Tile Grid

```pixel
tiles = []
width = 10
height = 8

// Create 2D grid as flat list
i = 0
while i < width * height {
    push(tiles, 0)
    i = i + 1
}

// Get tile at (x, y)
function get_tile(x, y) {
    return tiles[y * width + x]
}

// Set tile at (x, y)
function set_tile(x, y, value) {
    tiles[y * width + x] = value
}
```
