\
        (arr)->capacity = 0; \
    } while (0)

// Free an array's memory
#define ph_array_free(arr) \
    do { \
        PH_FREE((arr)->data); \
        ph_array_init(arr); \
    } while (0)

// Push a value onto t