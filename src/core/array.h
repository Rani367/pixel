\
        (arr)->capacity = 0; \
    } while (0)

// Free an array's memory
#define ph_array_free(arr) \
    do { \
        PH_FREE((arr)->data); \
        ph_array_init(arr); \
    } while (0)

// Push a value onto tty
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
