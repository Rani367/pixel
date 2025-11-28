)str[i];
        hash *= 16777619u;
    }
    return hash;
}
sv, StringView prefix) {
    if (prefix.length > sv.length) {
        return false;
    }
    return memcmp(sv.data, prefix.data, prefix.length) == 0;
}

bool sv_ends_with(StringView sv, StringView suffix) {
    if (suffix.length > sv.length) {
        return false;
    }
    return memcmp(sv.data + sv.length - suffix.length, suffix.data, suffix.length) == 0;
}

StringView sv_trim(StringView sv) {
    while (sv.length > 0 && isspace((unsigned char)sv.data[0])) {
        sv.data++;
        sv.length--;
    }
    while (sv.length > 0 && isspace((unsigned char)sv.data[sv.length - 1])) {
        sv.length--;
    }
    return sv;
}

#define SB_INITIAL_CAPACITY 64

static void sb_grow(StringBuilder* sb, size_t needed) {
    if (sb->capacity >= needed) {
        return;
    }

    size_t new_capacity = sb->capacity == 0 ? SB_INITIAL_CAPACITY : sb->capacity;
    while (new_capacity < needed) {
        new_capacity *= 2;
    }

    sb->data = PH_REALLOC(sb->data, new_capacity);
    sb->capacity = new_capacity;
}

void sb_init(StringBuilder* sb) {
    sb->data = NULL;
    sb->length = 0;
    sb->capacity = 0;
}

void sb_init_with_capacity(StringBuilder* sb, size_t capacity) {
    sb_init(sb);
    sb_grow(sb, capacity);
}

void sb_append(StringBuilder* sb, const char* str) {
    if (str == NULL) {
        return;
    }
    sb_append_n(sb, str, strlen(str));
}

void sb_append_sv(StringBuilder* sb, StringView sv) {
    sb_append_n(sb, sv.data, sv.length);
}

void sb_append_n(StringBuilder* sb, const char* str, size_t n) {
    ipacity = 0;
    return result;
}

StringView sb_view(StringBuilder* sb) {
    return sv_from_parts(sb->data, sb->length);
}

void sb_clear(StringBuilder* sb) {
    sb->length = 0;
    if (sb->data != NULL) {
        sb->data[0] = '\0';
    }
}

void sb_free(StringBuilder* sb) {
    PH_FREE(sb->data);
    sb_init(sb);
}

char* ph_strdup(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    size_t len = strlen(str);
    char* result = PH_ALLOC(len + 1);
    memcpy(result, str, len + 1);
    return result;
}

char* ph_strndup(const char* str, size_t n) {
    if (str == NULL) {
        return NULL;
    }
    char* result = PH_ALLOC(n + 1);
    memcpy(result, str, n);
    result[n] = '\0';
    return result;
}

uint32_t ph_hash_string(const char* str, size_t length) {
    // FNV-1a hash
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; i++) {
        hash ^= (uint8_t#include "strings.h"
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

StringView sv_from_cstr(const char* cstr) {
    StringView sv;
    sv.data = cstr;
    sv.length = cstr ? strlen(cstr) : 0;
    return sv;
}

StringView sv_from_parts(const char* data, size_t length) {
    StringView sv;
    sv.data = data;
    sv.length = length;
    return sv;
}

bool sv_equal(StringView a, StringView b) {
    if (a.length != b.length) {
        return false;
    }
    return memcmp(a.data, b.data, a.length) == 0;
}

bool sv_starts_with(StringView 