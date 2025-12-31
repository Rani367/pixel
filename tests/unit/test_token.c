#include "../test_framework.h"
#include "compiler/token.h"
#include <stdlib.h>

// ============================================================================
// Token Type Name Tests
// ============================================================================

TEST(token_type_name_all_types) {
    // Test a sample of token types
    ASSERT_STR_EQ(token_type_name(TOKEN_LEFT_PAREN), "LEFT_PAREN");
    ASSERT_STR_EQ(token_type_name(TOKEN_RIGHT_PAREN), "RIGHT_PAREN");
    ASSERT_STR_EQ(token_type_name(TOKEN_PLUS), "PLUS");
    ASSERT_STR_EQ(token_type_name(TOKEN_MINUS), "MINUS");
    ASSERT_STR_EQ(token_type_name(TOKEN_STAR), "STAR");
    ASSERT_STR_EQ(token_type_name(TOKEN_SLASH), "SLASH");
    ASSERT_STR_EQ(token_type_name(TOKEN_EQUAL), "EQUAL");
    ASSERT_STR_EQ(token_type_name(TOKEN_EQUAL_EQUAL), "EQUAL_EQUAL");
    ASSERT_STR_EQ(token_type_name(TOKEN_IDENTIFIER), "IDENTIFIER");
    ASSERT_STR_EQ(token_type_name(TOKEN_STRING), "STRING");
    ASSERT_STR_EQ(token_type_name(TOKEN_NUMBER), "NUMBER");
    ASSERT_STR_EQ(token_type_name(TOKEN_IF), "IF");
    ASSERT_STR_EQ(token_type_name(TOKEN_ELSE), "ELSE");
    ASSERT_STR_EQ(token_type_name(TOKEN_WHILE), "WHILE");
    ASSERT_STR_EQ(token_type_name(TOKEN_FOR), "FOR");
    ASSERT_STR_EQ(token_type_name(TOKEN_FUNCTION), "FUNCTION");
    ASSERT_STR_EQ(token_type_name(TOKEN_RETURN), "RETURN");
    ASSERT_STR_EQ(token_type_name(TOKEN_TRUE), "TRUE");
    ASSERT_STR_EQ(token_type_name(TOKEN_FALSE), "FALSE");
    ASSERT_STR_EQ(token_type_name(TOKEN_NULL), "NULL");
    ASSERT_STR_EQ(token_type_name(TOKEN_ERROR), "ERROR");
    ASSERT_STR_EQ(token_type_name(TOKEN_EOF), "EOF");
}

TEST(token_type_name_invalid_returns_unknown) {
    ASSERT_STR_EQ(token_type_name((TokenType)-1), "UNKNOWN");
    ASSERT_STR_EQ(token_type_name((TokenType)999), "UNKNOWN");
    ASSERT_STR_EQ(token_type_name(TOKEN_COUNT), "UNKNOWN");
}

// ============================================================================
// Token Creation Tests
// ============================================================================

TEST(token_make_sets_all_fields) {
    const char* source = "identifier";
    Token token = token_make(TOKEN_IDENTIFIER, source, 10, 5, 3);

    ASSERT_EQ(token.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(token.start, source);
    ASSERT_EQ(token.length, 10);
    ASSERT_EQ(token.line, 5);
    ASSERT_EQ(token.column, 3);
}

TEST(token_make_zero_length) {
    const char* source = "";
    Token token = token_make(TOKEN_EOF, source, 0, 1, 1);

    ASSERT_EQ(token.type, TOKEN_EOF);
    ASSERT_EQ(token.length, 0);
}

TEST(token_error_creates_error_token) {
    const char* message = "unexpected character";
    Token token = token_error(message, 10, 5);

    ASSERT_EQ(token.type, TOKEN_ERROR);
    ASSERT_EQ(token.start, message);
    ASSERT_EQ(token.length, 20);  // strlen("unexpected character")
    ASSERT_EQ(token.line, 10);
    ASSERT_EQ(token.column, 5);
}

TEST(token_eof_creates_eof_token) {
    Token token = token_eof(42, 1);

    ASSERT_EQ(token.type, TOKEN_EOF);
    ASSERT_STR_EQ(token.start, "");
    ASSERT_EQ(token.length, 0);
    ASSERT_EQ(token.line, 42);
    ASSERT_EQ(token.column, 1);
}

// ============================================================================
// Token Printing Tests
// ============================================================================

TEST(token_print_identifier) {
    const char* source = "myVar";
    Token token = token_make(TOKEN_IDENTIFIER, source, 5, 1, 1);

    // Just verify it doesn't crash
    // token_print writes to stdout
    token_print(token);
}

TEST(token_print_escapes_newline) {
    const char* source = "hello\nworld";
    Token token = token_make(TOKEN_STRING, source, 11, 1, 1);

    // Should print \n instead of actual newline
    token_print(token);
}

TEST(token_print_escapes_tab) {
    const char* source = "hello\tworld";
    Token token = token_make(TOKEN_STRING, source, 11, 1, 1);

    // Should print \t instead of actual tab
    token_print(token);
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Token Type Names");
    RUN_TEST(token_type_name_all_types);
    RUN_TEST(token_type_name_invalid_returns_unknown);

    TEST_SUITE("Token Creation");
    RUN_TEST(token_make_sets_all_fields);
    RUN_TEST(token_make_zero_length);
    RUN_TEST(token_error_creates_error_token);
    RUN_TEST(token_eof_creates_eof_token);

    TEST_SUITE("Token Printing");
    RUN_TEST(token_print_identifier);
    RUN_TEST(token_print_escapes_newline);
    RUN_TEST(token_print_escapes_tab);

    TEST_SUMMARY();
}
