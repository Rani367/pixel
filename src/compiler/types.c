#include "compiler/types.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// Singleton Primitive Types
// ============================================================================

static Type _type_num = { .kind = TY_NUM };
static Type _type_int = { .kind = TY_INT };
static Type _type_str = { .kind = TY_STR };
static Type _type_bool = { .kind = TY_BOOL };
static Type _type_none = { .kind = TY_NONE };
static Type _type_any = { .kind = TY_ANY };
static Type _type_error = { .kind = TY_ERROR };

Type* type_num(void) { return &_type_num; }
Type* type_int(void) { return &_type_int; }
Type* type_str(void) { return &_type_str; }
Type* type_bool(void) { return &_type_bool; }
Type* type_none(void) { return &_type_none; }
Type* type_any(void) { return &_type_any; }
Type* type_error(void) { return &_type_error; }

// ============================================================================
// Type Constructors
// ============================================================================

Type* type_list(Arena* arena, Type* element) {
    Type* type = arena_alloc(arena, sizeof(Type));
    type->kind = TY_LIST;
    type->as.list.element = element;
    return type;
}

Type* type_struct(Arena* arena, const char* name, int name_length,
                  Type** field_types, const char** field_names, int field_count) {
    Type* type = arena_alloc(arena, sizeof(Type));
    type->kind = TY_STRUCT;
    type->as.struct_type.name = name;
    type->as.struct_type.name_length = name_length;
    type->as.struct_type.field_types = field_types;
    type->as.struct_type.field_names = field_names;
    type->as.struct_type.field_count = field_count;
    return type;
}

Type* type_func(Arena* arena, Type** param_types, int param_count, Type* return_type) {
    Type* type = arena_alloc(arena, sizeof(Type));
    type->kind = TY_FUNC;
    type->as.func.param_types = param_types;
    type->as.func.param_count = param_count;
    type->as.func.return_type = return_type ? return_type : type_none();
    return type;
}

// ============================================================================
// Type Utilities
// ============================================================================

const char* type_kind_name(TypeKind kind) {
    static const char* names[] = {
        [TY_NUM] = "num",
        [TY_INT] = "int",
        [TY_STR] = "str",
        [TY_BOOL] = "bool",
        [TY_NONE] = "none",
        [TY_LIST] = "list",
        [TY_STRUCT] = "struct",
        [TY_FUNC] = "func",
        [TY_ANY] = "any",
        [TY_ERROR] = "error",
    };
    if (kind >= 0 && kind < TY_COUNT) {
        return names[kind];
    }
    return "unknown";
}

const char* type_to_string(Arena* arena, Type* type) {
    if (!type) return "none";

    switch (type->kind) {
        case TY_NUM:   return "num";
        case TY_INT:   return "int";
        case TY_STR:   return "str";
        case TY_BOOL:  return "bool";
        case TY_NONE:  return "none";
        case TY_ANY:   return "any";
        case TY_ERROR: return "error";

        case TY_LIST: {
            const char* elem = type_to_string(arena, type->as.list.element);
            int len = snprintf(NULL, 0, "list<%s>", elem);
            char* buf = arena_alloc(arena, len + 1);
            snprintf(buf, len + 1, "list<%s>", elem);
            return buf;
        }

        case TY_STRUCT: {
            // Copy name to ensure null termination
            char* buf = arena_alloc(arena, type->as.struct_type.name_length + 1);
            memcpy(buf, type->as.struct_type.name, type->as.struct_type.name_length);
            buf[type->as.struct_type.name_length] = '\0';
            return buf;
        }

        case TY_FUNC: {
            // Build function type string: func(T1, T2) -> R
            int len = 5;  // "func("
            for (int i = 0; i < type->as.func.param_count; i++) {
                if (i > 0) len += 2;  // ", "
                len += strlen(type_to_string(arena, type->as.func.param_types[i]));
            }
            len += 1;  // ")"

            if (type->as.func.return_type && type->as.func.return_type->kind != TY_NONE) {
                len += 4;  // " -> "
                len += strlen(type_to_string(arena, type->as.func.return_type));
            }

            char* buf = arena_alloc(arena, len + 1);
            int remaining = len + 1;
            char* p = buf;

            int written = snprintf(p, remaining, "func(");
            p += written;
            remaining -= written;

            for (int i = 0; i < type->as.func.param_count; i++) {
                if (i > 0) {
                    written = snprintf(p, remaining, ", ");
                    p += written;
                    remaining -= written;
                }
                written = snprintf(p, remaining, "%s", type_to_string(arena, type->as.func.param_types[i]));
                p += written;
                remaining -= written;
            }

            written = snprintf(p, remaining, ")");
            p += written;
            remaining -= written;

            if (type->as.func.return_type && type->as.func.return_type->kind != TY_NONE) {
                snprintf(p, remaining, " -> %s", type_to_string(arena, type->as.func.return_type));
            }

            return buf;
        }

        default:
            return "unknown";
    }
}

