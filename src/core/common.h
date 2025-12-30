#ifndef PH_COMMON_H
#define PH_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Assertions (disabled in release builds)
#ifdef DEBUG
#define PH_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "Assertion failed: %s at %s:%d\n", \
                    #cond, __FILE__, __LINE__); \
            abort(); \
        } \
    } while (0)
#else
#define PH_ASSERT(cond) ((void)0)
#endif

// Unreachable code marker
#if defined(__GNUC__) || defined(__clang__)
#define PH_UNREACHABLE() \
    do { \
        PH_ASSERT(false && "unreachable"); \
        __builtin_unreachable(); \
    } while (0)
#else
#define PH_UNREACHABLE() \
    do { \
        PH_ASSERT(false && "unreachable"); \
    } while (0)
#endif

// Static array length
#define PH_ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

// Unused parameter marker
#define PH_UNUSED(x) ((void)(x))

// Min/max macros
#define PH_MIN(a, b) ((a) < (b) ? (a) : (b))
#define PH_MAX(a, b) ((a) > (b) ? (a) : (b))

// Memory allocation wrappers (can be replaced for custom allocators)
#define PH_ALLOC(size) malloc(size)
#define PH_REALLOC(ptr, size) realloc(ptr, size)
#define PH_FREE(ptr) free(ptr)

// Grow capacity for dynamic arrays (factor of 2)
#define PH_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#endif // PH_COMMON_H
