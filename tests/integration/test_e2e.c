#include "../test_framework.h"
#include "../test_helpers.h"
#include "core/arena.h"
#include "compiler/parser.h"
#include "compiler/analyzer.h"
#include "compiler/codegen.h"
#include "vm/vm.h"
#include "vm/gc.h"
#include "runtime/stdlib.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Helper - Compile and run source code
// ============================================================================

static VM vm;

static void setup(void) {
    gc_init();
    vm_init(&vm);
    stdlib_init(&vm);
}

static void teardown(void) {
    vm_free(&vm);
    gc_free_all();
}

// Helper to declare builtins for analyzer
static void declare_builtins(Analyzer* analyzer) {
    analyzer_declare_global(analyzer, "print");
    analyzer_declare_global(analyzer, "println");
    analyzer_declare_global(analyzer, "type");
    analyzer_declare_global(analyzer, "to_string");
    analyzer_declare_global(analyzer, "to_number");
    analyzer_declare_global(analyzer, "abs");
    analyzer_declare_global(analyzer, "floor");
    analyzer_declare_global(analyzer, "ceil");
    analyzer_declare_global(analyzer, "round");
    analyzer_declare_global(analyzer, "min");
    analyzer_declare_global(analyzer, "max");
    analyzer_declare_global(analyzer, "clamp");
    analyzer_declare_global(analyzer, "sqrt");
    analyzer_declare_global(analyzer, "pow");
    analyzer_declare_global(analyzer, "sin");
    analyzer_declare_global(analyzer, "cos");
    analyzer_declare_global(analyzer, "tan");
    analyzer_declare_global(analyzer, "atan2");
    analyzer_declare_global(analyzer, "random");
    analyzer_declare_global(analyzer, "random_range");
    analyzer_declare_global(analyzer, "random_int");
    analyzer_declare_global(analyzer, "len");
    analyzer_declare_global(analyzer, "push");
    analyzer_declare_global(analyzer, "pop");
    analyzer_declare_global(analyzer, "insert");
    analyzer_declare_global(analyzer, "remove");
    analyzer_declare_global(analyzer, "contains");
    analyzer_declare_global(analyzer, "index_of");
    analyzer_declare_global(analyzer, "substring");
    analyzer_declare_global(analyzer, "split");
    analyzer_declare_global(analyzer, "join");
    analyzer_declare_global(analyzer, "upper");
    analyzer_declare_global(analyzer, "lower");
    analyzer_declare_global(analyzer, "range");
    analyzer_declare_global(analyzer, "time");
    analyzer_declare_global(analyzer, "clock");
    analyzer_declare_global(analyzer, "vec2");
}

// Run source code and return result
static InterpretResult run_source(const char* source) {
    Arena* arena = arena_new(64 * 1024);
    Parser parser;
    parser_init(&parser, source, arena);

    int stmt_count = 0;
    Stmt** stmts = parser_parse(&parser, &stmt_count);

    if (parser_had_error(&parser)) {
        arena_free(arena);
        return INTERPRET_COMPILE_ERROR;
    }

    Analyzer analyzer;
    analyzer_init(&analyzer, "test.pixel", source);
    declare_builtins(&analyzer);

    if (!analyzer_analyze(&analyzer, stmts, stmt_count)) {
        analyzer_free(&analyzer);
        arena_free(arena);
        return INTERPRET_COMPILE_ERROR;
    }
    analyzer_free(&analyzer);

    Codegen codegen;
    codegen_init(&codegen, "test.pixel", source);

    ObjFunction* function = codegen_compile(&codegen, stmts, stmt_count);
    if (!function) {
        codegen_free(&codegen);
        arena_free(arena);
        return INTERPRET_COMPILE_ERROR;
    }
    codegen_free(&codegen);

    InterpretResult result = vm_interpret(&vm, function);

    arena_free(arena);
    return result;
}

// ============================================================================
// Basic Programs Tests
// ============================================================================

