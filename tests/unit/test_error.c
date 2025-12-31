#include "../test_framework.h"
#include "core/error.h"
#include <stdlib.h>

// ============================================================================
// Source Location Tests
// ============================================================================

TEST(source_location_none_returns_zeroed) {
    SourceLocation loc = source_location_none();
    ASSERT_NULL(loc.file);
    ASSERT_EQ(loc.line, 0);
    ASSERT_EQ(loc.column, 0);
    ASSERT_EQ(loc.length, 0);
}

TEST(source_location_new_sets_all_fields) {
    SourceLocation loc = source_location_new("test.pixel", 10, 5, 3);
    ASSERT_STR_EQ(loc.file, "test.pixel");
    ASSERT_EQ(loc.line, 10);
    ASSERT_EQ(loc.column, 5);
    ASSERT_EQ(loc.length, 3);
}

// ============================================================================
// Error Creation Tests
// ============================================================================

TEST(error_new_basic_message) {
    SourceLocation loc = source_location_none();
    Error* err = error_new(ERR_INVALID_ARGUMENT, loc, "test error");

    ASSERT_NOT_NULL(err);
    ASSERT_EQ(err->code, ERR_INVALID_ARGUMENT);
    ASSERT_STR_EQ(err->message, "test error");
    ASSERT_NULL(err->cause);

    error_free(err);
}

TEST(error_new_formatted_message) {
    SourceLocation loc = source_location_none();
    Error* err = error_new(ERR_TYPE_MISMATCH, loc, "expected %s, got %s", "int", "string");

    ASSERT_NOT_NULL(err);
    ASSERT_STR_EQ(err->message, "expected int, got string");

    error_free(err);
}

TEST(error_new_with_location) {
    SourceLocation loc = source_location_new("main.pixel", 42, 10, 5);
    Error* err = error_new(ERR_UNDEFINED_VARIABLE, loc, "undefined variable 'x'");

    ASSERT_NOT_NULL(err);
    ASSERT_STR_EQ(err->location.file, "main.pixel");
    ASSERT_EQ(err->location.line, 42);
    ASSERT_EQ(err->location.column, 10);
    ASSERT_EQ(err->location.length, 5);

    error_free(err);
}

TEST(error_new_all_error_codes) {
    SourceLocation loc = source_location_none();

    // Test a sample of error codes
    ErrorCode codes[] = {
        ERR_NONE, ERR_OUT_OF_MEMORY, ERR_IO_ERROR,
        ERR_UNEXPECTED_CHARACTER, ERR_UNTERMINATED_STRING,
        ERR_EXPECTED_EXPRESSION, ERR_UNDEFINED_VARIABLE,
        ERR_DIVISION_BY_ZERO, ERR_STACK_OVERFLOW
    };

    for (size_t i = 0; i < sizeof(codes) / sizeof(codes[0]); i++) {
        Error* err = error_new(codes[i], loc, "test");
        ASSERT_NOT_NULL(err);
        ASSERT_EQ(err->code, codes[i]);
        error_free(err);
    }
}

// ============================================================================
// Error Wrapping Tests
// ============================================================================

TEST(error_wrap_adds_context) {
    SourceLocation loc = source_location_new("inner.pixel", 5, 1, 10);
    Error* inner = error_new(ERR_IO_ERROR, loc, "file not found");
    Error* outer = error_wrap(inner, "failed to load module");

    ASSERT_NOT_NULL(outer);
    ASSERT_STR_EQ(outer->message, "failed to load module");
    ASSERT_EQ(outer->cause, inner);
    ASSERT_EQ(outer->code, ERR_IO_ERROR);  // Inherits code from cause

    error_free(outer);  // Also frees inner
}

TEST(error_wrap_preserves_cause_chain) {
    SourceLocation loc = source_location_none();
    Error* err1 = error_new(ERR_IO_ERROR, loc, "read error");
    Error* err2 = error_wrap(err1, "parse error");
    Error* err3 = error_wrap(err2, "compilation failed");

    ASSERT_NOT_NULL(err3);
    ASSERT_EQ(err3->cause, err2);
    ASSERT_EQ(err3->cause->cause, err1);
    ASSERT_NULL(err1->cause);

    error_free(err3);  // Frees entire chain
}

TEST(error_wrap_null_cause) {
    Error* wrapped = error_wrap(NULL, "wrapping null");

    ASSERT_NOT_NULL(wrapped);
    ASSERT_EQ(wrapped->code, ERR_NONE);
    ASSERT_NULL(wrapped->cause);

    error_free(wrapped);
}

// ============================================================================
// Error Printing Tests
// ============================================================================

TEST(error_print_simple) {
    SourceLocation loc = source_location_none();
    Error* err = error_new(ERR_INVALID_ARGUMENT, loc, "bad argument");

    // Print to /dev/null just to verify no crash
    FILE* f = fopen("/dev/null", "w");
    if (f) {
        error_print(err, f);
        fclose(f);
    }

    error_free(err);
}

TEST(error_print_with_location) {
    SourceLocation loc = source_location_new("test.pixel", 10, 5, 3);
    Error* err = error_new(ERR_UNEXPECTED_TOKEN, loc, "unexpected token");

    FILE* f = fopen("/dev/null", "w");
    if (f) {
        error_print(err, f);
        fclose(f);
    }

    error_free(err);
}

TEST(error_print_null_safe) {
    FILE* f = fopen("/dev/null", "w");
    if (f) {
        error_print(NULL, f);  // Should not crash
        fclose(f);
    }
}

