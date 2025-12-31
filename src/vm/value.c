#include "vm/value.h"
#include "vm/object.h"
#include <stdio.h>
#include <string.h>

bool values_equal(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
        case VAL_NONE:   return true;
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJECT: return AS_OBJECT(a) == AS_OBJECT(b);
        default:         return false;  // LCOV_EXCL_LINE
    }
}

void value_print(Value value) {
    switch (value.type) {
        case VAL_NONE:
            printf("none");
            break;
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NUMBER:
            printf("%g", AS_NUMBER(value));
            break;
        case VAL_OBJECT:
            object_print(value);
            break;
    }
}

uint32_t value_hash(Value value) {
    switch (value.type) {
        case VAL_NONE:
            return 0;
        case VAL_BOOL:
            return AS_BOOL(value) ? 1 : 0;
        case VAL_NUMBER: {
            // Hash the bits of the double
            double num = AS_NUMBER(value);
            uint64_t bits;
            memcpy(&bits, &num, sizeof(bits));
            return (uint32_t)(bits ^ (bits >> 32));
        }
        case VAL_OBJECT:
            return object_hash(value);
        default:
            return 0;  // LCOV_EXCL_LINE
    }
}

bool value_is_truthy(Value value) {
    switch (value.type) {
        case VAL_NONE:   return false;
        case VAL_BOOL:   return AS_BOOL(value);
        case VAL_NUMBER: return true;
        case VAL_OBJECT: return true;
        default:         return false;  // LCOV_EXCL_LINE
    }
}

void value_array_init(ValueArray* array) {
    array->values = NULL;
    array->count = 0;
    array->capacity = 0;
}

void value_array_write(ValueArray* array, Value value) {
    if (array->count >= array->capacity) {
        int new_capacity = PH_GROW_CAPACITY(array->capacity);
        array->values = PH_REALLOC(array->values, sizeof(Value) * new_capacity);
        array->capacity = new_capacity;
    }

    array->values[array->count++] = value;
}

void value_array_free(ValueArray* array) {
    PH_FREE(array->values);
    value_array_init(array);
}
