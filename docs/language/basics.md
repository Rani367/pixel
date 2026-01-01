---
title: "Pixel Language Basics"
description: "Learn the fundamentals of Pixel programming: variables, data types, numbers, strings, booleans, and operators. Perfect for programming beginners."
keywords: ["Pixel variables", "data types", "programming basics", "operators", "beginner programming concepts"]
---

# Basics

## Variables

Variables are created by assignment. No declaration keyword is needed.

```pixel
x = 42
name = "Hero"
is_active = true
```

Variables are dynamically typed and can hold any value:

```pixel
x = 10      // number
x = "text"  // now a string
x = true    // now a boolean
```

## Data Types

| Type | Example | Description |
|------|---------|-------------|
| Number | `42`, `3.14`, `-10` | 64-bit floating point |
| String | `"hello"`, `'world'` | Text |
| Boolean | `true`, `false` | Logical values |
| None | `none` | Absence of value |
| List | `[1, 2, 3]` | Ordered collection |
| Struct | `{x: 10, y: 20}` | Key-value object |

## Numbers

All numbers are 64-bit floating point:

```pixel
age = 25
pi = 3.14159
negative = -100
```

## Strings

Strings can use single or double quotes:

```pixel
name = "Alice"
message = 'Hello, World!'
```

Concatenate with `+`:

```pixel
greeting = "Hello, " + name + "!"
```

## Booleans

```pixel
is_alive = true
game_over = false
```

## None

Represents the absence of a value:

```pixel
player = none

if player == none {
    println("No player loaded")
}
```

## Operators

### Arithmetic

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `5 + 3` → `8` |
| `-` | Subtraction | `5 - 3` → `2` |
| `*` | Multiplication | `5 * 3` → `15` |
| `/` | Division | `5 / 2` → `2.5` |
| `%` | Modulo | `5 % 3` → `2` |

### Comparison

| Operator | Description | Example |
|----------|-------------|---------|
| `==` | Equal | `5 == 5` → `true` |
| `!=` | Not equal | `5 != 3` → `true` |
| `<` | Less than | `3 < 5` → `true` |
| `>` | Greater than | `5 > 3` → `true` |
| `<=` | Less or equal | `3 <= 3` → `true` |
| `>=` | Greater or equal | `5 >= 5` → `true` |

### Logical

| Operator | Description | Example |
|----------|-------------|---------|
| `and` | Logical AND | `true and false` → `false` |
| `or` | Logical OR | `true or false` → `true` |
| `not` | Logical NOT | `not true` → `false` |

### Compound Assignment

Combine an operation with assignment:

| Operator | Equivalent | Example |
|----------|------------|---------|
| `+=` | `x = x + y` | `score += 10` |
| `-=` | `x = x - y` | `health -= damage` |
| `*=` | `x = x * y` | `speed *= 2` |
| `/=` | `x = x / y` | `value /= 2` |

```pixel
score = 0
score += 10    // score is now 10
score += 5     // score is now 15
score *= 2     // score is now 30
```

### Increment and Decrement

| Operator | Equivalent | Description |
|----------|------------|-------------|
| `++` | `x = x + 1` | Increment by 1 |
| `--` | `x = x - 1` | Decrement by 1 |

```pixel
count = 0
count++     // count is now 1
count++     // count is now 2
count--     // count is now 1
```

## Type Checking

Use `type()` to check a value's type:

```pixel
println(type(42))        // "number"
println(type("hello"))   // "string"
println(type(true))      // "boolean"
println(type(none))      // "none"
println(type([1, 2]))    // "list"
```

## Type Conversion

```pixel
// Number to string
s = to_string(42)        // "42"

// String to number
n = to_number("3.14")    // 3.14
```
