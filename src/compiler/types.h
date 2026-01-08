#ifndef PX_TYPES_H
#define PX_TYPES_H

#include "core/arena.h"
#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// Type System for Pixel Static (AOT Compilation)
// ============================================================================

typedef struct Type Type;

// Type kinds
typedef enum {
    TY_NUM,      // double (floating point)
    TY_INT,      // int32_t (integer)
    TY_STR,      // PxString* (string)
    TY_BOOL,     // bool
    TY_NONE,     // void/none (for function returns)
    TY_LIST,     // list<T>
    TY_STRUCT,   // User-defined struct
    TY_FUNC,     // Function type
    TY_ANY,      // Dynamic 'any' type (escape hatch)
    TY_ERROR,    // Error type (for error recovery)

    TY_COUNT
} TypeKind;

// Type representation
struct Type {
    TypeKind kind;
    union {
        // TY_LIST: element type
        struct {
            Type* element;
        } list;

        // TY_STRUCT: struct info
        struct {
            const char* name;
            int name_length;
            Type** field_types;
            const char** field_names;
            int field_count;
        } struct_type;

        // TY_FUNC: function signature
        struct {
            Type** param_types;
            int param_count;
            Type* return_type;  // NULL means TY_NONE
        } func;
    } as;
};

// ============================================================================
// Type Constructors
// ============================================================================

// Create primitive types (singleton instances)
Type* type_num(void);
Type* type_int(void);
Type* type_str(void);
Type* type_bool(void);
Type* type_none(void);
Type* type_any(void);
Type* type_error(void);

// Create composite types (arena-allocated)
Type* type_list(Arena* arena, Type* element);
Type* type_struct(Arena* arena, const char* name, int name_length,
                  Type** field_types, const char** field_names, int field_count);
Type* type_func(Arena* arena, Type** param_types, int param_count, Type* return_type);

// ============================================================================
// Type Utilities
// ============================================================================

// Get type kind name as string
const char* type_kind_name(TypeKind kind);

// Format type as human-readable string (arena-allocated)
const char* type_to_string(Arena* arena, Type* type);

// Check if two types are compatible (for assignment, etc.)
bool types_compatible(Type* expected, Type* actual);

// Check if types are exactly equal
bool types_equal(Type* a, Type* b);

// Check if type is a primitive (num, int, str, bool)
bool type_is_primitive(Type* type);

// Check if type is numeric (num or int)
bool type_is_numeric(Type* type);

// Get the C type string for code generation
const char* type_to_c_type(Type* type);

// Get the C default value for a type
const char* type_c_default_value(Type* type);

#endif // PX_TYPES_H
