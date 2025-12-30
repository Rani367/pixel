#include "runtime/stdlib.h"
#include "core/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

// ============================================================================
// Helper Functions
// ============================================================================

void define_native(VM* vm, const char* name, NativeFn function, int arity) {
    ObjString* name_str = string_copy(name, (int)strlen(name));
    ObjNative* native = native_new(function, name_str, arity);
    vm_define_global(vm, name_str, OBJECT_VAL(native));
}

// Helper for runtime errors from native functions
// Returns NONE_VAL for convenience in return statements
static Value native_error(const char* message) {
    fprintf(stderr, "Runtime error: %s\n", message);
    return NONE_VAL;
}

// ============================================================================
// I/O Functions
// ============================================================================

// print(value) - print without newline
static Value native_print(int arg_count, Value* args) {
    (void)arg_count;
    value_print(args[0]);
    return NONE_VAL;
}

// println(value) - print with newline
static Value native_println(int arg_count, Value* args) {
    (void)arg_count;
    value_print(args[0]);
    printf("\n");
    return NONE_VAL;
}

// ============================================================================
// Type Functions
// ============================================================================

// type(value) - return the type name as a string
static Value native_type(int arg_count, Value* args) {
    (void)arg_count;
    Value val = args[0];
    const char* type_name;

    if (IS_NONE(val)) {
        type_name = "none";
    } else if (IS_BOOL(val)) {
        type_name = "bool";
    } else if (IS_NUMBER(val)) {
        type_name = "number";
    } else if (IS_OBJECT(val)) {
        type_name = object_type_name(OBJ_TYPE(val));
    } else {
        type_name = "unknown";
    }

    return OBJECT_VAL(string_copy(type_name, (int)strlen(type_name)));
}

// to_string(value) - convert any value to a string
static Value native_to_string(int arg_count, Value* args) {
    (void)arg_count;
    Value val = args[0];

    // If already a string, return as-is
    if (IS_STRING(val)) {
        return val;
    }

    char buffer[256];
    int len = 0;

    if (IS_NONE(val)) {
        len = snprintf(buffer, sizeof(buffer), "none");
    } else if (IS_BOOL(val)) {
        len = snprintf(buffer, sizeof(buffer), "%s", AS_BOOL(val) ? "true" : "false");
    } else if (IS_NUMBER(val)) {
        double num = AS_NUMBER(val);
        if (num == (int)num) {
            len = snprintf(buffer, sizeof(buffer), "%d", (int)num);
        } else {
            len = snprintf(buffer, sizeof(buffer), "%g", num);
        }
    } else if (IS_VEC2(val)) {
        ObjVec2* v = AS_VEC2(val);
        len = snprintf(buffer, sizeof(buffer), "vec2(%g, %g)", v->x, v->y);
    } else if (IS_LIST(val)) {
        len = snprintf(buffer, sizeof(buffer), "<list>");
    } else if (IS_FUNCTION(val) || IS_CLOSURE(val)) {
        len = snprintf(buffer, sizeof(buffer), "<function>");
    } else if (IS_NATIVE(val)) {
        len = snprintf(buffer, sizeof(buffer), "<native fn>");
    } else if (IS_INSTANCE(val)) {
        ObjInstance* instance = AS_INSTANCE(val);
        len = snprintf(buffer, sizeof(buffer), "<%s instance>",
                      instance->struct_def->name->chars);
    } else if (IS_STRUCT_DEF(val)) {
        ObjStructDef* def = AS_STRUCT_DEF(val);
        len = snprintf(buffer, sizeof(buffer), "<struct %s>", def->name->chars);
    } else {
        len = snprintf(buffer, sizeof(buffer), "<object>");
    }

    return OBJECT_VAL(string_copy(buffer, len));
}

// to_number(string) - parse a string as a number
static Value native_to_number(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_STRING(args[0])) {
        return native_error("to_number() requires a string");
    }

    ObjString* str = AS_STRING(args[0]);
    char* end;
    double result = strtod(str->chars, &end);

    // Check if the entire string was consumed
    if (end == str->chars || *end != '\0') {
        return NONE_VAL;  // Return none for invalid numbers
    }

    return NUMBER_VAL(result);
}

// ============================================================================
// Math Functions
// ============================================================================

static Value native_abs(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0])) {
        return native_error("abs() requires a number");
    }
    return NUMBER_VAL(fabs(AS_NUMBER(args[0])));
}

static Value native_floor(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0])) {
        return native_error("floor() requires a number");
    }
    return NUMBER_VAL(floor(AS_NUMBER(args[0])));
}

static Value native_ceil(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0])) {
        return native_error("ceil() requires a number");
    }
    return NUMBER_VAL(ceil(AS_NUMBER(args[0])));
}

