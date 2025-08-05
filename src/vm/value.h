ward declaration for objects
typedef struct Object Object;

// Value types
typedef enum {
    VAL_NONE,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_OBJECT,
} ValueType;

// Tagged union value representation
typedef struct {
    ValueType