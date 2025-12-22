#ifndef PH_TOKEN_H
#define PH_TOKEN_H

#include "core/common.h"

typedef enum {
    // Single-character tokens
    TOKEN_LEFT_PAREN,      // (
    TOKEN_RIGHT_PAREN,     // )
    TOKEN_LEFT_BRACE,      // {
    TOKEN_RIGHT_BRACE,     // }
    TOKEN_LEFT_BRACKET,    // [
    TOKEN_RIGHT_BRACKET,   // ]
    TOKEN_COMMA,           // ,
    TOKEN_DOT,             // .
    TOKEN_SEMICOLON,       // ;
    TOKEN_PLUS,            // +
    TOKEN_MINUS,           // -
    TOKEN_STAR,            // *
    TOKEN_SLASH,           // /
    TOKEN_PERCENT,         // %
    TOKEN_COLON,           // :

    // One or two character tokens
    TOKEN_BANG,            // !
    TOKEN_BANG_EQUAL,      // !=
    TOKEN_EQUAL,           // =
    TTOKEN_PLUS_EQUAL,      // +=
    TOKEN_MINUS_EQUAL,     // -=
    TOKEN_STAR_EQUAL,      // *=
    TOKEN_SLASH_EQUAL,     // /=
    TOKEN_PLUS_PLUS,       // ++
    TOKEN_MINUS_MINUS,     // --

    // Literals
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,

    // Keywords
    TOKEN_AND,
    TOKEN_ELSE,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FUNCTION,
    TOKEN_IF,
    TOKEN_IN,
    TOKEN_NOT,
    TOKEN_NULL,
    TOKEN_OR,
    TOKEN_RETURN,
    TOKEN_STRUCT,
    TOKEN_THIS,
    TOKEN_TRUE,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_CONTINUE,

    // Special
    TOKEN_ERROR,           // Lexer error (message in lexeme)
    TOKEN_EOF,

    TOKEN_COUNT
} TokenType;

typedef struct {
    TokenType type;
    const char* start;     // Pointer into source
    int length;
    int line;
    int column;
} Token;

// Get the name of a token type for debugging
const char* token_type_name(TokenType type);

// Print a token for debugging
void token_print(Token token);

// Create a token
Token token_make(TokenType type, const char* start, int length, int line, int column);

// Create an error token
Token token_error(const char* message, int line, int column);

// Create an EOF token
Token token_eof(int line, int column);

#endif // PH_TOKEN_H