static Value native_round(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0])) {
        return native_error("round() requires a number");
    }
    return NUMBER_VAL(round(AS_NUMBER(args[0])));
}

static Value native_min(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        return native_error("min() requires two numbers");
    }
    double a = AS_NUMBER(args[0]);
    double b = AS_NUMBER(args[1]);
    return NUMBER_VAL(a < b ? a : b);
}

static Value native_max(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        return native_error("max() requires two numbers");
    }
    double a = AS_NUMBER(args[0]);
    double b = AS_NUMBER(args[1]);
    return NUMBER_VAL(a > b ? a : b);
}

static Value native_clamp(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("clamp() requires three numbers");
    }
    double x = AS_NUMBER(args[0]);
    double min_val = AS_NUMBER(args[1]);
    double max_val = AS_NUMBER(args[2]);
    if (x < min_val) return NUMBER_VAL(min_val);
    if (x > max_val) return NUMBER_VAL(max_val);
    return NUMBER_VAL(x);
}

static Value native_sqrt(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0])) {
        return native_error("sqrt() requires a number");
    }
    return NUMBER_VAL(sqrt(AS_NUMBER(args[0])));
}

static Value native_pow(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        return native_error("pow() requires two numbers");
    }
    return NUMBER_VAL(pow(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

static Value native_sin(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0])) {
        return native_error("sin() requires a number");
    }
    return NUMBER_VAL(sin(AS_NUMBER(args[0])));
}

static Value native_cos(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0])) {
        return native_error("cos() requires a number");
    }
    return NUMBER_VAL(cos(AS_NUMBER(args[0])));
}

static Value native_tan(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0])) {
        return native_error("tan() requires a number");
    }
    return NUMBER_VAL(tan(AS_NUMBER(args[0])));
}

static Value native_atan2(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        return native_error("atan2() requires two numbers");
    }
    return NUMBER_VAL(atan2(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

// random() - return a random number between 0 and 1
static Value native_random(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;
    return NUMBER_VAL((double)rand() / (double)RAND_MAX);
}

// random_range(min, max) - return a random number between min and max
static Value native_random_range(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        return native_error("random_range() requires two numbers");
    }
    double min_val = AS_NUMBER(args[0]);
    double max_val = AS_NUMBER(args[1]);
    double t = (double)rand() / (double)RAND_MAX;
    return NUMBER_VAL(min_val + t * (max_val - min_val));
}

// random_int(min, max) - return a random integer between min and max (inclusive)
static Value native_random_int(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        return native_error("random_int() requires two numbers");
    }
    int min_val = (int)AS_NUMBER(args[0]);
    int max_val = (int)AS_NUMBER(args[1]);
    int range = max_val - min_val + 1;
    return NUMBER_VAL(min_val + (rand() % range));
}

// ============================================================================
// List Functions
// ============================================================================

// len(list|string) - return the length
static Value native_len(int arg_count, Value* args) {
    (void)arg_count;
    if (IS_LIST(args[0])) {
        return NUMBER_VAL(AS_LIST(args[0])->count);
    }
    if (IS_STRING(args[0])) {
        return NUMBER_VAL(AS_STRING(args[0])->length);
    }
    return native_error("len() requires a list or string");
}

// push(list, value) - append value to list
static Value native_push(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_LIST(args[0])) {
        return native_error("push() requires a list as first argument");
    }
    ObjList* list = AS_LIST(args[0]);
    list_append(list, args[1]);
    return NONE_VAL;
}

// pop(list) - remove and return the last element
static Value native_pop(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_LIST(args[0])) {
        return native_error("pop() requires a list");
    }
    ObjList* list = AS_LIST(args[0]);
    if (list->count == 0) {
        return native_error("pop() on empty list");
    }
    Value result = list->items[list->count - 1];
    list->count--;
    return result;
}

// insert(list, index, value) - insert value at index
static Value native_insert(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_LIST(args[0])) {
        return native_error("insert() requires a list as first argument");
    }
    if (!IS_NUMBER(args[1])) {
        return native_error("insert() requires a number as second argument");
    }
    ObjList* list = AS_LIST(args[0]);
    int index = (int)AS_NUMBER(args[1]);

    // Handle negative indices
    if (index < 0) index = list->count + index + 1;

    if (index < 0 || index > list->count) {
        return native_error("insert() index out of bounds");
    }

    // Grow if needed (reuse list_append logic for capacity growth)
    list_append(list, NONE_VAL);

    // Shift elements right
    for (int i = list->count - 1; i > index; i--) {
        list->items[i] = list->items[i - 1];
    }

    list->items[index] = args[2];
    return NONE_VAL;
}

