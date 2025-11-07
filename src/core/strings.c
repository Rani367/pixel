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
    i