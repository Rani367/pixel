#include "px_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>

// ============================================================================
// Memory Management
// ============================================================================

void* px_alloc(size_t size) {
    void* ptr = calloc(1, size);
    if (ptr) {
        PxHeader* header = (PxHeader*)ptr;
        header->refcount = 1;
    }
    return ptr;
}

void px_free(void* ptr) {
    free(ptr);
}

void px_release(void* obj) {
    if (!obj) return;
    PxHeader* header = (PxHeader*)obj;
    if (--header->refcount == 0) {
        // Type-specific cleanup if needed
        px_free(obj);
    }
}

// ============================================================================
// Strings
// ============================================================================

PxString* px_string_new(const char* chars, int length) {
    PxString* str = px_alloc(sizeof(PxString) + length + 1);
    if (!str) return NULL;
    str->header.type_id = PX_TYPE_STRING;
    str->length = length;
    memcpy(str->chars, chars, length);
    str->chars[length] = '\0';
    return str;
}

PxString* px_string_from_cstr(const char* cstr) {
    return px_string_new(cstr, strlen(cstr));
}

PxString* px_string_concat(PxString* a, PxString* b) {
    if (!a && !b) return px_string_new("", 0);
    if (!a) return PX_RETAIN(b);
    if (!b) return PX_RETAIN(a);

    int new_len = a->length + b->length;
    PxString* result = px_alloc(sizeof(PxString) + new_len + 1);
    if (!result) return NULL;
    result->header.type_id = PX_TYPE_STRING;
    result->length = new_len;
    memcpy(result->chars, a->chars, a->length);
    memcpy(result->chars + a->length, b->chars, b->length);
    result->chars[new_len] = '\0';
    return result;
}

int px_string_len(PxString* s) {
    return s ? s->length : 0;
}

bool px_string_equal(PxString* a, PxString* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->length != b->length) return false;
    return memcmp(a->chars, b->chars, a->length) == 0;
}

PxString* px_string_from_num(double n) {
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%g", n);
    return px_string_new(buf, len);
}

PxString* px_string_from_int(int32_t n) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%d", n);
    return px_string_new(buf, len);
}

PxString* px_string_substring(PxString* s, int start, int end) {
    if (!s) return px_string_new("", 0);
    if (start < 0) start = 0;
    if (end > s->length) end = s->length;
    if (start >= end) return px_string_new("", 0);
    return px_string_new(s->chars + start, end - start);
}

PxString* px_string_upper(PxString* s) {
    if (!s) return px_string_new("", 0);
    PxString* result = px_alloc(sizeof(PxString) + s->length + 1);
    if (!result) return NULL;
    result->header.type_id = PX_TYPE_STRING;
    result->length = s->length;
    for (int i = 0; i < s->length; i++) {
        result->chars[i] = toupper((unsigned char)s->chars[i]);
    }
    result->chars[s->length] = '\0';
    return result;
}

PxString* px_string_lower(PxString* s) {
    if (!s) return px_string_new("", 0);
    PxString* result = px_alloc(sizeof(PxString) + s->length + 1);
    if (!result) return NULL;
    result->header.type_id = PX_TYPE_STRING;
    result->length = s->length;
    for (int i = 0; i < s->length; i++) {
        result->chars[i] = tolower((unsigned char)s->chars[i]);
    }
    result->chars[s->length] = '\0';
    return result;
}

// ============================================================================
// Generic Lists
// ============================================================================

PxList* px_list_new(size_t element_size) {
    PxList* list = px_alloc(sizeof(PxList));
    if (!list) return NULL;
    list->header.type_id = PX_TYPE_LIST;
    list->count = 0;
    list->capacity = 8;
    list->element_size = element_size;
    list->data = calloc(list->capacity, element_size);
    return list;
}

int px_list_len(PxList* list) {
    return list ? list->count : 0;
}

static void px_list_grow(PxList* list) {
    int new_cap = list->capacity * 2;
    void* new_data = realloc(list->data, new_cap * list->element_size);
    if (new_data) {
        list->data = new_data;
        list->capacity = new_cap;
    }
}

void px_list_push(PxList* list, const void* element) {
    if (!list) return;
    if (list->count >= list->capacity) {
        px_list_grow(list);
    }
    memcpy((char*)list->data + list->count * list->element_size,
           element, list->element_size);
    list->count++;
}

void px_list_pop(PxList* list, void* out) {
    if (!list || list->count == 0) return;
    list->count--;
    if (out) {
        memcpy(out, (char*)list->data + list->count * list->element_size,
               list->element_size);
    }
}

void px_list_get(PxList* list, int index, void* out) {
    if (!list || !out || index < 0 || index >= list->count) return;
    memcpy(out, (char*)list->data + index * list->element_size,
           list->element_size);
}