TEST(error_print_chain) {
    SourceLocation loc = source_location_none();
    Error* inner = error_new(ERR_IO_ERROR, loc, "inner error");
    Error* outer = error_wrap(inner, "outer error");

    FILE* f = fopen("/dev/null", "w");
    if (f) {
        error_print(outer, f);  // Should print both errors
        fclose(f);
    }

    error_free(outer);
}

TEST(error_print_pretty_with_source) {
    SourceLocation loc = source_location_new("test.pixel", 1, 5, 3);
    Error* err = error_new(ERR_UNDEFINED_VARIABLE, loc, "undefined 'foo'");

    const char* source = "let foo = bar";

    FILE* f = fopen("/dev/null", "w");
    if (f) {
        error_print_pretty(err, source, f);
        fclose(f);
    }

    error_free(err);
}

TEST(error_print_pretty_handles_tabs) {
    SourceLocation loc = source_location_new("test.pixel", 1, 5, 3);
    Error* err = error_new(ERR_UNDEFINED_VARIABLE, loc, "undefined");

    const char* source = "\t\tlet x = 1";  // Source with tabs

    FILE* f = fopen("/dev/null", "w");
    if (f) {
        error_print_pretty(err, source, f);
        fclose(f);
    }

    error_free(err);
}

TEST(error_print_pretty_null_source) {
    SourceLocation loc = source_location_new("test.pixel", 1, 1, 1);
    Error* err = error_new(ERR_INVALID_ARGUMENT, loc, "test");

    FILE* f = fopen("/dev/null", "w");
    if (f) {
        error_print_pretty(err, NULL, f);  // Falls back to simple print
        fclose(f);
    }

    error_free(err);
}

TEST(error_print_pretty_multiline) {
    SourceLocation loc = source_location_new("test.pixel", 2, 5, 3);
    Error* err = error_new(ERR_UNDEFINED_VARIABLE, loc, "undefined 'foo'");

    const char* source = "line 1\nlet foo = bar\nline 3";

    FILE* f = fopen("/dev/null", "w");
    if (f) {
        error_print_pretty(err, source, f);
        fclose(f);
    }

    error_free(err);
}

// ============================================================================
// Error Cleanup Tests
// ============================================================================

TEST(error_free_single) {
    SourceLocation loc = source_location_none();
    Error* err = error_new(ERR_NONE, loc, "test");
    error_free(err);  // Should not crash
}

TEST(error_free_chain) {
    SourceLocation loc = source_location_none();
    Error* err1 = error_new(ERR_IO_ERROR, loc, "error 1");
    Error* err2 = error_wrap(err1, "error 2");
    Error* err3 = error_wrap(err2, "error 3");
    error_free(err3);  // Should free entire chain
}

TEST(error_free_null_safe) {
    error_free(NULL);  // Should not crash
}

// ============================================================================
// Error Code Name Tests
// ============================================================================

TEST(error_code_name_all_valid) {
    // Test some specific error code names
    ASSERT_STR_EQ(error_code_name(ERR_NONE), "ERR_NONE");
    ASSERT_STR_EQ(error_code_name(ERR_OUT_OF_MEMORY), "ERR_OUT_OF_MEMORY");
    ASSERT_STR_EQ(error_code_name(ERR_IO_ERROR), "ERR_IO_ERROR");
    ASSERT_STR_EQ(error_code_name(ERR_UNEXPECTED_CHARACTER), "ERR_UNEXPECTED_CHARACTER");
    ASSERT_STR_EQ(error_code_name(ERR_UNDEFINED_VARIABLE), "ERR_UNDEFINED_VARIABLE");
    ASSERT_STR_EQ(error_code_name(ERR_DIVISION_BY_ZERO), "ERR_DIVISION_BY_ZERO");
}

TEST(error_code_name_invalid_returns_unknown) {
    ASSERT_STR_EQ(error_code_name((ErrorCode)-1), "ERR_UNKNOWN");
    ASSERT_STR_EQ(error_code_name((ErrorCode)999), "ERR_UNKNOWN");
    ASSERT_STR_EQ(error_code_name(ERR_COUNT), "ERR_UNKNOWN");
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("SourceLocation");
    RUN_TEST(source_location_none_returns_zeroed);
    RUN_TEST(source_location_new_sets_all_fields);

    TEST_SUITE("Error Creation");
    RUN_TEST(error_new_basic_message);
    RUN_TEST(error_new_formatted_message);
    RUN_TEST(error_new_with_location);
    RUN_TEST(error_new_all_error_codes);

    TEST_SUITE("Error Wrapping");
    RUN_TEST(error_wrap_adds_context);
    RUN_TEST(error_wrap_preserves_cause_chain);
    RUN_TEST(error_wrap_null_cause);

    TEST_SUITE("Error Printing");
    RUN_TEST(error_print_simple);
    RUN_TEST(error_print_with_location);
    RUN_TEST(error_print_null_safe);
    RUN_TEST(error_print_chain);
    RUN_TEST(error_print_pretty_with_source);
    RUN_TEST(error_print_pretty_handles_tabs);
    RUN_TEST(error_print_pretty_null_source);
    RUN_TEST(error_print_pretty_multiline);

    TEST_SUITE("Error Cleanup");
    RUN_TEST(error_free_single);
    RUN_TEST(error_free_chain);
    RUN_TEST(error_free_null_safe);

    TEST_SUITE("Error Code Names");
    RUN_TEST(error_code_name_all_valid);
    RUN_TEST(error_code_name_invalid_returns_unknown);

    TEST_SUMMARY();
}
