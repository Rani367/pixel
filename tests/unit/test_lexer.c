_EQ(lexer_scan_token(&lexer).type, TOKEN_NUMBER);      // 2
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_SEMICOLON);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_RIGHT_BRACE);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

int main(void) {
    TEST_SUITE("Lexer - Single Character Tokens");
    RUN_TEST(single_char_tokens);
    RUN_TEST(operator_tokens);

    TEST_SUITE("Lexer - Two Character Tokens");
    RUN_TEST(two_char_tokens);

    TEST_SUITE("Lexer - Keywords and Identifiers");
    RUN_TEST(keywords);
    RUN_TEST(identifiers);
    RUN_TEST(keyword_prefixes);

    TEST_SUITE("Lexer - Numbers");
    RUN_TEST(numbers);

    TEST_SUITE("Lexer - Strings");
    RUN_TEST(strings);
    RUN_TEST(string_escapes);
    RUN_TEST(unterminated_string);

    TEST_SUITE("Lexer - Comments");
    RUN_TEST(single_line_comment);
    RUN_TEST(multi_line_comment);
    RUN_TEST(unterminated_comment);

    TEST_SUITE("Lexer - Location Tracking");
    RUN_TEST(line_tracking);
    RUN_TEST(column_tracking);

    TEST_SUITE("Lexer - Error Handling");
    RUN_TEST(unexpected_character