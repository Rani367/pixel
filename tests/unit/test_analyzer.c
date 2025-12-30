#include "../test_framework.h"
#include "compiler/parser.h"
#include "compiler/analyzer.h"
#include "core/arena.h"

// Helper to parse and analyze source code
static bool analyze_source(const char* source, Analyzer* analyzer) {
    Arena* arena = arena_new(0);
    Parser parser;
    parser_init(&parser, source, arena);

    int count;
    Stmt** stmts = parser_parse(&parser, &count);

    if (parser_had_error(&parser)) {
        arena_free(arena);
        return false;
    }

    analyzer_init(analyzer, "test.pixel", source);
    bool result = analyzer_analyze(analyzer, stmts, count);

    arena_free(arena);
    return result;
}

// Helper to check if a specific error message is present
static bool has_error_containing(Analyzer* analyzer, const char* substring) {
    for (int i = 0; i < analyzer->error_count; i++) {
        if (strstr(analyzer->errors[i]->message, substring) != NULL) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// Variable Resolution Tests
// ============================================================================

TEST(analyze_valid_assignment) {
    Analyzer analyzer;
    bool result = analyze_source("x = 42", &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

TEST(analyze_undefined_variable) {
    Analyzer analyzer;
    bool result = analyze_source("y = x + 1", &analyzer);
    ASSERT(!result);
    ASSERT_EQ(analyzer.error_count, 1);
    ASSERT(has_error_containing(&analyzer, "Undefined variable 'x'"));
    analyzer_free(&analyzer);
}

TEST(analyze_variable_in_expression) {
    Analyzer analyzer;
    bool result = analyze_source("x = 1\ny = x + 2", &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

TEST(analyze_undefined_in_call) {
    Analyzer analyzer;
    bool result = analyze_source("print(undefined_var)", &analyzer);
    ASSERT(!result);
    ASSERT(has_error_containing(&analyzer, "Undefined variable 'undefined_var'"));
    analyzer_free(&analyzer);
}

// ============================================================================
// Scope Tests
// ============================================================================

TEST(analyze_scope_shadowing) {
    Analyzer analyzer;
    const char* source =
        "x = 1\n"
        "if true {\n"
        "    x = 2\n"  // This shadows the outer x
        "    y = x\n"  // Uses the inner x
        "}\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

TEST(analyze_nested_scopes) {
    Analyzer analyzer;
    const char* source =
        "a = 1\n"
        "if true {\n"
        "    b = 2\n"
        "    if true {\n"
        "        c = a + b\n"  // Both accessible
        "    }\n"
        "}\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

TEST(analyze_function_scope) {
    Analyzer analyzer;
    const char* source =
        "function foo(x) {\n"
        "    y = x + 1\n"
        "    return y\n"
        "}\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

TEST(analyze_function_params_accessible) {
    Analyzer analyzer;
    const char* source =
        "function add(a, b) {\n"
        "    return a + b\n"
        "}\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

// ============================================================================
// Control Flow Validation Tests
// ============================================================================

TEST(analyze_break_in_loop) {
    Analyzer analyzer;
    const char* source =
        "while true {\n"
        "    break\n"
        "}\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

TEST(analyze_break_outside_loop) {
    Analyzer analyzer;
    bool result = analyze_source("break", &analyzer);
    ASSERT(!result);
    ASSERT_EQ(analyzer.error_count, 1);
    ASSERT(has_error_containing(&analyzer, "'break' outside of loop"));
    analyzer_free(&analyzer);
}

TEST(analyze_continue_in_loop) {
    Analyzer analyzer;
    const char* source =
        "while true {\n"
        "    continue\n"
        "}\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

TEST(analyze_continue_outside_loop) {
    Analyzer analyzer;
    bool result = analyze_source("continue", &analyzer);
    ASSERT(!result);
    ASSERT_EQ(analyzer.error_count, 1);
    ASSERT(has_error_containing(&analyzer, "'continue' outside of loop"));
    analyzer_free(&analyzer);
}

TEST(analyze_return_in_function) {
    Analyzer analyzer;
    const char* source =
        "function foo() {\n"
        "    return 42\n"
        "}\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

TEST(analyze_return_outside_function) {
    Analyzer analyzer;
    bool result = analyze_source("return 42", &analyzer);
    ASSERT(!result);
    ASSERT_EQ(analyzer.error_count, 1);
    ASSERT(has_error_containing(&analyzer, "'return' outside of function"));
    analyzer_free(&analyzer);
}

TEST(analyze_break_in_nested_loop) {
    Analyzer analyzer;
    const char* source =
        "items = [1, 2, 3]\n"
        "while true {\n"
        "    for i in items {\n"
        "        break\n"
        "    }\n"
        "}\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

TEST(analyze_break_in_function_in_loop) {
    Analyzer analyzer;
    // break inside a function defined inside a loop should fail
    const char* source =
        "while true {\n"
        "    function inner() {\n"
        "        break\n"  // This break is NOT in a loop context
        "    }\n"
        "}\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(!result);
    ASSERT(has_error_containing(&analyzer, "'break' outside of loop"));
    analyzer_free(&analyzer);
}

// ============================================================================
// For Loop Tests
// ============================================================================

TEST(analyze_for_loop_variable) {
    Analyzer analyzer;
    const char* source =
        "items = [1, 2, 3]\n"
        "for i in items {\n"
        "    x = i * 2\n"  // i should be accessible
        "}\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

// ============================================================================
// Struct Tests
// ============================================================================

TEST(analyze_struct_declaration) {
    Analyzer analyzer;
    const char* source =
        "struct Point { x, y }\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

TEST(analyze_struct_duplicate_field) {
    Analyzer analyzer;
    const char* source =
        "struct Point { x, x }\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(!result);
    ASSERT(has_error_containing(&analyzer, "Duplicate field 'x'"));
    analyzer_free(&analyzer);
}

// ============================================================================
// Did You Mean Tests
// ============================================================================

TEST(analyze_did_you_mean) {
    Analyzer analyzer;
    const char* source =
        "count = 10\n"
        "x = coutn\n";  // Typo: coutn instead of count
    bool result = analyze_source(source, &analyzer);
    ASSERT(!result);
    ASSERT(has_error_containing(&analyzer, "Did you mean"));
    analyzer_free(&analyzer);
}

// ============================================================================
// Redeclaration Tests
// ============================================================================

TEST(analyze_redeclaration_in_scope) {
    Analyzer analyzer;
    const char* source =
        "function foo(x) {\n"
        "    x = 10\n"  // This is just assignment, not redeclaration
        "}\n";
    bool result = analyze_source(source, &analyzer);
    // This should be allowed - x is already declared as parameter
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

// ============================================================================
// Anonymous Function Tests
// ============================================================================

TEST(analyze_anonymous_function) {
    Analyzer analyzer;
    const char* source =
        "add = function(a, b) { return a + b }\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

TEST(analyze_nested_functions) {
    Analyzer analyzer;
    const char* source =
        "function outer() {\n"
        "    x = 1\n"
        "    function inner() {\n"
        "        return x\n"  // Access outer variable
        "    }\n"
        "    return inner\n"
        "}\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

// ============================================================================
// Complex Tests
// ============================================================================

TEST(analyze_game_loop_pattern) {
    Analyzer analyzer;
    // Test the common game pattern where variables are assigned in one function
    // and read in another. With implicit globals, this should work.
    const char* source =
        "function on_start() {\n"
        "    player_x = 100\n"
        "    player_y = 100\n"
        "}\n"
        "\n"
        "function on_update(dt) {\n"
        "    player_x = player_x + 10 * dt\n"
        "}\n"
        "\n"
        "function on_draw() {\n"
        "    result = player_x + player_y\n"
        "}\n";
    bool result = analyze_source(source, &analyzer);
    // player_x and player_y are implicitly global when first assigned
    // so they're accessible from all functions
    ASSERT(result);
    ASSERT_EQ(analyzer.error_count, 0);
    analyzer_free(&analyzer);
}

TEST(analyze_multiple_errors) {
    Analyzer analyzer;
    const char* source =
        "break\n"
        "continue\n"
        "return 42\n";
    bool result = analyze_source(source, &analyzer);
    ASSERT(!result);
    ASSERT_EQ(analyzer.error_count, 3);
    analyzer_free(&analyzer);
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Analyzer - Variable Resolution");
    RUN_TEST(analyze_valid_assignment);
    RUN_TEST(analyze_undefined_variable);
    RUN_TEST(analyze_variable_in_expression);
    RUN_TEST(analyze_undefined_in_call);

    TEST_SUITE("Analyzer - Scope");
    RUN_TEST(analyze_scope_shadowing);
    RUN_TEST(analyze_nested_scopes);
    RUN_TEST(analyze_function_scope);
    RUN_TEST(analyze_function_params_accessible);

    TEST_SUITE("Analyzer - Control Flow");
    RUN_TEST(analyze_break_in_loop);
    RUN_TEST(analyze_break_outside_loop);
    RUN_TEST(analyze_continue_in_loop);
    RUN_TEST(analyze_continue_outside_loop);
    RUN_TEST(analyze_return_in_function);
    RUN_TEST(analyze_return_outside_function);
    RUN_TEST(analyze_break_in_nested_loop);
    RUN_TEST(analyze_break_in_function_in_loop);

    TEST_SUITE("Analyzer - For Loop");
    RUN_TEST(analyze_for_loop_variable);

    TEST_SUITE("Analyzer - Structs");
    RUN_TEST(analyze_struct_declaration);
    RUN_TEST(analyze_struct_duplicate_field);

    TEST_SUITE("Analyzer - Suggestions");
    RUN_TEST(analyze_did_you_mean);

    TEST_SUITE("Analyzer - Redeclaration");
    RUN_TEST(analyze_redeclaration_in_scope);

    TEST_SUITE("Analyzer - Anonymous Functions");
    RUN_TEST(analyze_anonymous_function);
    RUN_TEST(analyze_nested_functions);

    TEST_SUITE("Analyzer - Complex");
    RUN_TEST(analyze_game_loop_pattern);
    RUN_TEST(analyze_multiple_errors);

    TEST_SUMMARY();
}
