#include "../test_framework.h"
#include "../test_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// ============================================================================
// Helper - recreate has_pixel_extension for testing
// This mirrors the static function in main.c
// ============================================================================

static int has_pixel_extension(const char* filename) {
    size_t len = strlen(filename);
    return len > 6 && strcmp(filename + len - 6, ".pixel") == 0;
}

// ============================================================================
// Helper - run pixel binary and capture exit code
// ============================================================================

static int run_pixel_command(const char* args[], int* exit_code) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Redirect stdout/stderr to /dev/null
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);
        execvp(args[0], (char* const*)args);
        exit(127);  // exec failed
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            *exit_code = WEXITSTATUS(status);
            return 1;  // success
        }
    }
    return 0;  // fork failed
}

// ============================================================================
// Extension Check Tests
// ============================================================================

TEST(has_pixel_extension_true) {
    ASSERT(has_pixel_extension("game.pixel"));
    ASSERT(has_pixel_extension("path/to/game.pixel"));
    ASSERT(has_pixel_extension("/absolute/path/script.pixel"));
}

TEST(has_pixel_extension_false) {
    ASSERT_FALSE(has_pixel_extension("game.txt"));
    ASSERT_FALSE(has_pixel_extension("game.py"));
    ASSERT_FALSE(has_pixel_extension("game.pixe"));  // typo
    ASSERT_FALSE(has_pixel_extension("gamepixel"));  // no dot
    ASSERT_FALSE(has_pixel_extension(".pixel.backup"));
}

TEST(has_pixel_extension_short_name) {
    // Names too short to have ".pixel" extension
    ASSERT_FALSE(has_pixel_extension("a.pxl"));
    ASSERT_FALSE(has_pixel_extension("test"));
    ASSERT_FALSE(has_pixel_extension(""));
    ASSERT_FALSE(has_pixel_extension(".pixel"));  // Exactly ".pixel" with no name
}

TEST(has_pixel_extension_exact) {
    // Exactly 6 chars before .pixel
    ASSERT(has_pixel_extension("x.pixel"));  // len = 7, has .pixel at end

    // Edge case: ".pixel" alone
    ASSERT_FALSE(has_pixel_extension(".pixel"));  // len = 6, needs > 6
}

// ============================================================================
// CLI Command Tests (requires pixel binary to exist)
// ============================================================================

TEST(cli_version_command) {
    const char* args[] = {"./build/pixel", "version", NULL};
    int exit_code = -1;

    // Only run if binary exists
    if (access("./build/pixel", X_OK) == 0) {
        if (run_pixel_command(args, &exit_code)) {
            ASSERT_EQ(exit_code, 0);
        }
    }
}

TEST(cli_help_command) {
    const char* args[] = {"./build/pixel", "help", NULL};
    int exit_code = -1;

    if (access("./build/pixel", X_OK) == 0) {
        if (run_pixel_command(args, &exit_code)) {
            ASSERT_EQ(exit_code, 0);
        }
    }
}

TEST(cli_no_args_shows_usage) {
    const char* args[] = {"./build/pixel", NULL};
    int exit_code = -1;

    if (access("./build/pixel", X_OK) == 0) {
        if (run_pixel_command(args, &exit_code)) {
            ASSERT_EQ(exit_code, 1);  // Usage shown with error exit
        }
    }
}

TEST(cli_run_missing_file) {
    const char* args[] = {"./build/pixel", "run", "/nonexistent/file.pixel", NULL};
    int exit_code = -1;

    if (access("./build/pixel", X_OK) == 0) {
        if (run_pixel_command(args, &exit_code)) {
            ASSERT_EQ(exit_code, 1);  // Should fail
        }
    }
}

TEST(cli_run_valid_file) {
    // Create a simple valid pixel file
    const char* content = "x = 1 + 2\n";
    const char* path = test_create_temp_file(content);

    const char* args[] = {"./build/pixel", "run", path, NULL};
    int exit_code = -1;

    if (access("./build/pixel", X_OK) == 0) {
        if (run_pixel_command(args, &exit_code)) {
            ASSERT_EQ(exit_code, 0);  // Should succeed
        }
    }

    test_remove_temp_file(path);
}

TEST(cli_run_syntax_error) {
    // Create a file with syntax error
    const char* content = "x = + + +\n";
    const char* path = test_create_temp_file(content);

    const char* args[] = {"./build/pixel", "run", path, NULL};
    int exit_code = -1;

    if (access("./build/pixel", X_OK) == 0) {
        if (run_pixel_command(args, &exit_code)) {
            ASSERT_EQ(exit_code, 1);  // Should fail with syntax error
        }
    }

    test_remove_temp_file(path);
}

TEST(cli_unknown_command) {
    const char* args[] = {"./build/pixel", "unknown_cmd", NULL};
    int exit_code = -1;

    if (access("./build/pixel", X_OK) == 0) {
        if (run_pixel_command(args, &exit_code)) {
            ASSERT_EQ(exit_code, 1);  // Unknown command fails
        }
    }
}

TEST(cli_direct_pixel_file) {
    // Create a simple valid pixel file
    const char* content = "x = 42\n";
    const char* path = test_create_temp_file(content);

    const char* args[] = {"./build/pixel", path, NULL};
    int exit_code = -1;

    if (access("./build/pixel", X_OK) == 0) {
        if (run_pixel_command(args, &exit_code)) {
            ASSERT_EQ(exit_code, 0);  // Should run directly
        }
    }

    test_remove_temp_file(path);
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Extension Check");
    RUN_TEST(has_pixel_extension_true);
    RUN_TEST(has_pixel_extension_false);
    RUN_TEST(has_pixel_extension_short_name);
    RUN_TEST(has_pixel_extension_exact);

    TEST_SUITE("CLI Commands");
    RUN_TEST(cli_version_command);
    RUN_TEST(cli_help_command);
    RUN_TEST(cli_no_args_shows_usage);
    RUN_TEST(cli_run_missing_file);
    RUN_TEST(cli_run_valid_file);
    RUN_TEST(cli_run_syntax_error);
    RUN_TEST(cli_unknown_command);
    RUN_TEST(cli_direct_pixel_file);

    TEST_SUMMARY();
}