// remove(list, index) - remove element at index
static Value native_remove(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_LIST(args[0])) {
        return native_error("remove() requires a list as first argument");
    }
    if (!IS_NUMBER(args[1])) {
        return native_error("remove() requires a number as second argument");
    }
    ObjList* list = AS_LIST(args[0]);
    int index = (int)AS_NUMBER(args[1]);

    // Handle negative indices
    if (index < 0) index = list->count + index;

    if (index < 0 || index >= list->count) {
        return native_error("remove() index out of bounds");
    }

    Value result = list->items[index];

    // Shift elements left
    for (int i = index; i < list->count - 1; i++) {
        list->items[i] = list->items[i + 1];
    }
    list->count--;

    return result;
}

// contains(list, value) - check if list contains value
static Value native_contains(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_LIST(args[0])) {
        return native_error("contains() requires a list as first argument");
    }
    ObjList* list = AS_LIST(args[0]);
    Value needle = args[1];

    for (int i = 0; i < list->count; i++) {
        if (values_equal(list->items[i], needle)) {
            return BOOL_VAL(true);
        }
    }
    return BOOL_VAL(false);
}

// index_of(list, value) - find index of value, or -1 if not found
static Value native_index_of(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_LIST(args[0])) {
        return native_error("index_of() requires a list as first argument");
    }
    ObjList* list = AS_LIST(args[0]);
    Value needle = args[1];

    for (int i = 0; i < list->count; i++) {
        if (values_equal(list->items[i], needle)) {
            return NUMBER_VAL(i);
        }
    }
    return NUMBER_VAL(-1);
}

// ============================================================================
// String Functions
// ============================================================================

// substring(string, start, end) - extract substring
static Value native_substring(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_STRING(args[0])) {
        return native_error("substring() requires a string as first argument");
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("substring() requires numbers for start and end");
    }

    ObjString* str = AS_STRING(args[0]);
    int len = (int)str->length;
    int start = (int)AS_NUMBER(args[1]);
    int end = (int)AS_NUMBER(args[2]);

    // Handle negative indices
    if (start < 0) start = len + start;
    if (end < 0) end = len + end;

    // Clamp to valid range
    if (start < 0) start = 0;
    if (end > len) end = len;
    if (start >= end) {
        return OBJECT_VAL(string_copy("", 0));
    }

    return OBJECT_VAL(string_copy(str->chars + start, end - start));
}

// split(string, delimiter) - split string by delimiter
static Value native_split(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        return native_error("split() requires two strings");
    }

    ObjString* str = AS_STRING(args[0]);
    ObjString* delim = AS_STRING(args[1]);
    ObjList* result = list_new();

    if (delim->length == 0) {
        // Split into individual characters
        for (uint32_t i = 0; i < str->length; i++) {
            ObjString* ch = string_copy(&str->chars[i], 1);
            list_append(result, OBJECT_VAL(ch));
        }
    } else {
        const char* start = str->chars;
        const char* end = str->chars + str->length;
        const char* pos;

        while ((pos = strstr(start, delim->chars)) != NULL && start < end) {
            ObjString* part = string_copy(start, (int)(pos - start));
            list_append(result, OBJECT_VAL(part));
            start = pos + delim->length;
        }

        // Add the remaining part
        ObjString* part = string_copy(start, (int)(end - start));
        list_append(result, OBJECT_VAL(part));
    }

    return OBJECT_VAL(result);
}

// join(list, delimiter) - join list elements with delimiter
static Value native_join(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_LIST(args[0])) {
        return native_error("join() requires a list as first argument");
    }
    if (!IS_STRING(args[1])) {
        return native_error("join() requires a string as second argument");
    }

    ObjList* list = AS_LIST(args[0]);
    ObjString* delim = AS_STRING(args[1]);

    if (list->count == 0) {
        return OBJECT_VAL(string_copy("", 0));
    }

    // Calculate total length needed
    size_t total_len = 0;
    for (int i = 0; i < list->count; i++) {
        if (!IS_STRING(list->items[i])) {
            return native_error("join() list must contain only strings");
        }
        total_len += AS_STRING(list->items[i])->length;
        if (i > 0) total_len += delim->length;
    }

    // Build the result
    char* buffer = PH_ALLOC(total_len + 1);
    char* ptr = buffer;

    for (int i = 0; i < list->count; i++) {
        if (i > 0) {
            memcpy(ptr, delim->chars, delim->length);
            ptr += delim->length;
        }
        ObjString* s = AS_STRING(list->items[i]);
        memcpy(ptr, s->chars, s->length);
        ptr += s->length;
    }
    *ptr = '\0';

    ObjString* result = string_take(buffer, (int)total_len);
    return OBJECT_VAL(result);
}

