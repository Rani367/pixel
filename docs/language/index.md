---
title: "Pixel Language Guide"
description: "Complete guide to the Pixel programming language syntax and features. Learn variables, functions, control flow, and data structures for game development."
keywords: ["Pixel syntax", "programming language guide", "game scripting language", "Pixel language reference", "beginner programming syntax"]
---

# Language Guide

This guide covers the Pixel programming language syntax and features.

## Contents

1. [Basics](basics.md) - Variables, types, and operators
2. [Control Flow](control-flow.md) - Conditionals and loops
3. [Functions](functions.md) - Defining and using functions
4. [Data Structures](data-structures.md) - Lists and structs

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
```

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
pixel run game.pixel
```