void px_list_set(PxList* list, int index, const void* element) {
    if (!list || index < 0 || index >= list->count) return;
    memcpy((char*)list->data + index * list->element_size,
           element, list->element_size);
}

// ============================================================================
// Specialized Lists: PxList_num
// ============================================================================

PxList_num* PxList_num_new(void) {
    PxList_num* list = px_alloc(sizeof(PxList_num));
    if (!list) return NULL;
    list->header.type_id = PX_TYPE_LIST;
    list->count = 0;
    list->capacity = 8;
    list->data = calloc(list->capacity, sizeof(double));
    return list;
}

PxList_num* PxList_num_from(int count, ...) {
    PxList_num* list = PxList_num_new();
    if (!list) return NULL;
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        double val = va_arg(args, double);
        PxList_num_push(list, val);
    }
    va_end(args);
    return list;
}

void PxList_num_push(PxList_num* list, double value) {
    if (!list) return;
    if (list->count >= list->capacity) {
        int new_cap = list->capacity * 2;
        double* new_data = realloc(list->data, new_cap * sizeof(double));
        if (new_data) {
            list->data = new_data;
            list->capacity = new_cap;
        }
    }
    list->data[list->count++] = value;
}

double PxList_num_pop(PxList_num* list) {
    if (!list || list->count == 0) return 0.0;
    return list->data[--list->count];
}

double PxList_num_get(PxList_num* list, int index) {
    if (!list || index < 0 || index >= list->count) return 0.0;
    return list->data[index];
}

void PxList_num_set(PxList_num* list, int index, double value) {
    if (!list || index < 0 || index >= list->count) return;
    list->data[index] = value;
}

int PxList_num_len(PxList_num* list) {
    return list ? list->count : 0;
}

// ============================================================================
// Specialized Lists: PxList_int
// ============================================================================

PxList_int* PxList_int_new(void) {
    PxList_int* list = px_alloc(sizeof(PxList_int));
    if (!list) return NULL;
    list->header.type_id = PX_TYPE_LIST;
    list->count = 0;
    list->capacity = 8;
    list->data = calloc(list->capacity, sizeof(int32_t));
    return list;
}

PxList_int* PxList_int_from(int count, ...) {
    PxList_int* list = PxList_int_new();
    if (!list) return NULL;
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        int32_t val = va_arg(args, int32_t);
        PxList_int_push(list, val);
    }
    va_end(args);
    return list;
}

void PxList_int_push(PxList_int* list, int32_t value) {
    if (!list) return;
    if (list->count >= list->capacity) {
        int new_cap = list->capacity * 2;
        int32_t* new_data = realloc(list->data, new_cap * sizeof(int32_t));
        if (new_data) {
            list->data = new_data;
            list->capacity = new_cap;
        }
    }
    list->data[list->count++] = value;
}

int32_t PxList_int_pop(PxList_int* list) {
    if (!list || list->count == 0) return 0;
    return list->data[--list->count];
}

int32_t PxList_int_get(PxList_int* list, int index) {
    if (!list || index < 0 || index >= list->count) return 0;
    return list->data[index];
}

void PxList_int_set(PxList_int* list, int index, int32_t value) {
    if (!list || index < 0 || index >= list->count) return;
    list->data[index] = value;
}

int PxList_int_len(PxList_int* list) {
    return list ? list->count : 0;
}

// ============================================================================
// Specialized Lists: PxList_str
// ============================================================================

PxList_str* PxList_str_new(void) {
    PxList_str* list = px_alloc(sizeof(PxList_str));
    if (!list) return NULL;
    list->header.type_id = PX_TYPE_LIST;
    list->count = 0;
    list->capacity = 8;
    list->data = calloc(list->capacity, sizeof(PxString*));
    return list;
}

PxList_str* PxList_str_from(int count, ...) {
    PxList_str* list = PxList_str_new();
    if (!list) return NULL;
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        PxString* val = va_arg(args, PxString*);
        PxList_str_push(list, val);
    }
    va_end(args);
    return list;
}

void PxList_str_push(PxList_str* list, PxString* value) {
    if (!list) return;
    if (list->count >= list->capacity) {
        int new_cap = list->capacity * 2;
        PxString** new_data = realloc(list->data, new_cap * sizeof(PxString*));
        if (new_data) {
            list->data = new_data;
            list->capacity = new_cap;
        }
    }
    list->data[list->count++] = PX_RETAIN(value);
}

PxString* PxList_str_pop(PxList_str* list) {
    if (!list || list->count == 0) return NULL;
    return list->data[--list->count];  // Caller owns the reference
}

PxString* PxList_str_get(PxList_str* list, int index) {
    if (!list || index < 0 || index >= list->count) return NULL;
    return list->data[index];
}

