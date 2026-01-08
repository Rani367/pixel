#include "token.h"
#include <stdio.h>
#include <string.h>

static const char* token_names[] = {
    [TOKEN_LEFT_PAREN] = "LEFT_PAREN",
    [TOKEN_RIGHT_PAREN] = "RIGHT_PAREN",
    [TOKEN_LEFT_BRACE] = "LEFT_BRACE",
    [TOKEN_RIGHT_BRACE] = "RIGHT_BRACE",
    [TOKEN_LEFT_BRACKET] = "LEFT_BRACKET",
    [TOKEN_RIGHT_BRACKET] = "RIGHT_BRACKET",
    [TOKEN_COMMA] = "COMMA",
    [TOKEN_DOT] = "DOT",
    [TOKEN_SEMICOLON] = "SEMICOLON",
    [TOKEN_PLUS] = "PLUS",
    [TOKEN_MINUS] = "MINUS",
    [TOKEN_STAR] = "STAR",
    [TOKEN_SLASH] = "SLASH",
    [TOKEN_PERCENT] = "PERCENT",
    [TOKEN_COLON] = "COLON",

    [TOKEN_BANG] = "BANG",
    [TOKEN_BANG_EQUAL] = "BANG_EQUAL",
    [TOKEN_EQUAL] = "EQUAL",
    [TOKEN_EQUAL_EQUAL] = "EQUAL_EQUAL",
    [TOKEN_GREATER] = "GREATER",
    [TOKEN_GREATER_EQUAL] = "GREATER_EQUAL",
    [TOKEN_LESS] = "LESS",
    [TOKEN_LESS_EQUAL] = "LESS_EQUAL",
    [TOKEN_ARROW] = "ARROW",
    [TOKEN_PLUS_EQUAL] = "PLUS_EQUAL",
    [TOKEN_MINUS_EQUAL] = "MINUS_EQUAL",
    [TOKEN_STAR_EQUAL] = "STAR_EQUAL",
    [TOKEN_SLASH_EQUAL] = "SLASH_EQUAL",
    [TOKEN_PLUS_PLUS] = "PLUS_PLUS",
    [TOKEN_MINUS_MINUS] = "MINUS_MINUS",

    [TOKEN_IDENTIFIER] = "IDENTIFIER",
    [TOKEN_STRING] = "STRING",
    [TOKEN_NUMBER] = "NUMBER",

    [TOKEN_AND] = "AND",
    [TOKEN_ELSE] = "ELSE",
    [TOKEN_FALSE] = "FALSE",
    [TOKEN_FOR] = "FOR",
    [TOKEN_FUNCTION] = "FUNCTION",
    [TOKEN_IF] = "IF",
    [TOKEN_IN] = "IN",
    [TOKEN_NOT] = "NOT",
    [TOKEN_NULL] = "NULL",
    [TOKEN_OR] = "OR",
    [TOKEN_RETURN] = "RETURN",
    [TOKEN_STRUCT] = "STRUCT",
    [TOKEN_THIS] = "THIS",
    [TOKEN_TRUE] = "TRUE",
    [TOKEN_WHILE] = "WHILE",
    [TOKEN_BREAK] = "BREAK",
    [TOKEN_CONTINUE] = "CONTINUE",

    [TOKEN_TYPE_NUM] = "TYPE_NUM",
    [TOKEN_TYPE_INT] = "TYPE_INT",
    [TOKEN_TYPE_STR] = "TYPE_STR",
    [TOKEN_TYPE_BOOL] = "TYPE_BOOL",
    [TOKEN_TYPE_NONE] = "TYPE_NONE",
    [TOKEN_TYPE_LIST] = "TYPE_LIST",
    [TOKEN_TYPE_FUNC] = "TYPE_FUNC",
    [TOKEN_TYPE_ANY] = "TYPE_ANY",

    [TOKEN_ERROR] = "ERROR",
    [TOKEN_EOF] = "EOF",
};

const char* token_type_name(TokenType type) {
    if (type >= 0 && type < TOKEN_COUNT) {
        return token_names[type];
    }
    return "UNKNOWN";
}

void token_print(Token token) {
    printf("%3d:%-3d %-15s '",
           token.line, token.column, token_type_name(token.type));

    for (int i = 0; i < token.length; i++) {
        char c = token.start[i];
        if (c == '\n') {
            printf("\\n");
        } else if (c == '\t') {
            printf("\\t");
        } else {
            putchar(c);
        }
    }
    printf("'\n");
}

Token token_make(TokenType type, const char* start, int length, int line, int column) {
    Token token;
    token.type = type;
    token.start = start;
    token.length = length;
    token.line = line;
    token.column = column;
    return token;
}

Token token_error(const char* message, int line, int column) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = line;
    token.column = column;
    return token;
}

Token token_eof(int line, int column) {
    Token token;
    token.type = TOKEN_EOF;
    token.start = "";
    token.length = 0;
    token.line = line;
    token.column = column;
    return token;
}
