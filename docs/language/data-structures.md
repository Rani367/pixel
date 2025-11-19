 = 0
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

Structs are key-value objects for grouping related data.

### Creating Structs

```pixel
// Empty struct, add properties later
player = {}
player.x = 100
player.y = 200
player.health = 100

// Inline creation
enemy = {x: 300, y: 200, health: 50}
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

### Adding Properties

Properties can be added at any time:

```pixel
player = {}
player.name = "Hero"
player.level = 1
player.inventory = []  // Nested list
```

### Nested Structs

```pixel
game = {}
game.player = {x: 100, y: 200}
game.settings = {volume: 0.8, fullscreen: false}

// Access nested properties
println(game.player.x)
println(game.settings.volume)
```

### Structs in Lists

```pixel
enemies = []

enemy1 = {x: 100, y: 100, health: 30}
enemy2 = {x: 200, y: 150, health: 30}

push(enemies, enemy1)
push(enemies, enemy2)

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
bullets = []

function spawn_bullet(x, y, dx, dy) {
    b = {}
    b.x = x
    b.y = y
    b.vel_x = dx
    b.vel_y = dy
    b.active = true
    push(bullets, b)
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

### Game State Struct

```pixel
game = {}
game.score = 0
game.lives = 3
game.level = 1
game.paused = false
game.enemies = []
game.bullets = []

function reset_game() {
    game.score = 0
    game.lives = 3
    game.level = 1
    game.paused = false
    game.enemies = []
    game.bullets = []
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
i