# Core Language Reference

## Variables

Variables are dynamically typed and do not need declaration:

```pixel
x = 42
name = "Player"
is_alive = true
items = []
```

## Data Types

| Type | Example | Description |
|------|---------|-------------|
| Number | `42`, `3.14`, `-10` | 64-bit floating point |
| String | `"hello"`, `'world'` | Text (single or double quotes) |
| Boolean | `true`, `false` | Logical values |
| None | `none` | Absence of value |
| List | `[1, 2, 3]` | Ordered collection |
| Struct | `{x: 10, y: 20}` | Key-value object |
| Vec2 | `vec2(10, 20)` | 2D vector |

## Operators

### Arithmetic
| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `5 + 3` = `8` |
| `-` | Subtraction | `5 - 3` = `2` |
| `*` | Multiplication | `5 * 3` = `15` |
| `/` | Division | `5 / 2` = `2.5` |
| `%` | Modulo | `5 % 3` = `2` |

### Comparison
| Operator | Description | Example |
|----------|-------------|---------|
| `==` | Equal | `5 == 5` = `true` |
| `!=` | Not equal | `5 != 3` = `true` |
| `<` | Less than | `3 < 5` = `true` |
| `>` | Greater than | `5 > 3` = `true` |
| `<=` | Less or equal | `3 <= 3` = `true` |
| `>=` | Greater or equal | `5 >= 5` = `true` |

### Logical
| Operator | Description | Example |
|----------|-------------|---------|
| `and` | Logical AND | `true and false` = `false` |
| `or` | Logical OR | `true or false` = `true` |
| `not` | Logical NOT | `not true` = `false` |

## Control Flow

### If Statements

```pixel
if x > 10 {
    print("big")
} else if x > 5 {
    print("medium")
} else {
    print("small")
}
```

### While Loops

```pixel
i = 0
while i < 10 {
    print(i)
    i = i + 1
}
```

### Break and Continue

```pixel
i = 0
while true {
    i = i + 1
    if i == 5 {
        continue  // Skip to next iteration
    }
    if i >= 10 {
        break     // Exit loop
    }
    print(i)
}
```

## Functions

### Defining Functions

```pixel
function greet(name) {
    return "Hello, " + name + "!"
}

message = greet("World")
print(message)
```

### Anonymous Functions

```pixel
add = function(a, b) {
    return a + b
}

result = add(3, 4)  // 7
```

### Closures

Functions can capture variables from their enclosing scope:

```pixel
function make_counter() {
    count = 0
    return function() {
        count = count + 1
        return count
    }
}

counter = make_counter()
print(counter())  // 1
print(counter())  // 2
```

## Lists

### Creating Lists

```pixel
empty = []
numbers = [1, 2, 3, 4, 5]
mixed = [42, "hello", true, [1, 2]]
```

### List Functions

| Function | Description | Example |
|----------|-------------|---------|
| `len(list)` | Get length | `len([1,2,3])` = `3` |
| `push(list, item)` | Add to end | `push(items, "new")` |
| `pop(list)` | Remove from end | `last = pop(items)` |
| `insert(list, i, item)` | Insert at index | `insert(items, 0, "first")` |
| `remove(list, i)` | Remove at index | `remove(items, 2)` |
| `contains(list, item)` | Check membership | `contains([1,2], 2)` = `true` |
| `index_of(list, item)` | Find index | `index_of([a,b,c], b)` = `1` |

### Accessing Elements

```pixel
items = ["a", "b", "c"]
first = items[0]      // "a"
items[1] = "B"        // Modify element
```

## Structs

### Creating Structs

```pixel
player = {}
player.x = 100
player.y = 200
player.name = "Hero"

// Or inline
enemy = {x: 300, y: 200, health: 50}
```

### Accessing Properties

```pixel
print(player.name)    // "Hero"
player.health = 100   // Add new property
```

## Strings

### String Functions

| Function | Description | Example |
|----------|-------------|---------|
| `len(str)` | Get length | `len("hello")` = `5` |
| `substring(str, start, len)` | Extract substring | `substring("hello", 1, 3)` = `ell` |
| `split(str, sep)` | Split by separator | `split("a,b,c", ",")` = `["a","b","c"]` |
| `join(list, sep)` | Join with separator | `join(["a","b"], "-")` = `"a-b"` |
| `upper(str)` | Uppercase | `upper("hi")` = `"HI"` |
| `lower(str)` | Lowercase | `lower("HI")` = `"hi"` |
| `to_string(val)` | Convert to string | `to_string(42)` = `"42"` |
| `to_number(str)` | Convert to number | `to_number("42")` = `42` |

### String Concatenation

```pixel
greeting = "Hello, " + name + "!"
```

## Type Functions

| Function | Description | Example |
|----------|-------------|---------|
| `type(val)` | Get type name | `type(42)` = `"number"` |
| `to_string(val)` | Convert to string | `to_string(true)` = `"true"` |
| `to_number(str)` | Parse number | `to_number("3.14")` = `3