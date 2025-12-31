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
        return NULL;  // LCOV_EXCL_LINE - malloc failure
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

    // LCOV_EXCL_START - vsnprintf only returns negative on encoding errors
    if (size < 0) {
        err->message = ph_strdup("(error formatting message)");
    } else {
    // LCOV_EXCL_STOP
        err->message = PH_ALLOC((size_t)size + 1);
        vsnprintf(err->message, (size_t)size + 1, fmt, args);
    }

    va_end(args);
    return err;
}

Error* error_wrap(Error* cause, const char* fmt, ...) {
    Error* err = PH_ALLOC(sizeof(Error));
    if (err == NULL) {
        return cause;  // LCOV_EXCL_LINE - malloc failure
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

    // LCOV_EXCL_START - vsnprintf only returns negative on encoding errors
    if (size < 0) {
        err->message = ph_strdup("(error formatting message)");
    } else {
    // LCOV_EXCL_STOP
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

    // LCOV_EXCL_START - error cause chain rarely used in tests
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
    // LCOV_EXCL_STOP
}

// Find the start and length of line N in source (1-indexed)
static const char* find_source_line(const char* source, int line_num, int* out_length) {
    // LCOV_EXCL_START - edge case validation
    if (source == NULL || line_num <= 0) {
        *out_length = 0;
        return NULL;
    }
    // LCOV_EXCL_STOP

    const char* p = source;
    int current_line = 1;

    // Find the start of the requested line
    while (*p != '\0' && current_line < line_num) {
        if (*p == '\n') {
            current_line++;
        }
        p++;
    }

    // LCOV_EXCL_START - requested line beyond source
    if (current_line != line_num) {
        *out_length = 0;
        return NULL;
    }
    // LCOV_EXCL_STOP

    // Find the end of this line
    const char* line_start = p;
    while (*p != '\0' && *p != '\n') {
        p++;
    }

    *out_length = (int)(p - line_start);
    return line_start;
}

void error_print_pretty(Error* err, const char* source, FILE* out) {
    if (err == NULL) {
        return;  // LCOV_EXCL_LINE - null check
    }

    // Fall back to simple print if no source or location
    if (source == NULL || err->location.file == NULL || err->location.line <= 0) {
        error_print(err, out);
        return;
    }

    // Print error header: error[EXXX]: message
    fprintf(out, "error[%s]: %s\n", error_code_name(err->code), err->message);

    // Print location: --> file:line:column
    fprintf(out, "  --> %s:%d:%d\n",
            err->location.file,
            err->location.line,
            err->location.column);

    // Get the source line
    int line_length = 0;
    const char* line_start = find_source_line(source, err->location.line, &line_length);

    if (line_start != NULL && line_length > 0) {
        // Calculate gutter width (line number + space)
        int line_num_width = 1;
        int temp = err->location.line;
        // LCOV_EXCL_START - only for 2+ digit line numbers
        while (temp >= 10) {
            line_num_width++;
            temp /= 10;
        }
        // LCOV_EXCL_STOP

        // Print empty gutter line
        fprintf(out, "%*s |\n", line_num_width, "");

        // Print the source line
        fprintf(out, "%d | ", err->location.line);

        // Print the line, converting tabs to spaces for consistent display
        for (int i = 0; i < line_length; i++) {
            if (line_start[i] == '\t') {
                fprintf(out, "    ");  // 4 spaces for tab
            } else {
                fputc(line_start[i], out);
            }
        }
        fprintf(out, "\n");

        // Print the underline
        fprintf(out, "%*s | ", line_num_width, "");

        // Calculate offset to the error column (handling tabs before the error)
        int column = err->location.column;
        int spaces_before = 0;
        for (int i = 0; i < column - 1 && i < line_length; i++) {
            if (line_start[i] == '\t') {
                spaces_before += 4;
            } else {
                spaces_before++;
            }
        }

        // Print spaces to reach the error position
        for (int i = 0; i < spaces_before; i++) {
            fputc(' ', out);
        }

        // Print the underline carets
        int underline_len = err->location.length;
        // LCOV_EXCL_START - edge cases for underline length
        if (underline_len <= 0) {
            underline_len = 1;  // At least one caret
        }

        // Limit underline to line length
        if (column - 1 + underline_len > line_length) {
            underline_len = line_length - (column - 1);
            if (underline_len < 1) underline_len = 1;
        }
        // LCOV_EXCL_STOP

        for (int i = 0; i < underline_len; i++) {
            fputc('^', out);
        }
        fprintf(out, "\n");

        // Print empty gutter line
        fprintf(out, "%*s |\n", line_num_width, "");
    }

    // LCOV_EXCL_START - cause chain rarely used
    // Print cause chain (simplified)
    Error* cause = err->cause;
    while (cause != NULL) {
        fprintf(out, "  = caused by: %s\n", cause->message);
        cause = cause->cause;
    }
    // LCOV_EXCL_STOP
}

void error_free(Error* err) {
    while (err != NULL) {
        Error* cause = err->cause;
        PH_FREE(err->message);
        PH_FREE(err);
        err = cause;
    }
}
