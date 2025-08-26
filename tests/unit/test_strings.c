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