void PxList_str_set(PxList_str* list, int index, PxString* value) {
    if (!list || index < 0 || index >= list->count) return;
    px_release(list->data[index]);
    list->data[index] = PX_RETAIN(value);
}

int PxList_str_len(PxList_str* list) {
    return list ? list->count : 0;
}

// ============================================================================
// Vec2
// ============================================================================

PxVec2 px_vec2(double x, double y) {
    return (PxVec2){ x, y };
}

PxVec2 px_vec2_add(PxVec2 a, PxVec2 b) {
    return (PxVec2){ a.x + b.x, a.y + b.y };
}

PxVec2 px_vec2_sub(PxVec2 a, PxVec2 b) {
    return (PxVec2){ a.x - b.x, a.y - b.y };
}

PxVec2 px_vec2_mul(PxVec2 v, double s) {
    return (PxVec2){ v.x * s, v.y * s };
}

double px_vec2_len(PxVec2 v) {
    return sqrt(v.x * v.x + v.y * v.y);
}

PxVec2 px_vec2_normalize(PxVec2 v) {
    double len = px_vec2_len(v);
    if (len == 0) return (PxVec2){ 0, 0 };
    return (PxVec2){ v.x / len, v.y / len };
}

// ============================================================================
// I/O Functions
// ============================================================================

void px_print(PxString* s) {
    if (s) {
        fwrite(s->chars, 1, s->length, stdout);
    }
}

void px_println(PxString* s) {
    px_print(s);
    putchar('\n');
}

void px_print_num(double n) {
    printf("%g", n);
}

// ============================================================================
// Math Functions
// ============================================================================

double px_abs(double x) { return fabs(x); }
double px_floor(double x) { return floor(x); }
double px_ceil(double x) { return ceil(x); }
double px_round(double x) { return round(x); }
double px_min(double a, double b) { return a < b ? a : b; }
double px_max(double a, double b) { return a > b ? a : b; }
double px_clamp(double x, double min_val, double max_val) {
    if (x < min_val) return min_val;
    if (x > max_val) return max_val;
    return x;
}
double px_sqrt(double x) { return sqrt(x); }
double px_pow(double base, double exp) { return pow(base, exp); }
double px_sin(double x) { return sin(x); }
double px_cos(double x) { return cos(x); }
double px_tan(double x) { return tan(x); }
double px_atan2(double y, double x) { return atan2(y, x); }

static bool rand_initialized = false;

double px_random(void) {
    if (!rand_initialized) {
        srand((unsigned int)time(NULL));
        rand_initialized = true;
    }
    return (double)rand() / RAND_MAX;
}

double px_random_range(double min_val, double max_val) {
    return min_val + px_random() * (max_val - min_val);
}

int32_t px_random_int(int32_t min_val, int32_t max_val) {
    if (!rand_initialized) {
        srand((unsigned int)time(NULL));
        rand_initialized = true;
    }
    return min_val + rand() % (max_val - min_val + 1);
}

// ============================================================================
// Type Conversion
// ============================================================================

PxString* px_to_string(PxValue val) {
    switch (val.type) {
        case PX_VAL_NONE: return px_string_from_cstr("none");
        case PX_VAL_BOOL: return px_string_from_cstr(val.as.b ? "true" : "false");
        case PX_VAL_NUM:  return px_string_from_num(val.as.num);
        case PX_VAL_INT:  return px_string_from_int(val.as.i);
        case PX_VAL_STR:  return PX_RETAIN((PxString*)val.as.obj);
        default: return px_string_from_cstr("[object]");
    }
}

double px_to_number(PxValue val) {
    switch (val.type) {
        case PX_VAL_NUM:  return val.as.num;
        case PX_VAL_INT:  return (double)val.as.i;
        case PX_VAL_BOOL: return val.as.b ? 1.0 : 0.0;
        default: return 0.0;
    }
}

const char* px_type_name(PxValue val) {
    switch (val.type) {
        case PX_VAL_NONE:   return "none";
        case PX_VAL_BOOL:   return "bool";
        case PX_VAL_NUM:    return "num";
        case PX_VAL_INT:    return "int";
        case PX_VAL_STR:    return "str";
        case PX_VAL_LIST:   return "list";
        case PX_VAL_STRUCT: return "struct";
        case PX_VAL_FUNC:   return "func";
        default: return "unknown";
    }
}

// ============================================================================
// Time Functions
// ============================================================================

double px_time(void) {
    return (double)time(NULL);
}

#ifdef _WIN32
#include <windows.h>
double px_clock(void) {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
}
#else
#include <sys/time.h>
double px_clock(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}
#endif

// ============================================================================
// Runtime Initialization
// ============================================================================

void px_init(void) {
    // Initialize random seed
    srand((unsigned int)time(NULL));
    rand_initialized = true;
}

void px_shutdown(void) {
    // Cleanup if needed
}
