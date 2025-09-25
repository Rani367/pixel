lit into individual characters
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
