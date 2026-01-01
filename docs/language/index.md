---
title: "Pixel Language Guide"
description: "Complete guide to the Pixel programming language syntax and features. Learn variables, functions, control flow, and data structures for game development."
keywords: ["Pixel syntax", "programming language guide", "game scripting language", "Pixel language reference", "beginner programming syntax"]
---

# Language Guide

This guide covers the Pixel programming language syntax and features.

## Contents

1. [Basics](basics) - Variables, types, and operators
2. [Control Flow](control-flow) - Conditionals and loops
3. [Functions](functions) - Defining and using functions
4. [Data Structures](data-structures) - Lists and structs

## Overview

Pixel is a dynamically-typed language with a simple, readable syntax. There are no semicolons, no explicit type declarations, and minimal boilerplate.

```pixel
// Variables
name = "Player"
health = 100
is_alive = true

// Functions
function greet(who) {
    return "Hello, " + who + "!"
}

// Control flow
if health > 0 {
    println(greet(name))
}

// For loops
for i in range(10) {
    println(i)
}
```

## Key Features

- **Dynamic typing** - Variables can hold any type
- **No semicolons** - Line breaks end statements
- **For and while loops** - `for x in list` and `while condition`
- **Compound operators** - `+=`, `-=`, `*=`, `/=`, `++`, `--`
- **First-class functions** - Functions are values, closures supported
- **Game callbacks** - Define `on_start`, `on_update`, `on_draw`

## Comments

```pixel
// Single-line comment

/*
   Multi-line
   comment
*/
```

## File Extension

Pixel source files use the `.pixel` extension:
```
game.pixel
utils.pixel
main.pixel
```

## Running Programs

```bash
pixel game.pixel
```

## See Also

- [API Reference](/pixel/docs/api) - All built-in functions
- [Getting Started](/pixel/docs/getting-started) - Create your first game
- [Guides](/pixel/docs/guides) - In-depth tutorials
