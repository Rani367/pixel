#ifndef PX_RUNTIME_H
#define PX_RUNTIME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// Pixel AOT Runtime Library
//
// Minimal runtime for AOT-compiled Pixel programs.
// Provides memory management, strings, lists, and basic I/O.
// ============================================================================

// ============================================================================
// Memory Management
// ============================================================================

// Object header for reference counting
typedef struct {
    uint32_t refcount;
    uint16_t type_id;
} PxHeader;

// Type IDs for runtime type checking
typedef enum {
    PX_TYPE_STRING = 1,
    PX_TYPE_LIST,
    PX_TYPE_STRUCT,
    PX_TYPE_CLOSURE,
} PxTypeId;

// Allocate memory with reference counting header
void* px_alloc(size_t size);

// Free memory
void px_free(void* ptr);

// Retain (increment refcount)
#define PX_RETAIN(obj) ((obj) ? (((PxHeader*)(obj))->refcount++, (obj)) : NULL)

// Release (decrement refcount, free if zero)
void px_release(void* obj);

// ============================================================================
// Dynamic Value Type (for 'any' type)
// ============================================================================

typedef enum {
    PX_VAL_NONE,
    PX_VAL_BOOL,
    PX_VAL_NUM,
    PX_VAL_INT,
    PX_VAL_STR,
    PX_VAL_LIST,
    PX_VAL_STRUCT,
    PX_VAL_FUNC,
} PxValueType;

typedef struct {
    PxValueType type;
    union {
        bool b;
        double num;
        int32_t i;
        void* obj;  // PxString*, PxList*, etc.
    } as;
} PxValue;

#define PX_NONE ((PxValue){ .type = PX_VAL_NONE })
#define PX_BOOL(v) ((PxValue){ .type = PX_VAL_BOOL, .as.b = (v) })
#define PX_NUM(v) ((PxValue){ .type = PX_VAL_NUM, .as.num = (v) })
#define PX_INT(v) ((PxValue){ .type = PX_VAL_INT, .as.i = (v) })

// ============================================================================
// Strings
// ============================================================================

typedef struct {
    PxHeader header;
    int length;
    char chars[];  // Flexible array member
} PxString;

// Create a new string from chars
PxString* px_string_new(const char* chars, int length);

// Create string from C string literal
PxString* px_string_from_cstr(const char* cstr);

// Concatenate two strings
PxString* px_string_concat(PxString* a, PxString* b);

// Get string length
int px_string_len(PxString* s);

// Compare strings for equality
bool px_string_equal(PxString* a, PxString* b);

// Convert number to string
PxString* px_string_from_num(double n);

// Convert int to string
PxString* px_string_from_int(int32_t n);

// Get substring
PxString* px_string_substring(PxString* s, int start, int end);

// Convert to uppercase
PxString* px_string_upper(PxString* s);

// Convert to lowercase
PxString* px_string_lower(PxString* s);

// ============================================================================
// Lists (Generic via macros)
// ============================================================================

// Generic list structure
typedef struct {
    PxHeader header;
    int count;
    int capacity;
    size_t element_size;
    void* data;
} PxList;

// Create a new list
PxList* px_list_new(size_t element_size);

// Get list length
int px_list_len(PxList* list);

// Push element to list
void px_list_push(PxList* list, const void* element);

// Pop element from list
void px_list_pop(PxList* list, void* out);

// Get element at index
void px_list_get(PxList* list, int index, void* out);

// Set element at index
void px_list_set(PxList* list, int index, const void* element);

// ============================================================================
// Specialized Lists (for common types)
// ============================================================================

// List of doubles
typedef struct {
    PxHeader header;
    int count;
    int capacity;
    double* data;
} PxList_num;

PxList_num* PxList_num_new(void);
PxList_num* PxList_num_from(int count, ...);
void PxList_num_push(PxList_num* list, double value);
double PxList_num_pop(PxList_num* list);
double PxList_num_get(PxList_num* list, int index);
void PxList_num_set(PxList_num* list, int index, double value);
int PxList_num_len(PxList_num* list);

// List of integers
typedef struct {
    PxHeader header;
    int count;
    int capacity;
    int32_t* data;
} PxList_int;

PxList_int* PxList_int_new(void);
PxList_int* PxList_int_from(int count, ...);
void PxList_int_push(PxList_int* list, int32_t value);
int32_t PxList_int_pop(PxList_int* list);
int32_t PxList_int_get(PxList_int* list, int index);
void PxList_int_set(PxList_int* list, int index, int32_t value);
int PxList_int_len(PxList_int* list);

// List of strings
typedef struct {
    PxHeader header;
    int count;
    int capacity;
    PxString** data;
} PxList_str;

PxList_str* PxList_str_new(void);
PxList_str* PxList_str_from(int count, ...);
void PxList_str_push(PxList_str* list, PxString* value);
PxString* PxList_str_pop(PxList_str* list);
PxString* PxList_str_get(PxList_str* list, int index);
void PxList_str_set(PxList_str* list, int index, PxString* value);
int PxList_str_len(PxList_str* list);

// ============================================================================
// Vec2 (2D Vector)
// ============================================================================

typedef struct {
    double x;
    double y;
} PxVec2;

PxVec2 px_vec2(double x, double y);
PxVec2 px_vec2_add(PxVec2 a, PxVec2 b);
PxVec2 px_vec2_sub(PxVec2 a, PxVec2 b);
PxVec2 px_vec2_mul(PxVec2 v, double s);
double px_vec2_len(PxVec2 v);
PxVec2 px_vec2_normalize(PxVec2 v);

// ============================================================================
// I/O Functions
// ============================================================================

// Print string (no newline)
void px_print(PxString* s);

// Print string with newline
void px_println(PxString* s);

// Print number
void px_print_num(double n);

// ============================================================================
// Math Functions
// ============================================================================

double px_abs(double x);
double px_floor(double x);
double px_ceil(double x);
double px_round(double x);
double px_min(double a, double b);
double px_max(double a, double b);
double px_clamp(double x, double min_val, double max_val);
double px_sqrt(double x);
double px_pow(double base, double exp);
double px_sin(double x);
double px_cos(double x);
double px_tan(double x);
double px_atan2(double y, double x);
double px_random(void);
double px_random_range(double min_val, double max_val);
int32_t px_random_int(int32_t min_val, int32_t max_val);

// ============================================================================
// Type Conversion
// ============================================================================

PxString* px_to_string(PxValue val);
double px_to_number(PxValue val);
const char* px_type_name(PxValue val);

// ============================================================================
// Runtime Initialization
// ============================================================================

// Initialize runtime (call before any other function)
void px_init(void);

// Shutdown runtime (call at end)
void px_shutdown(void);

// ============================================================================
// Time Functions
// ============================================================================

double px_time(void);   // Unix timestamp
double px_clock(void);  // High-resolution clock for timing

#endif // PX_RUNTIME_H
