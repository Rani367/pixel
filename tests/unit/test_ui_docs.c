// Documentation Coverage Tests for UI System
// Verifies that all public UI functions have documentation comments
//
// This test parses the UI header files and ensures each function declaration
// is preceded by a documentation comment. Documentation must include:
// - Function name and brief description
// - Parameter descriptions (if any)
// - Return value description (if applicable)

#include "../test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_LINE_LEN 512
#define MAX_FUNCTIONS 100

typedef struct {
    char name[128];
    int line;
    bool has_doc;
} FunctionInfo;

static FunctionInfo functions[MAX_FUNCTIONS];
static int function_count = 0;

// Check if a line is a function declaration
static bool is_function_decl(const char* line) {
    // Skip whitespace
    while (*line && isspace(*line)) line++;

    // Skip common prefixes
    if (strncmp(line, "static ", 7) == 0) return false;  // Skip static
    if (strncmp(line, "//", 2) == 0) return false;       // Skip comments
    if (strncmp(line, "#", 1) == 0) return false;        // Skip preprocessor
    if (strncmp(line, "typedef", 7) == 0) return false;  // Skip typedefs
    if (strncmp(line, "}", 1) == 0) return false;        // Skip closing braces
    if (strncmp(line, "{", 1) == 0) return false;        // Skip opening braces
    if (strlen(line) < 5) return false;                  // Skip short lines

    // Look for function pattern: return_type name(
    const char* paren = strchr(line, '(');
    if (!paren) return false;

    // Check for semicolon (declaration, not definition)
    const char* semi = strchr(line, ';');
    if (!semi) return false;

    // Must have a return type and name before paren
    const char* space = strchr(line, ' ');
    if (!space || space > paren) return false;

    return true;
}

// Extract function name from declaration
static void extract_function_name(const char* line, char* name, size_t size) {
    const char* paren = strchr(line, '(');
    if (!paren) {
        name[0] = '\0';
        return;
    }

    // Go backwards from paren to find start of name
    const char* end = paren - 1;
    while (end > line && isspace(*end)) end--;

    const char* start = end;
    while (start > line && (isalnum(*(start - 1)) || *(start - 1) == '_')) start--;

    size_t len = end - start + 1;
    if (len >= size) len = size - 1;
    strncpy(name, start, len);
    name[len] = '\0';
}

// Parse a header file and check documentation
static int check_file_docs(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("    Cannot open %s\n", filename);
        return -1;
    }

    char line[MAX_LINE_LEN];
    char prev_lines[5][MAX_LINE_LEN] = {{0}};
    int prev_line_count = 0;
    int current_line = 0;
    int undocumented = 0;

    function_count = 0;

    while (fgets(line, sizeof(line), file)) {
        current_line++;

        // Strip trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';

        if (is_function_decl(line)) {
            char name[128];
            extract_function_name(line, name, sizeof(name));

            if (strlen(name) > 0) {
                // Check if any of the previous 5 lines is a comment
                bool has_doc = false;
                for (int i = 0; i < prev_line_count; i++) {
                    const char* p = prev_lines[i];
                    while (*p && isspace(*p)) p++;
                    if (strncmp(p, "//", 2) == 0) {
                        has_doc = true;
                        break;
                    }
                }

                if (function_count < MAX_FUNCTIONS) {
                    strncpy(functions[function_count].name, name, sizeof(functions[function_count].name) - 1);
                    functions[function_count].line = current_line;
                    functions[function_count].has_doc = has_doc;
                    function_count++;
                }

                if (!has_doc) {
                    undocumented++;
                }
            }
        }

        // Shift previous lines
        for (int i = 4; i > 0; i--) {
            strcpy(prev_lines[i], prev_lines[i - 1]);
        }
        strcpy(prev_lines[0], line);
        if (prev_line_count < 5) prev_line_count++;
    }

    fclose(file);
    return undocumented;
}

// ============================================================================
// Tests
// ============================================================================

TEST(ui_h_documentation) {
    int undocumented = check_file_docs("../src/engine/ui.h");

    if (undocumented > 0) {
        printf("\n    Undocumented functions in ui.h:\n");
        for (int i = 0; i < function_count; i++) {
            if (!functions[i].has_doc) {
                printf("      Line %d: %s\n", functions[i].line, functions[i].name);
            }
        }
    }

    ASSERT_EQ(undocumented, 0);
}

TEST(ui_natives_h_documentation) {
    int undocumented = check_file_docs("../src/engine/ui_natives.h");

    if (undocumented > 0) {
        printf("\n    Undocumented functions in ui_natives.h:\n");
        for (int i = 0; i < function_count; i++) {
            if (!functions[i].has_doc) {
                printf("      Line %d: %s\n", functions[i].line, functions[i].name);
            }
        }
    }

    ASSERT_EQ(undocumented, 0);
}

TEST(ui_menus_h_documentation) {
    int undocumented = check_file_docs("../src/engine/ui_menus.h");

    if (undocumented > 0) {
        printf("\n    Undocumented functions in ui_menus.h:\n");
        for (int i = 0; i < function_count; i++) {
            if (!functions[i].has_doc) {
                printf("      Line %d: %s\n", functions[i].line, functions[i].name);
            }
        }
    }

    ASSERT_EQ(undocumented, 0);
}

TEST(ui_h_has_functions) {
    check_file_docs("../src/engine/ui.h");

    // ui.h should have at least 15 public functions
    ASSERT_GT(function_count, 15);
}

TEST(ui_natives_h_has_functions) {
    check_file_docs("../src/engine/ui_natives.h");

    // ui_natives.h should have at least 1 function (ui_natives_init)
    ASSERT_GT(function_count, 0);
}

TEST(ui_menus_h_has_functions) {
    check_file_docs("../src/engine/ui_menus.h");

    // ui_menus.h should have at least 1 function (ui_menus_init)
    ASSERT_GT(function_count, 0);
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("UI Documentation Coverage");
    RUN_TEST(ui_h_documentation);
    RUN_TEST(ui_natives_h_documentation);
    RUN_TEST(ui_menus_h_documentation);

    TEST_SUITE("UI Function Counts");
    RUN_TEST(ui_h_has_functions);
    RUN_TEST(ui_natives_h_has_functions);
    RUN_TEST(ui_menus_h_has_functions);

    TEST_SUMMARY();
}
