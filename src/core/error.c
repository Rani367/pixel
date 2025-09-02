#include "error.h"
#include "strings.h"
#include <stdarg.h>
#include <string.h>

static const char* error_code_names[] = {
    [ERR_NONE] = "ERR_NONE",

    [ERR_OUT_OF_MEMORY] = "ERR_OUT_OF_MEMORY",
    [ERR_IO_ERROR] = "ERR_IO_ERROR",
    [ERR_INVALID_ARGUMENT] = "ERR_INVALID_ARGUMENT",

    [ERR_UNEXPECTED_CHARACTER] = "ERR_UNEXPECTED_CHARACTER",
    [ERR_UNTERMINATED_STRING] = "ERR_UNTERMINATED_STRING",
    [ERR_UNTERMINATED_COMMENT] = "ERR_UNTERMINATED_COMMENT",
    [ERR_INVALID_NUMBER] = "ERR_INVALID_NUMBER",
    [ERR_INVALID_ESCAPE] = "ERR_INVALID_ESCAPE",

    [ERR_EXPECTED_EXPRESSION] = "ERR_EXPECTED_EXPRESSION",
    [ERR_EXPECTED_TOKEN] = "ERR_EXPECTED_TOKEN",
    [ERR_UNEXPECTED_TOKEN] = "ERR_UNEXPECTED_TOKEN",
    [ERR_TOO_MANY_PARAMETERS] = "ERR_TOO_MANY_PARAMETERS",
    [ERR_TOO_MANY_ARGUMENTS] = "ERR_TOO_MANY_ARGUMENTS",
    [ERR_INVALID_ASSIGNMENT] = "ERR_INVALID_ASSIGNMENT",

    [ERR_UNDEFINED_VARIABLE] = "ERR_UNDEFINED_VARIABLE",
    [ERR_UNDEFINED_FUNCTION] = "ERR_UNDEFINED_FUNCTION",
    [ERR_REDEFINED_VARIABLE] = "ERR_REDEFINED_VARIABLE",
    [ERR_REDEFINED_FUNCTION] = "ERR_REDEFINED_FUNCTION",
    [ERR_TYPE_MISMATCH] = "ERR_TYPE_MISMATCH",
    [ERR_ARITY_MISMATCH] = "ERR_ARITY_MISMATCH",

    [ERR_DIVISION_BY_ZERO] = "ERR_DIVISION_BY_ZERO",
    [ERR_STACK_OVERFLOW] = "ERR_STACK_OVERFLOW",
    [ERR_INDEX_OUT_OF_BOUNDS] = "ERR_INDEX_OUT_OF_BOUNDS",
    [ERR_NULL_REFERENCE] = "ERR_NULL_REFERENCE",
};

const char* error_code_name(ErrorCode code) {
    if (code >= 0 && code < ERR_COUNT) {
        return error_code_names[code];
    }
    return "ERR_UNKNOWN";
}

SourceLocation source_location_none(void) {
    SourceLocation loc;
    loc.file = NULL;
    loc.line = 0;
    loc.column = 0;
    loc.length = 0;
    return loc;
}

SourceLocation source_location_new(const char* file, int line, int column, int length) {
    SourceLocation loc;
    loc.file = file;
    loc.line = line;
    loc.column = column;
    loc.length = length;
    return loc;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
Error* error_new(ErrorCode code, SourceLocation loc, const char* fmt, ...) {
    Error* err = PH_ALLOC(sizeof(Error));
    if (err == NULL) {
        return NULL;
    }

    err->code = code;
    err->location = loc;
    err->cause = NULL;

    va_list args;
    va_start(args, fmt);

    // Determine message size
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

    if (size < 0) {
        err->message = ph_strdup("(error formatting message)");
    } else {
        err->message = PH_ALLOC((size_t)size + 1);
        vsnprintf(err->message, (size_t)size + 1, fmt, args);
    }

    va_end(args);
    return err;
}

Error* error_wrap(Error* cause, const char* fmt, ...) {
    Error* err = PH_ALLOC(sizeof(Error));
    if (err == NULL) {
        return cause;  // Can't wrap, return original
    }

    err->code = cause ? cause->code : ERR_NONE;
    err->location = cause ? cause->location : source_location_none();
    err->cause = cause;

    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

    if (size < 0) {
        err->message = ph_strdup("(error formatting message)");
    } else {
        err->message = PH_ALLOC((size_t)size + 1);
        vsnprintf(err->message, (size_t)size + 1, fmt, args);
    }

    va_end(args);
    return err;
}
#pragma GCC diagnostic pop

void error_print(Error* err, FILE* out) {
    if (err == NULL) {
        return;
    }

    // Print the error
    if (err->location.file != NULL) {
        fprintf(out, "%s:%d:%d: error: %s\n",
                err->location.file,
                err->location.line,
                err->location.column,
                err->message);
    } else {
        fprintf(out, "error: %s\n", err->message);
    }

    // Print the cause chain
    Error* cause = err->cause;
    while (cause != NULL) {
        if (cause->location.file != NULL) {
            fprintf(out, "  caused by: %s:%d:%d: %s\n",
                    cause->location.file,
                    cause->location.line,
                    cause->location.column,
                    cause->message);
        } else {
            fprintf(out, "  caused by: %s\n", cause->message);
        }
        cause = cause->cause;
    }
}

// Find the start and length of line N in source (1-indexed)
static const char* find_source_line(const char* source, int line_num, int* out_le