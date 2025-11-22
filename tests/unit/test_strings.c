", 5);
    ASSERT_NE(hash1, hash2);
}

int main(void) {
    TEST_SUITE("StringView");

    RUN_TEST(sv_from_cstr);
    RUN_TEST(sv_from_cstr_null);
    RUN_TEST(sv_from_parts);
    RUN_TEST(sv_equal_same);
    RUN_TEST(sv_equal_different);
    RUN_TEST(sv_equal_different_length);
    RUN_TEST(sv_starts_with);
    RUN_TEST(sv_ends_with);
    RUN_TEST(sv_trim);
    RUN_TEST(sv_trim_no_whitespace);

    TEST_SUITE("StringBuilder");

    RUN_TEST(sb_init_empty);
    RUN_TEST(sb_append);
    RUN_TEST(sb_append_char);
    RUN_TEST(sb_appendf);
    RUN_TEST(sb_clear);
    RUN_TEST(sb_finish_empty);

    TEST_SUITE("String Utilities");

    RUN_TEST(ph_strdup_basic);
    RUN_TEST(ph_strndup_basic);
    RUN_TEST(ph_hash_string_consistent);
    RUN_TEST(ph_hash_string_different);

    TEST_SUMMARY();
}
#include "../test_framework.h"
#include "core/strings.h"

TEST(sv_from_cstr) {
    StringView sv = sv_from_cstr("hello");
    ASSERT_EQ(sv.length, 5);
    ASSERT_EQ(memcmp(sv.data, "hello", 5), 0);
}

TEST(sv_from_cstr_null) {
    StringView sv = sv_from_cstr(NULL);
    ASSERT_NULL(sv.data);
    ASSERT_EQ(sv.length, 0);
}

TEST(sv_from_parts) {
    const char* str = "hello world";
    StringView sv = sv_from_parts(str + 6, 5);
    ASSERT_EQ(sv.length, 5);
    ASSERT_EQ(memcmp(sv.data, "world", 5), 0);
}

TEST(sv_equal_same) {
    StringView a = sv_from_cstr("hello");
    StringView b = sv_from_cstr("hello");
    ASSERT(sv_equal(a, b));
}

TEST(sv_equal_different) {
    StringView a = sv_from_cstr("hello");
    StringView b = sv_from_cstr("world");
    ASSERT(!sv_equal(a, b));
}

TEST(sv_equal_different_length) {
    StringView a = sv_from_cstr("hello");
    StringView b = sv_from_cstr("hello!");
    ASSERT(!sv_equal(a, b));
}

TEST(sv_starts_with) {
    StringView sv = sv_from_cstr("hello world");
    ASSERT(sv_starts_with(sv, sv_from_cstr("hello")));
    ASSERT(sv_starts_with(sv, sv_from_cstr("")));
    ASSERT(!sv_starts_with(sv, sv_from_cstr("world")));
}

TEST(sv_ends_with) {
    StringView sv = sv_from_cstr("hello world");
    ASSERT(sv_ends_with(sv, sv_from_cstr("world")));
    ASSERT(sv_ends_with(sv, sv_from_cstr("")));
    ASSERT(!sv_ends_with(sv, sv_from_cstr("hello")));
}

TEST(sv_trim) {
    StringView sv = sv_trim(sv_from_cstr("  hello  "));
    ASSERT_EQ(sv.length, 5);
    ASSERT_EQ(memcmp(sv.data, "hello", 5), 0);
}

TEST(sv_trim_no_whitespace) {
    StringView sv = sv_trim(sv_from_cstr("hello"));
    ASSERT_EQ(sv.length, 5);
}

TEST(sb_init_empty) {
    StringBuilder sb;
    sb_init(&sb);
    ASSERT_NULL(sb.data);
    ASSERT_EQ(sb.length, 0);
    ASSERT_EQ(sb.capacity, 0);
    sb_free(&sb);
}

TEST(sb_append) {
    StringBuilder sb;
    sb_init(&sb);

    sb_append(&sb, "hello");
    sb_append(&sb, " ");
    sb_append(&sb, "world");

    char* result = sb_finish(&sb);
    ASSERT_STR_EQ(result, "hello world");
    free(result);
}

TEST(sb_append_char) {
    StringBuilder sb;
    sb_init(&sb);

    sb_append_char(&sb, 'a');
    sb_append_char(&sb, 'b');
    sb_append_char(&sb, 'c');

    char* result = sb_finish(&sb);
    ASSERT_STR_EQ(result, "abc");
    free(result);
}

TEST(sb_appendf) {
    StringBuilder sb;
    sb_init(&sb);

    sb_appendf(&sb, "number: %d, string: %s", 42, "test");

    char* result = sb_finish(&sb);
    ASSERT_STR_EQ(result, "number: 42, string: test");
    free(result);
}

TEST(sb_clear) {
    StringBuilder sb;
    sb_init(&sb);

    sb_append(&sb, "hello");
    sb_clear(&sb);

    ASSERT_EQ(sb.length, 0);
    ASSERT(sb.capacity > 0);  // Capacity preserved

    sb_free(&sb);
}

TEST(sb_finish_empty) {
    StringBuilder sb;
    sb_init(&sb);

    char* result = sb_finish(&sb);
    ASSERT_NOT_NULL(result);
    ASSERT_STR_EQ(result, "");
    free(result);
}

TEST(ph_strdup_basic) {
    char* dup = ph_strdup("hello");
    ASSERT_NOT_NULL(dup);
    ASSERT_STR_EQ(dup, "hello");
    free(dup);
}

TEST(ph_strndup_basic) {
    char* dup = ph_strndup("hello world", 5);
    ASSERT_NOT_NULL(dup);
    ASSERT_STR_EQ(dup, "hello");
    free(dup);
}

TEST(ph_hash_string_consistent) {
    const char* str = "test string";
    uint32_t hash1 = ph_hash_string(str, strlen(str));
    uint32_t hash2 = ph_hash_string(str, strlen(str));
    ASSERT_EQ(hash1, hash2);
}

TEST(ph_hash_string_different) {
    uint32_t hash1 = ph_hash_string("hello", 5);
    uint32_t hash2 = ph_hash_string("world