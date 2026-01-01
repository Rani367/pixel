---
title: "Core API Reference"
description: "Complete API reference for Pixel's core functions: type conversion, I/O, string manipulation, list operations, and utility functions."
keywords: ["Pixel API", "core functions", "string functions", "list operations", "type conversion"]
---

# Core API Reference

This page documents all core functions available in Pixel. For language syntax (variables, operators, control flow, etc.), see the [Language Guide](/pixel/docs/language/basics).

## Output Functions

### print(value)
Prints a value without a newline.

```pixel
print("Hello ")
print("World")
// Output: Hello World
```

### println(value)
Prints a value followed by a newline.

```pixel
println("Hello")
println("World")
// Output:
// Hello
// World
```

## Type Functions

### type(value)
Returns the type name of a value as a string.

| Type | Returns |
|------|---------|
| Number | `"number"` |
| String | `"string"` |
| Boolean | `"boolean"` |
| None | `"none"` |
| List | `"list"` |
| Struct | `"struct"` |
| Function | `"function"` |

```pixel
println(type(42))        // "number"
println(type("hello"))   // "string"
println(type(true))      // "boolean"
println(type(none))      // "none"
println(type([1, 2]))    // "list"
println(type({x: 10}))   // "struct"
```

### to_string(value)
Converts any value to a string.

```pixel
s = to_string(42)        // "42"
s = to_string(3.14)      // "3.14"
s = to_string(true)      // "true"
s = to_string([1, 2])    // "[1, 2]"
```

### to_number(string)
Parses a string as a number. Returns `none` if parsing fails.

```pixel
n = to_number("42")      // 42
n = to_number("3.14")    // 3.14
n = to_number("-10")     // -10
n = to_number("abc")     // none
```

## String Functions

### len(string)
Returns the length of a string in characters.

```pixel
len("hello")     // 5
len("")          // 0
```

### substring(string, start, length)
Extracts a portion of a string starting at `start` with the given `length`.

```pixel
substring("hello", 0, 2)   // "he"
substring("hello", 1, 3)   // "ell"
substring("hello", 2, 10)  // "llo" (clamped to end)
```

### split(string, separator)
Splits a string by the separator and returns a list of parts.

```pixel
split("a,b,c", ",")        // ["a", "b", "c"]
split("hello world", " ")  // ["hello", "world"]
split("abc", "")           // ["a", "b", "c"]
```

### join(list, separator)
Joins a list of values into a string with the separator between each.

```pixel
join(["a", "b", "c"], ",")    // "a,b,c"
join(["hello", "world"], " ") // "hello world"
join([1, 2, 3], "-")          // "1-2-3"
```

### upper(string)
Converts a string to uppercase.

```pixel
upper("hello")    // "HELLO"
upper("Hello")    // "HELLO"
```

### lower(string)
Converts a string to lowercase.

```pixel
lower("HELLO")    // "hello"
lower("Hello")    // "hello"
```

## List Functions

### len(list)
Returns the number of elements in a list.

```pixel
len([1, 2, 3])    // 3
len([])           // 0
```

### push(list, item)
Adds an item to the end of a list. Modifies the list in place.

```pixel
items = []
push(items, "sword")    // items is now ["sword"]
push(items, "shield")   // items is now ["sword", "shield"]
```

### pop(list)
Removes and returns the last item from a list. Returns `none` if empty.

```pixel
items = ["a", "b", "c"]
last = pop(items)       // last is "c", items is now ["a", "b"]
```

### insert(list, index, item)
Inserts an item at the specified index. Items after are shifted right.

```pixel
items = ["a", "c"]
insert(items, 1, "b")   // items is now ["a", "b", "c"]
```

### remove(list, index)
Removes the item at the specified index. Items after are shifted left.

```pixel
items = ["a", "b", "c"]
remove(items, 1)        // items is now ["a", "c"]
```

### contains(list, item)
Returns `true` if the list contains the item.

```pixel
items = [1, 2, 3]
contains(items, 2)      // true
contains(items, 5)      // false
```

### index_of(list, item)
Returns the index of the first occurrence of item, or `-1` if not found.

```pixel
items = ["a", "b", "c"]
index_of(items, "b")    // 1
index_of(items, "x")    // -1
```

## Utility Functions

### range(stop)
Returns a list of integers from `0` to `stop - 1`.

```pixel
range(5)    // [0, 1, 2, 3, 4]
range(0)    // []
```

### range(start, stop)
Returns a list of integers from `start` to `stop - 1`.

```pixel
range(3, 7)    // [3, 4, 5, 6]
range(5, 5)    // []
```

### range(start, stop, step)
Returns a list of integers from `start` to `stop` (exclusive), incrementing by `step`.

```pixel
range(0, 10, 2)    // [0, 2, 4, 6, 8]
range(10, 0, -1)   // [10, 9, 8, 7, 6, 5, 4, 3, 2, 1]
range(0, 10, 3)    // [0, 3, 6, 9]
```

### time()
Returns the current Unix timestamp (seconds since January 1, 1970).

```pixel
now = time()
println(now)    // e.g., 1735689600
```

### clock()
Returns a high-resolution time value in seconds. Use for measuring elapsed time.

```pixel
start = clock()
// ... do some work ...
elapsed = clock() - start
println("Took " + to_string(elapsed) + " seconds")
```

## See Also

- [Language Guide](/pixel/docs/language/basics) - Variables, types, operators, control flow
- [Math API](/pixel/docs/api/math) - Mathematical functions
- [Engine API](/pixel/docs/api/engine) - Window, drawing, sprites
