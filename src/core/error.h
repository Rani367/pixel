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

    /ERS,
    ERR_TOO_MANY_ARGUMENTS,
    ERR_INVALID_ASSIGNMENT,

    // Semantic errors
    ERR_UNDEFINED_VARIABLE,
    ERR_UNDEFINED_FUNCTION,
    ERR_REDEFINED_VARIABLE,
    ERR_REDEFINED_FUNCTION,
    ERR_TYPE_MISMATCH,
    ERR_ARITY_MISMATCH,

    // Runtime errors
    ERR_DIVISION_BY_ZERO,
    ERR_STACK_OVERFLOW,
    ERR_INDEX_OUT_OF_BOUNDS,
    ERR_NULL_REFERENCE,

    ERR_COUNT
} ErrorCode;

// Source location for error reporting
typedef struct {
    const char* file;
    int line;
    int column;
    int length;  // Length of the error span for underlining
} SourceLocation;

// Error structure with optional chaining
typedef struct Error {
    ErrorCode code;
    char* message;
    SourceLocation location;
    struct Error* cause;  // For error chaining
} Error;

// Create a new error
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 3, 4)))
#endif
Error* error_new(ErrorCode code, SourceLocation loc, const char* fmt, ...);

// Wrap an existing error with additional context
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
Error* error_wrap(Error* cause, const char* fmt, ...);

// Print an error to a file (usually stderr)
void error_print(Error* err, FILE* out);

// Print an error with source code context and underlines
// source: the full source code that was parsed
void error_print_pretty(Error* err, const char* source, FILE* out);

// Free an error and its chain
void error_free(Error* err);

// Get the error code name as a string
const char* error_code_name(ErrorCode code);

// Create a "no location" source location
SourceLocation source_location_none(void);

// Create a source location
SourceLocation source_location_new(const char* file, int line, int column, int length);

#endif // PH_ERROR_H
