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
    T