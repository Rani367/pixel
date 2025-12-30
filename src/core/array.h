#ifndef PH_ARRAY_H
#define PH_ARRAY_H

#include "common.h"

// Define a dynamic array type for a given element type
#define PH_ARRAY_DEFINE(name, type) \
    typedef struct { \
        type* data; \
        int count; \
        int capacity; \
    } name

// Initialize an array to empty state
#define ph_array_init(arr) \
    do { \
        (arr)->data = NULL; \
        (arr)->count = 0; \
        (arr)->capacity = 0; \
    } while (0)

// Free an array's memory
#define ph_array_free(arr) \
    do { \
        PH_FREE((arr)->data); \
        ph_array_init(arr); \
    } while (0)

// Push a value onto the end of the array
#define ph_array_push(arr, value) \
    do { \
        if ((arr)->count >= (arr)->capacity) { \
            int new_cap = PH_GROW_CAPACITY((arr)->capacity); \
            (arr)->data = PH_REALLOC((arr)->data, sizeof(*(arr)->data) * new_cap); \
            (arr)->capacity = new_cap; \
        } \
        (arr)->data[(arr)->count] = (value); \
        (arr)->count++; \
    } while (0)

// Pop a value from the end of the array
#define ph_array_pop(arr) \
    ((arr)->data[--(arr)->count])

// Get the last element without removing it
#define ph_array_last(arr) \
    ((arr)->data[(arr)->count - 1])

// Check if array is empty
#define ph_array_is_empty(arr) \
    ((arr)->count == 0)

// Clear the array (keep capacity)
#define ph_array_clear(arr) \
    do { \
        (arr)->count = 0; \
    } while (0)

// Reserve capacity for at least n elements
#define ph_array_reserve(arr, n) \
    do { \
        if ((arr)->capacity < (n)) { \
            (arr)->data = PH_REALLOC((arr)->data, sizeof(*(arr)->data) * (n)); \
            (arr)->capacity = (n); \
        } \
    } while (0)

// Common array type definitions
PH_ARRAY_DEFINE(ByteArray, uint8_t);
PH_ARRAY_DEFINE(IntArray, int);
PH_ARRAY_DEFINE(PtrArray, void*);

#endif // PH_ARRAY_H
