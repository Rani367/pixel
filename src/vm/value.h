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
    ValueType type;
    union {
        bool boolean;
        double number;
        Object* object;
    } as;
} Value;

// Value constructors
#define NONE_VAL          ((Value){VAL_NONE, {.number = 0}})
#define BOOL_VAL(b)       ((Value){VAL_BOOL, {.boolean = (b)}})
#define NUMBER_VAL(n)     ((Value){VAL_NUMBER, {.number = (n)}})
#define OBJECT_VAL(o)     ((Value){VAL_OBJECT, {.object = (Object*)(o)}})

// Type checking macros
#define IS_NONE(v)        ((v).type == VAL_NONE)
#define IS_BOOL(v)        ((v).type == VAL_BOOL)
#define IS_NUMBER(v)      ((v).type == VAL_NUMBER)
#define IS_OBJECT(v)      ((v).type == VAL_OBJECT)

// Value extraction macros
#define AS_BOOL(v)        ((v).as.boolean)
#define AS_NUMBER(v)      ((v).as.number)
#define AS_OBJECT(v)      ((v).as.object)

// Value operations
bool values_equal(Value a, Value b);
void value_print(Value value);
uint32_t value_hash(Value value);

// Truthiness (none and false are falsey, everything else is truthy)
bool value_is_truthy(Value value);

// Value array (dynamic array for constant pools, etc.)
typedef struct {
    Value* values;
    int count;
    int capacity;
} ValueArray;

void value_array_init(ValueArray* array);
void value_array_write(ValueArray* array, Value value);
void value_array_free(ValueArray* array);

#endif // PH_VALUE_H
