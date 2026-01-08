#include "lexer.h"
#include <string.h>

void lexer_init(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 1;
    lexer->start_column = 1;
    lexer->error = NULL;
}

static bool is_at_end(Lexer* lexer) {
    return *lexer->current == '\0';
}

static char advance(Lexer* lexer) {
    char c = *lexer->current;
    lexer->current++;
    lexer->column++;
    return c;
}

static char peek(Lexer* lexer) {
    return *lexer->current;
}

static char peek_next(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->current[1];
}

static bool match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (*lexer->current != expected) return false;
    lexer->current++;
    lexer->column++;
    return true;
}

static Token make_token(Lexer* lexer, TokenType type) {
    return token_make(
        type,
        lexer->start,
        (int)(lexer->current - lexer->start),
        lexer->line,
        lexer->start_column
    );
}

static Token error_token(Lexer* lexer, const char* message) {
    return token_error(message, lexer->line, lexer->start_column);
}

static void skip_whitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                lexer->column = 0;  // Will be 1 after advance
                advance(lexer);
                break;
            case '/':
                if (peek_next(lexer) == '/') {
                    // Single-line comment
                    while (peek(lexer) != '\n' && !is_at_end(lexer)) {
                        advance(lexer);
                    }
                } else if (peek_next(lexer) == '*') {
                    // Multi-line comment
                    advance(lexer);  // consume /
                    advance(lexer);  // consume *
                    bool found_end = false;
                    while (!is_at_end(lexer)) {
                        if (peek(lexer) == '*' && peek_next(lexer) == '/') {
                            advance(lexer);  // consume *
                            advance(lexer);  // consume /
                            found_end = true;
                            break;
                        }
                        if (peek(lexer) == '\n') {
                            lexer->line++;
                            lexer->column = 0;
                        }
                        advance(lexer);
                    }
                    if (!found_end) {
                        lexer->error = "Unterminated comment";
                        return;
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static Token scan_string(Lexer* lexer) {
    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\n') {
            lexer->line++;  // LCOV_EXCL_LINE
            lexer->column = 0;  // LCOV_EXCL_LINE
        }  // LCOV_EXCL_LINE
        if (peek(lexer) == '\\' && peek_next(lexer) != '\0') {
            advance(lexer);  // Skip the backslash
        }
        advance(lexer);
    }

    if (is_at_end(lexer)) {
        return error_token(lexer, "Unterminated string");
    }

    advance(lexer);  // Closing quote
    return make_token(lexer, TOKEN_STRING);
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static Token scan_number(Lexer* lexer) {
    while (is_digit(peek(lexer))) {
        advance(lexer);
    }

    // Look for fractional part
    if (peek(lexer) == '.' && is_digit(peek_next(lexer))) {
        advance(lexer);  // Consume the dot
        while (is_digit(peek(lexer))) {
            advance(lexer);
        }
    }

    return make_token(lexer, TOKEN_NUMBER);
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static TokenType check_keyword(Lexer* lexer, int start, int length,
                                const char* rest, TokenType type) {
    if (lexer->current - lexer->start == start + length &&
        memcmp(lexer->start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type(Lexer* lexer) {
    switch (lexer->start[0]) {
        case 'a':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'n':
                        // "and" vs "any"
                        if (lexer->current - lexer->start == 3) {
                            if (lexer->start[2] == 'd') return TOKEN_AND;
                            if (lexer->start[2] == 'y') return TOKEN_TYPE_ANY;  // LCOV_EXCL_LINE
                        }
                        break;
                }
            }
            break;
        case 'b':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'r': return check_keyword(lexer, 2, 3, "eak", TOKEN_BREAK);
                    case 'o': return check_keyword(lexer, 2, 2, "ol", TOKEN_TYPE_BOOL);  // LCOV_EXCL_LINE
                }
            }
            break;
        case 'c': return check_keyword(lexer, 1, 7, "ontinue", TOKEN_CONTINUE);
        case 'e': return check_keyword(lexer, 1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'a': return check_keyword(lexer, 2, 3, "lse", TOKEN_FALSE);
                    case 'o': return check_keyword(lexer, 2, 1, "r", TOKEN_FOR);
                    case 'u':
                        // "func" vs "function"
                        if (lexer->current - lexer->start == 4) {
                            return check_keyword(lexer, 2, 2, "nc", TOKEN_TYPE_FUNC);  // LCOV_EXCL_LINE
                        }
                        return check_keyword(lexer, 2, 6, "nction", TOKEN_FUNCTION);
                }
            }
            break;
        case 'i':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'f': return lexer->current - lexer->start == 2 ? TOKEN_IF : TOKEN_IDENTIFIER;
                    case 'n':
                        // "in" vs "int"
                        if (lexer->current - lexer->start == 2) return TOKEN_IN;
                        return check_keyword(lexer, 2, 1, "t", TOKEN_TYPE_INT);  // LCOV_EXCL_LINE
                }
            }
            break;
        case 'l': return check_keyword(lexer, 1, 3, "ist", TOKEN_TYPE_LIST);  // LCOV_EXCL_LINE
        case 'n':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'o':
                        // "not" vs "none"
                        if (lexer->current - lexer->start == 3) {
                            return check_keyword(lexer, 2, 1, "t", TOKEN_NOT);
                        }
                        return check_keyword(lexer, 2, 2, "ne", TOKEN_TYPE_NONE);  // LCOV_EXCL_LINE
                    case 'u':
                        // "null" vs "num"
                        if (lexer->current - lexer->start == 3) {
                            return check_keyword(lexer, 2, 1, "m", TOKEN_TYPE_NUM);  // LCOV_EXCL_LINE
                        }
                        return check_keyword(lexer, 2, 2, "ll", TOKEN_NULL);
                }
            }
            break;
        case 'o': return check_keyword(lexer, 1, 1, "r", TOKEN_OR);
        case 'r': return check_keyword(lexer, 1, 5, "eturn", TOKEN_RETURN);
        case 's':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 't':
                        // "str" vs "struct"
                        if (lexer->current - lexer->start == 3) {
                            return check_keyword(lexer, 2, 1, "r", TOKEN_TYPE_STR);  // LCOV_EXCL_LINE
                        }
                        return check_keyword(lexer, 2, 4, "ruct", TOKEN_STRUCT);
                }
            }
            break;
        case 't':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'h': return check_keyword(lexer, 2, 2, "is", TOKEN_THIS);
                    case 'r': return check_keyword(lexer, 2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'w': return check_keyword(lexer, 1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

static Token scan_identifier(Lexer* lexer) {
    while (is_alpha(peek(lexer)) || is_digit(peek(lexer))) {
        advance(lexer);
    }
    return make_token(lexer, identifier_type(lexer));
}

Token lexer_scan_token(Lexer* lexer) {
    skip_whitespace(lexer);

    // Check for errors from skip_whitespace (e.g., unterminated comment)
    if (lexer->error != NULL) {
        const char* msg = lexer->error;
        lexer->error = NULL;
        return error_token(lexer, msg);
    }

    lexer->start = lexer->current;
    lexer->start_column = lexer->column;

    if (is_at_end(lexer)) {
        return token_eof(lexer->line, lexer->column);
    }

    char c = advance(lexer);

    if (is_alpha(c)) return scan_identifier(lexer);
    if (is_digit(c)) return scan_number(lexer);

    switch (c) {
        case '(': return make_token(lexer, TOKEN_LEFT_PAREN);
        case ')': return make_token(lexer, TOKEN_RIGHT_PAREN);
        case '{': return make_token(lexer, TOKEN_LEFT_BRACE);
        case '}': return make_token(lexer, TOKEN_RIGHT_BRACE);
        case '[': return make_token(lexer, TOKEN_LEFT_BRACKET);
        case ']': return make_token(lexer, TOKEN_RIGHT_BRACKET);
        case ',': return make_token(lexer, TOKEN_COMMA);
        case '.': return make_token(lexer, TOKEN_DOT);
        case ';': return make_token(lexer, TOKEN_SEMICOLON);
        case ':': return make_token(lexer, TOKEN_COLON);
        case '%': return make_token(lexer, TOKEN_PERCENT);

        case '+':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_PLUS_EQUAL);
            if (match(lexer, '+')) return make_token(lexer, TOKEN_PLUS_PLUS);
            return make_token(lexer, TOKEN_PLUS);
        case '-':
            if (match(lexer, '>')) return make_token(lexer, TOKEN_ARROW);
            if (match(lexer, '=')) return make_token(lexer, TOKEN_MINUS_EQUAL);
            if (match(lexer, '-')) return make_token(lexer, TOKEN_MINUS_MINUS);
            return make_token(lexer, TOKEN_MINUS);
        case '*':
            return make_token(lexer, match(lexer, '=') ? TOKEN_STAR_EQUAL : TOKEN_STAR);
        case '/':
            return make_token(lexer, match(lexer, '=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH);

        case '!':
            return make_token(lexer, match(lexer, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return make_token(lexer, match(lexer, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return make_token(lexer, match(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return make_token(lexer, match(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

        case '"': return scan_string(lexer);
    }

    return error_token(lexer, "Unexpected character");
}
