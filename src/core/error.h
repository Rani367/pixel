/ Parser errors
    ERR_EXPECTED_EXPRESSION,
    ERR_EXPECTED_TOKEN,
    ERR_UNEXPECTED_TOKEN,
    ERR_TOO_MANY_PARAMET#ifndef PH_ERROR_H
#define PH_ERROR_H

#include "common.h"

// Error codes
typedef enum {
    ERR_NONE = 0,

    // General errors
    ERR_OUT_OF_MEMORY,
    ERR_IO_ERROR,
    ERR_INVALID_ARGUMENT,

    // Lexer errors
    ERR_UNEXPECTED_CHARACTER,
    ERR_UNTERMINATED_STRING,
    ERR_UNTERMINATED_COMMENT,
    ERR_INVALID_NUMBER,
    ERR_INVALID_ESCAPE,

    /