// upper(string) - convert to uppercase
static Value native_upper(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_STRING(args[0])) {
        return native_error("upper() requires a string");
    }

    ObjString* str = AS_STRING(args[0]);
    char* buffer = PH_ALLOC(str->length + 1);

    for (uint32_t i = 0; i < str->length; i++) {
        buffer[i] = (char)toupper((unsigned char)str->chars[i]);
    }
    buffer[str->length] = '\0';

    return OBJECT_VAL(string_take(buffer, (int)str->length));
}

// lower(string) - convert to lowercase
static Value native_lower(int arg_count, Value* args) {
    (void)arg_count;
    if (!IS_STRING(args[0])) {
        return native_error("lower() requires a string");
    }

    ObjString* str = AS_STRING(args[0]);
    char* buffer = PH_ALLOC(str->length + 1);

    for (uint32_t i = 0; i < str->length; i++) {
        buffer[i] = (char)tolower((unsigned char)str->chars[i]);
    }
    buffer[str->length] = '\0';

    return OBJECT_VAL(string_take(buffer, (int)str->length));
}

// ============================================================================
// Utility Functions
// ============================================================================

// range(stop) or range(start, stop) or range(start, stop, step) - create a list of numbers
static Value native_range(int arg_count, Value* args) {
    double start = 0, stop, step = 1;

    if (arg_count == 1) {
        if (!IS_NUMBER(args[0])) {
            return native_error("range() requires numbers");
        }
        stop = AS_NUMBER(args[0]);
    } else if (arg_count == 2) {
        if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
            return native_error("range() requires numbers");
        }
        start = AS_NUMBER(args[0]);
        stop = AS_NUMBER(args[1]);
    } else if (arg_count == 3) {
        if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
            return native_error("range() requires numbers");
        }
        start = AS_NUMBER(args[0]);
        stop = AS_NUMBER(args[1]);
        step = AS_NUMBER(args[2]);
    } else {
        return native_error("range() takes 1-3 arguments");
    }

    if (step == 0) {
        return native_error("range() step cannot be zero");
    }

    ObjList* result = list_new();

    if (step > 0) {
        for (double i = start; i < stop; i += step) {
            list_append(result, NUMBER_VAL(i));
        }
    } else {
        for (double i = start; i > stop; i += step) {
            list_append(result, NUMBER_VAL(i));
        }
    }

    return OBJECT_VAL(result);
}

// time() - return current Unix timestamp in seconds
static Value native_time(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;
    return NUMBER_VAL((double)time(NULL));
}

// clock() - return high-resolution time in seconds (for measuring elapsed time)
static Value native_clock(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

// ============================================================================
// Standard Library Initialization
// ============================================================================

void stdlib_init(VM* vm) {
    // Seed random number generator
    srand((unsigned int)time(NULL));

    // I/O functions
    define_native(vm, "print", native_print, 1);
    define_native(vm, "println", native_println, 1);

    // Type functions
    define_native(vm, "type", native_type, 1);
    define_native(vm, "to_string", native_to_string, 1);
    define_native(vm, "to_number", native_to_number, 1);

    // Math functions
    define_native(vm, "abs", native_abs, 1);
    define_native(vm, "floor", native_floor, 1);
    define_native(vm, "ceil", native_ceil, 1);
    define_native(vm, "round", native_round, 1);
    define_native(vm, "min", native_min, 2);
    define_native(vm, "max", native_max, 2);
    define_native(vm, "clamp", native_clamp, 3);
    define_native(vm, "sqrt", native_sqrt, 1);
    define_native(vm, "pow", native_pow, 2);
    define_native(vm, "sin", native_sin, 1);
    define_native(vm, "cos", native_cos, 1);
    define_native(vm, "tan", native_tan, 1);
    define_native(vm, "atan2", native_atan2, 2);
    define_native(vm, "random", native_random, 0);
    define_native(vm, "random_range", native_random_range, 2);
    define_native(vm, "random_int", native_random_int, 2);

    // List functions
    define_native(vm, "len", native_len, 1);
    define_native(vm, "push", native_push, 2);
    define_native(vm, "pop", native_pop, 1);
    define_native(vm, "insert", native_insert, 3);
    define_native(vm, "remove", native_remove, 2);
    define_native(vm, "contains", native_contains, 2);
    define_native(vm, "index_of", native_index_of, 2);

    // String functions
    define_native(vm, "substring", native_substring, 3);
    define_native(vm, "split", native_split, 2);
    define_native(vm, "join", native_join, 2);
    define_native(vm, "upper", native_upper, 1);
    define_native(vm, "lower", native_lower, 1);

    // Utility functions
    define_native(vm, "range", native_range, -1);  // Variadic: 1-3 args
    define_native(vm, "time", native_time, 0);
    define_native(vm, "clock", native_clock, 0);
}
