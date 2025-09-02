#ifndef PH_STRINGS_H
#define PH_STRINGS_H

#include "common.h"

// String view (non-owning reference to a string)
typedef struct {
    const char* data;
    size_t length;
} StringView;

// Create a string view from a C string
StringView sv_from_cstr(const char* cstr);

// Create a string view from a pointer and length
StringView sv_from_parts(const char* data, size_t length);

// Check if two string views are equal
bool sv_equal(StringView a, StringView b);

// Check if a string view starts with a prefix
bool sv_starts_with(StringView sv, StringView prefix);

// Check if a string view ends with a suffix
bool sv_ends_with(StringView sv, StringView suffix);

// Trim whitespace from both ends
StringView sv_trim(StringView sv);

// String builder (for constructing strings dynamically)
typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} StringBuilder;

// Initialize a string builder
void sb_init(StringBuilder* sb);

// Initialize a string builder with initial capacity
void sb_init_with_capacity(StringBuilder* sb, size_t capacity);

// Append a C string
void sb_append(StringBuilder* sb, const char* str);

// Append a string view
void sb_append_sv(StringBuilder* sb, StringView sv);

// Append n characters from a pointer
void sb_append_n(StringBuilder* sb, const char* str, size_t n);

// Append a single character
void sb_append_char(StringBuilder* sb, char c);

// Append formatted string (printf-style)
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
void sb_appendf(StringBuilder* sb, const char* fmt, ...);

// Get the built string (null-terminated, owned by caller)
// After calling this, the builder is reset
char* sb_finish(StringBuilder* sb);

// Get a view of the current contents (valid until next modification)
StringView sb_view(StringBuilder* sb);

// Clear the builder (keep capacity)
void sb_clear(StringBuilder* sb);

// Free the builder's memory
void sb_free(StringBuilder* sb);

// Utility: duplicate a string
char* ph_strdup(const char* str);

// Utility: duplicate n characters of a string
char* ph_strndup(const char* str, size_t n);

// Utility: hash a string (FNV-1a)
uint32_t ph_hash_string(const char* str, size_t length);

#endif // PH_STRINGS_H