bool types_equal(Type* a, Type* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;

    switch (a->kind) {
        case TY_NUM:
        case TY_INT:
        case TY_STR:
        case TY_BOOL:
        case TY_NONE:
        case TY_ANY:
        case TY_ERROR:
            return true;  // Singletons, same kind means equal

        case TY_LIST:
            return types_equal(a->as.list.element, b->as.list.element);

        case TY_STRUCT:
            // Structs are equal if they have the same name
            if (a->as.struct_type.name_length != b->as.struct_type.name_length) return false;
            return memcmp(a->as.struct_type.name, b->as.struct_type.name,
                          a->as.struct_type.name_length) == 0;

        case TY_FUNC:
            if (a->as.func.param_count != b->as.func.param_count) return false;
            if (!types_equal(a->as.func.return_type, b->as.func.return_type)) return false;
            for (int i = 0; i < a->as.func.param_count; i++) {
                if (!types_equal(a->as.func.param_types[i], b->as.func.param_types[i])) {
                    return false;
                }
            }
            return true;

        default:
            return false;
    }
}

bool types_compatible(Type* expected, Type* actual) {
    // 'any' is compatible with everything
    if (expected->kind == TY_ANY || actual->kind == TY_ANY) return true;

    // Error type is compatible with everything (to allow error recovery)
    if (expected->kind == TY_ERROR || actual->kind == TY_ERROR) return true;

    // num and int are compatible with each other (implicit conversion)
    if (type_is_numeric(expected) && type_is_numeric(actual)) return true;

    // Otherwise, require exact equality
    return types_equal(expected, actual);
}

bool type_is_primitive(Type* type) {
    if (!type) return false;
    return type->kind == TY_NUM ||
           type->kind == TY_INT ||
           type->kind == TY_STR ||
           type->kind == TY_BOOL;
}

bool type_is_numeric(Type* type) {
    if (!type) return false;
    return type->kind == TY_NUM || type->kind == TY_INT;
}

const char* type_to_c_type(Type* type) {
    if (!type) return "void";

    switch (type->kind) {
        case TY_NUM:   return "double";
        case TY_INT:   return "int32_t";
        case TY_STR:   return "PxString*";
        case TY_BOOL:  return "bool";
        case TY_NONE:  return "void";
        case TY_ANY:   return "PxValue";  // Boxed value for dynamic typing
        case TY_ERROR: return "void";

        case TY_LIST:
            // Lists are specialized by element type
            // The caller should use type_to_c_list_type() for more specific types
            return "PxList*";

        case TY_STRUCT:
            // Struct pointer - caller should prefix with Px
            return "void*";  // Placeholder, actual codegen handles this

        case TY_FUNC:
            // Function pointer - caller handles actual signature
            return "void*";  // Placeholder

        default:
            return "void";
    }
}

const char* type_c_default_value(Type* type) {
    if (!type) return "0";

    switch (type->kind) {
        case TY_NUM:   return "0.0";
        case TY_INT:   return "0";
        case TY_STR:   return "NULL";
        case TY_BOOL:  return "false";
        case TY_NONE:  return "0";
        case TY_ANY:   return "PX_NONE";
        case TY_LIST:  return "NULL";
        case TY_STRUCT: return "NULL";
        case TY_FUNC:  return "NULL";
        default:       return "0";
    }
}