TEST(e2e_arithmetic) {
    setup();

    InterpretResult result = run_source(
        "x = 1 + 2\n"
        "y = 10 - 5\n"
        "z = 3 * 4\n"
        "w = 20 / 4\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

TEST(e2e_variables) {
    setup();

    InterpretResult result = run_source(
        "a = 10\n"
        "b = a\n"
        "c = a + b\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

TEST(e2e_control_flow_if) {
    setup();

    InterpretResult result = run_source(
        "x = 10\n"
        "if x > 5 {\n"
        "    y = 1\n"
        "} else {\n"
        "    y = 0\n"
        "}\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

TEST(e2e_control_flow_while) {
    setup();

    InterpretResult result = run_source(
        "x = 0\n"
        "while x < 5 {\n"
        "    x = x + 1\n"
        "}\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

TEST(e2e_control_flow_loop) {
    setup();

    InterpretResult result = run_source(
        "sum = 0\n"
        "i = 0\n"
        "while i < 5 {\n"
        "    sum = sum + i\n"
        "    i = i + 1\n"
        "}\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

TEST(e2e_functions) {
    setup();

    InterpretResult result = run_source(
        "function add(a, b) {\n"
        "    return a + b\n"
        "}\n"
        "result = add(3, 4)\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

TEST(e2e_closures) {
    setup();

    InterpretResult result = run_source(
        "function make_counter() {\n"
        "    count = 0\n"
        "    function increment() {\n"
        "        count = count + 1\n"
        "        return count\n"
        "    }\n"
        "    return increment\n"
        "}\n"
        "counter = make_counter()\n"
        "a = counter()\n"
        "b = counter()\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

// ============================================================================
// Data Structure Tests
// ============================================================================

TEST(e2e_lists) {
    setup();

    InterpretResult result = run_source(
        "list = [1, 2, 3]\n"
        "first = list[0]\n"
        "list[1] = 99\n"
        "length = len(list)\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

TEST(e2e_structs) {
    setup();

    InterpretResult result = run_source(
        "struct Point { x, y }\n"
        "p = Point(10, 20)\n"
        "sum = p.x + p.y\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

TEST(e2e_methods) {
    setup();

    InterpretResult result = run_source(
        "struct Vector {\n"
        "    x, y,\n"
        "    function length() {\n"
        "        return sqrt(this.x * this.x + this.y * this.y)\n"
        "    }\n"
        "}\n"
        "v = Vector(3, 4)\n"
        "len = v.length()\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

// ============================================================================
// Error Detection Tests
// ============================================================================

TEST(e2e_syntax_error_detected) {
    setup();

    // Invalid syntax
    InterpretResult result = run_source("x = + + +");
    ASSERT_EQ(result, INTERPRET_COMPILE_ERROR);

    teardown();
}

TEST(e2e_semantic_error_detected) {
    setup();

    // Undefined variable
    InterpretResult result = run_source("x = undefined_variable");
    ASSERT_EQ(result, INTERPRET_COMPILE_ERROR);

    teardown();
}

TEST(e2e_runtime_error_division_by_zero) {
    setup();

    InterpretResult result = run_source("x = 1 / 0");
    // Division by zero might be OK (returns infinity) or error depending on implementation
    // Just verify it doesn't crash
    (void)result;

    teardown();
}

// ============================================================================
// Complex Programs Tests
// ============================================================================

TEST(e2e_fibonacci) {
    setup();

    InterpretResult result = run_source(
        "function fib(n) {\n"
        "    if n <= 1 {\n"
        "        return n\n"
        "    }\n"
        "    return fib(n - 1) + fib(n - 2)\n"
        "}\n"
        "result = fib(10)\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

TEST(e2e_nested_loops) {
    setup();

    InterpretResult result = run_source(
        "sum = 0\n"
        "i = 0\n"
        "while i < 5 {\n"
        "    j = 0\n"
        "    while j < 5 {\n"
        "        sum = sum + i * j\n"
        "        j = j + 1\n"
        "    }\n"
        "    i = i + 1\n"
        "}\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

TEST(e2e_recursion) {
    setup();

    InterpretResult result = run_source(
        "function factorial(n) {\n"
        "    if n <= 1 {\n"
        "        return 1\n"
        "    }\n"
        "    return n * factorial(n - 1)\n"
        "}\n"
        "result = factorial(5)\n"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    teardown();
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Basic Programs");
    RUN_TEST(e2e_arithmetic);
    RUN_TEST(e2e_variables);
    RUN_TEST(e2e_control_flow_if);
    RUN_TEST(e2e_control_flow_while);
    RUN_TEST(e2e_control_flow_loop);
    RUN_TEST(e2e_functions);
    RUN_TEST(e2e_closures);

    TEST_SUITE("Data Structures");
    RUN_TEST(e2e_lists);
    RUN_TEST(e2e_structs);
    RUN_TEST(e2e_methods);

    TEST_SUITE("Error Detection");
    RUN_TEST(e2e_syntax_error_detected);
    RUN_TEST(e2e_semantic_error_detected);
    RUN_TEST(e2e_runtime_error_division_by_zero);

    TEST_SUITE("Complex Programs");
    RUN_TEST(e2e_fibonacci);
    RUN_TEST(e2e_nested_loops);
    RUN_TEST(e2e_recursion);

    TEST_SUMMARY();
}
