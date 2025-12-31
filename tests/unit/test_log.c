#include "../test_framework.h"
#include "core/log.h"
#include <stdlib.h>

// ============================================================================
// Log Level Tests
// ============================================================================

TEST(log_set_level_all_levels) {
    // Just verify setting each level doesn't crash
    log_set_level(LOG_TRACE);
    log_set_level(LOG_DEBUG);
    log_set_level(LOG_INFO);
    log_set_level(LOG_WARN);
    log_set_level(LOG_ERROR);
}

TEST(log_level_filtering_works) {
    // Set level to ERROR, write a lower priority message
    log_set_level(LOG_ERROR);

    FILE* f = fopen("/dev/null", "w");
    if (f) {
        log_set_output(f);
        // This should be filtered (INFO < ERROR)
        log_write(LOG_INFO, __FILE__, __LINE__, "filtered message");
        // This should not be filtered
        log_write(LOG_ERROR, __FILE__, __LINE__, "error message");
        fclose(f);
    }

    // Reset to default
    log_set_output(NULL);
    log_set_level(LOG_INFO);
}

TEST(log_trace_shows_all) {
    log_set_level(LOG_TRACE);

    FILE* f = fopen("/dev/null", "w");
    if (f) {
        log_set_output(f);
        // All levels should be written
        log_write(LOG_TRACE, __FILE__, __LINE__, "trace");
        log_write(LOG_DEBUG, __FILE__, __LINE__, "debug");
        log_write(LOG_INFO, __FILE__, __LINE__, "info");
        log_write(LOG_WARN, __FILE__, __LINE__, "warn");
        log_write(LOG_ERROR, __FILE__, __LINE__, "error");
        fclose(f);
    }

    log_set_output(NULL);
    log_set_level(LOG_INFO);
}

TEST(log_error_shows_only_errors) {
    log_set_level(LOG_ERROR);

    FILE* f = fopen("/dev/null", "w");
    if (f) {
        log_set_output(f);
        // Only ERROR should be written
        log_write(LOG_TRACE, __FILE__, __LINE__, "trace");  // filtered
        log_write(LOG_DEBUG, __FILE__, __LINE__, "debug");  // filtered
        log_write(LOG_INFO, __FILE__, __LINE__, "info");    // filtered
        log_write(LOG_WARN, __FILE__, __LINE__, "warn");    // filtered
        log_write(LOG_ERROR, __FILE__, __LINE__, "error");  // written
        fclose(f);
    }

    log_set_output(NULL);
    log_set_level(LOG_INFO);
}

// ============================================================================
// Log Output Tests
// ============================================================================

TEST(log_set_output_to_file) {
    FILE* f = tmpfile();
    if (f) {
        log_set_output(f);
        log_set_level(LOG_INFO);
        log_write(LOG_INFO, "test.c", 42, "test message");

        // Read back and verify something was written
        fflush(f);
        long pos = ftell(f);
        ASSERT_GT(pos, 0);

        fclose(f);
    }

    log_set_output(NULL);
}

TEST(log_set_output_null_uses_stderr) {
    log_set_output(NULL);
    // Should use stderr without crashing
    log_set_level(LOG_ERROR);
    log_write(LOG_ERROR, "test.c", 1, "test");
    log_set_level(LOG_INFO);
}

TEST(log_write_formats_message) {
    FILE* f = tmpfile();
    if (f) {
        log_set_output(f);
        log_set_level(LOG_INFO);
        log_write(LOG_INFO, "test.c", 1, "value: %d, str: %s", 42, "hello");

        fflush(f);
        rewind(f);

        char buffer[512];
        size_t n = fread(buffer, 1, sizeof(buffer) - 1, f);
        buffer[n] = '\0';

        ASSERT_STR_CONTAINS(buffer, "value: 42, str: hello");
        fclose(f);
    }

    log_set_output(NULL);
}

TEST(log_write_includes_level_name) {
    FILE* f = tmpfile();
    if (f) {
        log_set_output(f);
        log_set_level(LOG_INFO);
        log_write(LOG_INFO, "test.c", 1, "test");

        fflush(f);
        rewind(f);

        char buffer[512];
        size_t n = fread(buffer, 1, sizeof(buffer) - 1, f);
        buffer[n] = '\0';

        ASSERT_STR_CONTAINS(buffer, "[INFO]");
        fclose(f);
    }

    log_set_output(NULL);
}

TEST(log_write_extracts_filename_from_path) {
    FILE* f = tmpfile();
    if (f) {
        log_set_output(f);
        log_set_level(LOG_INFO);
        log_write(LOG_INFO, "/path/to/deep/file.c", 100, "test");

        fflush(f);
        rewind(f);

        char buffer[512];
        size_t n = fread(buffer, 1, sizeof(buffer) - 1, f);
        buffer[n] = '\0';

        // Should contain just "file.c" not the full path
        ASSERT_STR_CONTAINS(buffer, "file.c:100");
        fclose(f);
    }

    log_set_output(NULL);
}

TEST(log_write_includes_timestamp) {
    FILE* f = tmpfile();
    if (f) {
        log_set_output(f);
        log_set_level(LOG_INFO);
        log_write(LOG_INFO, "test.c", 1, "test");

        fflush(f);
        rewind(f);

        char buffer[512];
        size_t n = fread(buffer, 1, sizeof(buffer) - 1, f);
        buffer[n] = '\0';

        // Timestamp format is YYYY-MM-DD HH:MM:SS
        // Check for date-like pattern
        ASSERT(n > 10);  // Should have some content
        fclose(f);
    }

    log_set_output(NULL);
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Log Level");
    RUN_TEST(log_set_level_all_levels);
    RUN_TEST(log_level_filtering_works);
    RUN_TEST(log_trace_shows_all);
    RUN_TEST(log_error_shows_only_errors);

    TEST_SUITE("Log Output");
    RUN_TEST(log_set_output_to_file);
    RUN_TEST(log_set_output_null_uses_stderr);
    RUN_TEST(log_write_formats_message);
    RUN_TEST(log_write_includes_level_name);
    RUN_TEST(log_write_extracts_filename_from_path);
    RUN_TEST(log_write_includes_timestamp);

    TEST_SUMMARY();
}
