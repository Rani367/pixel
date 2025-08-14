# Functions

## Defining Functions

```pixel
function greet(name) {
    println("Hello, " + name + "!")
}

greet("World")  // Hello, World!
```

## Return Values

```pixel
function add(a, b) {
    return a + b
}

result = add(3, 4)  // 7
```

Functions without an explicit return statement return `none`.

## Multiple Parameters

```pixel
function create_enemy(x, y, health) {
    enemy = {}
    enemy.x = x
    enemy.y = y
    enemy.health = health
    return enemy
}

boss = create_enemy(400, 100, 500)
```

## No Parameters

```pixel
function get_random_color() {
    r = random_int(0, 255)
    g = random_int(0, 255)
    b = random_int(0, 255)
    return rgb(r, g, b)
}
```

## Early Return

```pixel
function divide(a, b) {
    if b == 0 {
        println("Error: division by zero")
        return none
    }
    return a / b
}
```

## First-Class Functions

Functions are values. You can store them in variables and pass them around:

```pixel
function double(x) {
    return x * 2
}

operation = double
result = operation(5)  // 10
```

## Anonymous Functions

Create functions without naming them:

```pixel
add = function(a, b) {
    return a + b
}

result = add(3, 4)  // 7
```

## Closures

Functions capture variables from their enclosing scope:

```pixel
function make_counter() {
    count = 0
    return function() {
        count = count + 1
        return count
    }
}

counter = make_counter()
println(counter())  // 1
println(counter())  // 2
println(counter())  // 3
```

## Game Callbacks

Pixel automatically calls these functions if you define them:

```pixel
function on_start() {
    // Called once when the game starts
    create_window(800, 600, "My Game")
}

function on_update(dt) {
    // Called every frame
    // dt = time since last frame in seconds
    player_x = player_x + speed * dt
}

function on_draw() {
    // Called every frame after on_update
    clear(BLACK)
    draw_sprite(player)
}
```

### Input Callbacks

```pixel
function on_key_down(key) {
    if key == KEY_SPACE {
        jump()
    }
}

function on_key_up(key) {
    // Key was released
}

function onon == MOUSE_LEFT {
        shoot(x, y)
    }
}

function on_mouse_move(x, y) {
    cursor_x = x
    cursor_y = y
}
```

## Recursion

Functions can call themselves:

```pixel
function factorial(n) {
    if n <= 1 {
        return 1
    }
    return n * factorial(n - 1)
}

println(factorial(5))  // 120
```

## Common Patterns

### Helper Functions

```pixel
function distance(x1, y1, x2, y2) {
    dx = x2 - x1
    dy = y2 - y1
    return sqrt(dx * dx + dy * dy)
}

function is_in_range(enemy) {
    d = distance(player.x, player.y, enemy.x, enemy.y)
    return d < attack_range
}
```

### State Machines

```pixel
state