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
    RUN_TEST(unexpected_character#include "../test_framework.h"
#include "compiler/lexer.h"

TEST(single_char_tokens) {
    Lexer lexer;
    lexer_init(&lexer, "( ) { } [ ] , . ; : %");

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_LEFT_PAREN);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_RIGHT_PAREN);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_LEFT_BRACE);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_RIGHT_BRACE);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_LEFT_BRACKET);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_RIGHT_BRACKET);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_COMMA);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_DOT);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_SEMICOLON);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_COLON);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_PERCENT);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(operator_tokens) {
    Lexer lexer;
    lexer_init(&lexer, "+ - * / = ! < >");

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_PLUS);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_MINUS);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_STAR);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_SLASH);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EQUAL);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_BANG);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_LESS);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_GREATER);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(two_