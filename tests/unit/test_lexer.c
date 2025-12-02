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

    Token tok;

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_STRING);
    ASSERT_EQ(tok.length, 14);  // "hello\nworld" = 14

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_STRING);
    ASSERT_EQ(tok.length, 11);  // "tab\there" = 11

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_STRING);
    ASSERT_EQ(tok.length, 15);  // "quote\"inside" = 15

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(unterminated_string) {
    Lexer lexer;
    lexer_init(&lexer, "\"unterminated");

    Token tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_ERROR);
}

TEST(single_line_comment) {
    Lexer lexer;
    lexer_init(&lexer, "foo // this is a comment\nbar");

    Token tok;

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tok.length, 3);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tok.length, 3);
    ASSERT_EQ(tok.line, 2);

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(multi_line_comment) {
    Lexer lexer;
    lexer_init(&lexer, "foo /* this is\na multi-line\ncomment */ bar");

    Token tok;

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tok.line, 1);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tok.line, 3);

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(unterminated_comment) {
    Lexer lexer;
    lexer_init(&lexer, "foo /* this comment never ends");

    Token tok;

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_ERROR);
}

TEST(line_tracking) {
    Lexer lexer;
    lexer_init(&lexer, "foo\nbar\n  baz");

    Token tok;

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.line, 1);
    ASSERT_EQ(tok.column, 1);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.line, 2);
    ASSERT_EQ(tok.column, 1);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.line, 3);
    ASSERT_EQ(tok.column, 3);
}

TEST(column_tracking) {
    Lexer lexer;
    lexer_init(&lexer, "foo bar baz");

    Token tok;

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.column, 1);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.column, 5);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.column, 9);
}

TEST(unexpected_character) {
    Lexer lexer;
    lexer_init(&lexer, "@");

    Token tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_ERROR);
}

TEST(empty_source) {
    Lexer lexer;
    lexer_init(&lexer, "");

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(whitespace_only) {
    Lexer lexer;
    lexer_init(&lexer, "   \t\n\n   ");

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(complex_expression) {
    Lexer lexer;
    lexer_init(&lexer, "player.x = player.x + 200 * dt");

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_IDENTIFIER);  // player
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_DOT);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_IDENTIFIER);  // x
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EQUAL);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_IDENTIFIER);  // player
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_DOT);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_IDENTIFIER);  // x
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_PLUS);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_NUMBER);      // 200
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_STAR);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_IDENTIFIER);  // dt
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(function_definition) {
    Lexer lexer;
    lexer_init(&lexer, "function on_update(dt) { return dt * 2; }");

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_FUNCTION);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_IDENTIFIER);  // on_update
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_LEFT_PAREN);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_IDENTIFIER);  // dt
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_RIGHT_PAREN);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_LEFT_BRACE);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_RETURN);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_IDENTIFIER);  // dt
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_STAR);
    ASSERTchar_tokens) {
    Lexer lexer;
    lexer_init(&lexer, "== != <= >= += -= *= /= ->");

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EQUAL_EQUAL);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_BANG_EQUAL);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_LESS_EQUAL);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_GREATER_EQUAL);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_PLUS_EQUAL);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_MINUS_EQUAL);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_STAR_EQUAL);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_SLASH_EQUAL);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_ARROW);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(keywords) {
    Lexer lexer;
    lexer_init(&lexer, "and else false for function if in not null or return struct this true while break continue");

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_AND);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_ELSE);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_FALSE);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_FOR);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_FUNCTION);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_IF);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_IN);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_NOT);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_NULL);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_OR);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_RETURN);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_STRUCT);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_THIS);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_TRUE);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_WHILE);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_BREAK);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_CONTINUE);
    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(identifiers) {
    Lexer lexer;
    lexer_init(&lexer, "foo bar _private camelCase PascalCase with123numbers");

    Token tok;

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tok.length, 3);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tok.length, 3);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tok.length, 8);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tok.length, 9);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tok.length, 10);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tok.length, 14);

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(keyword_prefixes) {
    Lexer lexer;
    lexer_init(&lexer, "iffy format whilever");

    Token tok;

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);
}

TEST(numbers) {
    Lexer lexer;
    lexer_init(&lexer, "42 0 123 3.14 0.5 123.456");

    Token tok;

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_NUMBER);
    ASSERT_EQ(tok.length, 2);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_NUMBER);
    ASSERT_EQ(tok.length, 1);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_NUMBER);
    ASSERT_EQ(tok.length, 3);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_NUMBER);
    ASSERT_EQ(tok.length, 4);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_NUMBER);
    ASSERT_EQ(tok.length, 3);

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_NUMBER);
    ASSERT_EQ(tok.length, 7);

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(strings) {
    Lexer lexer;
    lexer_init(&lexer, "\"hello\" \"world\" \"with spaces\"");

    Token tok;

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_STRING);
    ASSERT_EQ(tok.length, 7);  // "hello" = 7

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_STRING);
    ASSERT_EQ(tok.length, 7);  // "world" = 7

    tok = lexer_scan_token(&lexer);
    ASSERT_EQ(tok.type, TOKEN_STRING);
    ASSERT_EQ(tok.length, 13);  // "with spaces" = 13

    ASSERT_EQ(lexer_scan_token(&lexer).type, TOKEN_EOF);
}

TEST(string_escapes) {
    Lexer lexer;
    lexer_init(&lexer, "\"hello\\nworld\" \"tab\\there\" \"quote\\\"inside\""););

    TEST_SUITE("Lexer - Edge Cases");
    RUN_TEST(empty_source);
    RUN_TEST(whitespace_only);

    TEST_SUITE("Lexer - Integration");
    RUN_TEST(complex_expression);
    RUN_TEST(function_definition);

    TEST_SUMMARY();
}
