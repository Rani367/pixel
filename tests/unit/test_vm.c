       "result_y = p.y"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("result_x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 10);

    ASSERT(get_global("result_y", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 20);

    teardown();
}

TEST(struct_positional_with_methods) {
    setup();
    InterpretResult result = run_source(
        "struct Player {\n"
        "    health, name,\n"
        "    function take_damage(amount) {\n"
        "        this.health = this.health - amount\n"
        "    }\n"
        "}\n"
        "p = Player(100, \"Hero\")\n"
        "p.take_damage(30)\n"
        "result = p.health"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("result", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 70);

    teardown();
}

// ============================================================================
// Increment/Decrement Tests
// ============================================================================

TEST(postfix_increment) {
    setup();
    InterpretResult result = run_source(
        "x = 5\n"
        "y = x++\n"
        "z = x"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("y", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 5);  // y gets old value

    ASSERT(get_global("z", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 6);  // x was incremented

    teardown();
}

TEST(postfix_decrement) {
    setup();
    InterpretResult result = run_source(
        "x = 10\n"
        "y = x--\n"
        "z = x"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("y", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 10);  // y gets old value

    ASSERT(get_global("z", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 9);   // x was decremented

    teardown();
}

TEST(postfix_in_loop) {
    setup();
    InterpretResult result = run_source(
        "i = 0\n"
        "sum = 0\n"
        "while i < 5 {\n"
        "    sum = sum + i++\n"
        "}\n"
        "result = sum"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("result", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 10);  // 0+1+2+3+4 = 10

    ASSERT(get_global("i", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 5);

    teardown();
}

// ============================================================================
// Runtime Error Tests
// ============================================================================

TEST(error_type_arithmetic) {
    setup();
    InterpretResult result = run_source("x = 1 + \"hello\"");
    ASSERT_EQ(result, INTERPRET_RUNTIME_ERROR);
    teardown();
}

TEST(error_undefined_variable) {
    setup();
    // Note: The analyzer catches undefined variables at compile time,
    // so this returns COMPILE_ERROR, not RUNTIME_ERROR
    InterpretResult result = run_source("x = undefined_var");
    ASSERT_EQ(result, INTERPRET_COMPILE_ERROR);
    teardown();
}

TEST(error_call_non_function) {
    setup();
    InterpretResult result = run_source("x = 42\nx()");
    ASSERT_EQ(result, INTERPRET_RUNTIME_ERROR);
    teardown();
}

TEST(error_wrong_arity) {
    setup();
    InterpretResult result = run_source(
        "function add(a, b) { return a + b }\n"
        "x = add(1)"
    );
    ASSERT_EQ(result, INTERPRET_RUNTIME_ERROR);
    teardown();
}

TEST(error_index_out_of_bounds) {
    setup();
    InterpretResult result = run_source(
        "arr = [1, 2, 3]\n"
        "x = arr[10]"
    );
    ASSERT_EQ(result, INTERPRET_RUNTIME_ERROR);
    teardown();
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("VM - Arithmetic");
    RUN_TEST(arithmetic_add);
    RUN_TEST(arithmetic_subtract);
    RUN_TEST(arithmetic_multiply);
    RUN_TEST(arithmetic_divide);
    RUN_TEST(arithmetic_modulo);
    RUN_TEST(arithmetic_negate);
    RUN_TEST(arithmetic_complex);

    TEST_SUITE("VM - Strings");
    RUN_TEST(string_concat);

    TEST_SUITE("VM - Comparisons");
    RUN_TEST(comparison_equal);
    RUN_TEST(comparison_not_equal);
    RUN_TEST(comparison_less);
    RUN_TEST(comparison_less_equal);
    RUN_TEST(comparison_greater);
    RUN_TEST(comparison_greater_equal);

    TEST_SUITE("VM - Logical");
    RUN_TEST(logical_not);
    RUN_TEST(logical_and);
    RUN_TEST(logical_or);

    TEST_SUITE("VM - Variables");
    RUN_TEST(global_variable);

    TEST_SUITE("VM - Control Flow");
    RUN_TEST(if_true);
    RUN_TEST(if_false);
    RUN_TEST(if_else_true);
    RUN_TEST(if_else_false);
    RUN_TEST(while_loop);
    RUN_TEST(nested_while);

    TEST_SUITE("VM - Functions");
    RUN_TEST(function_simple);
    RUN_TEST(function_recursion);
    RUN_TEST(function_fibonacci);

    TEST_SUITE("VM - Closures");
    RUN_TEST(closure_simple);
    RUN_TEST(closure_multiple);

    TEST_SUITE("VM - Lists");
    RUN_TEST(list_create);
    RUN_TEST(list_index_get);
    RUN_TEST(list_index_set);
    RUN_TEST(list_negative_index_get);
    RUN_TEST(list_negative_index_set);
    RUN_TEST(string_negative_index);

    TEST_SUITE("VM - Structs");
    RUN_TEST(struct_create);
    RUN_TEST(struct_property_access);
    RUN_TEST(struct_method_simple);
    RUN_TEST(struct_method_with_params);
    RUN_TEST(struct_method_return_value);
    RUN_TEST(struct_positional_constructor);
    RUN_TEST(struct_positional_with_methods);

    TEST_SUITE("VM - Increment/Decrement");
    RUN_TEST(postfix_increment);
    RUN_TEST(postfix_decrement);
    RUN_TEST(postfix_in_loop);

    TEST_SUITE("VM - Runtime Errors");
    RUN_TEST(error_type_arithmetic);
    RUN_TEST(error_undefined_variable);
    RUN_TEST(error_call_non_function);
    RUN_TEST(error_wrong_arity);
    RUN_TEST(error_index_out_of_bounds);

    TEST_SUMMARY();
}
#include "../test_framework.h"
#include "core/arena.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/analyzer.h"
#include "compiler/codegen.h"
#include "vm/vm.h"
#include "vm/object.h"
#include <string.h>
#include <math.h>

// Global VM for tests
static VM vm;

// Helper to compile and run source code
static InterpretResult run_source(const char* source) {
    Arena* arena = arena_new(1024 * 64);

    Parser parser;
    parser_init(&parser, source, arena);

    int count = 0;
    Stmt** statements = parser_parse(&parser, &count);
    if (statements == NULL || parser_had_error(&parser)) {
        arena_free(arena);
        return INTERPRET_COMPILE_ERROR;
    }

    Analyzer analyzer;
    analyzer_init(&analyzer, "test", source);
    bool analyze_ok = analyzer_analyze(&analyzer, statements, count);
    if (!analyze_ok) {
        analyzer_free(&analyzer);
        arena_free(arena);
        return INTERPRET_COMPILE_ERROR;
    }

    Codegen codegen;
    codegen_init(&codegen, "test", source);
    ObjFunction* function = codegen_compile(&codegen, statements, count);

    codegen_free(&codegen);
    analyzer_free(&analyzer);
    arena_free(arena);

    if (function == NULL) {
        return INTERPRET_COMPILE_ERROR;
    }

    return vm_interpret(&vm, function);
}

// Helper to get a global variable value (returns pointer to the stored Value)
static bool get_global(const char* name, Value** out) {
    return table_get(&vm.globals, name, strlen(name), (void**)out);
}

// Setup/teardown
static void setup(void) {
    vm_init(&vm);
}

static void teardown(void) {
    vm_free(&vm);
}

// ============================================================================
// Basic Arithmetic Tests
// ============================================================================

TEST(arithmetic_add) {
    setup();
    InterpretResult result = run_source("x = 1 + 2");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 3);

    teardown();
}

TEST(arithmetic_subtract) {
    setup();
    InterpretResult result = run_source("x = 10 - 3");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 7);

    teardown();
}

TEST(arithmetic_multiply) {
    setup();
    InterpretResult result = run_source("x = 4 * 5");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 20);

    teardown();
}

TEST(arithmetic_divide) {
    setup();
    InterpretResult result = run_source("x = 20 / 4");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 5);

    teardown();
}

TEST(arithmetic_modulo) {
    setup();
    InterpretResult result = run_source("x = 17 % 5");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 2);

    teardown();
}

TEST(arithmetic_negate) {
    setup();
    InterpretResult result = run_source("x = -42");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), -42);

    teardown();
}

TEST(arithmetic_complex) {
    setup();
    InterpretResult result = run_source("x = (1 + 2) * 3 - 4 / 2");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 7);

    teardown();
}

// ============================================================================
// String Operations Tests
// ============================================================================

TEST(string_concat) {
    setup();
    InterpretResult result = run_source("x = \"hello\" + \" \" + \"world\"");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "hello world");

    teardown();
}

// ============================================================================
// Comparison Tests
// ============================================================================

TEST(comparison_equal) {
    setup();
    InterpretResult result = run_source("x = 5 == 5\ny = 5 == 6");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), true);

    ASSERT(get_global("y", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), false);

    teardown();
}

TEST(comparison_not_equal) {
    setup();
    InterpretResult result = run_source("x = 5 != 6\ny = 5 != 5");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), true);

    ASSERT(get_global("y", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), false);

    teardown();
}

TEST(comparison_less) {
    setup();
    InterpretResult result = run_source("x = 3 < 5\ny = 5 < 3");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), true);

    ASSERT(get_global("y", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), false);

    teardown();
}

TEST(comparison_less_equal) {
    setup();
    InterpretResult result = run_source("x = 3 <= 3\ny = 4 <= 3");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), true);

    ASSERT(get_global("y", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), false);

    teardown();
}

TEST(comparison_greater) {
    setup();
    InterpretResult result = run_source("x = 5 > 3\ny = 3 > 5");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), true);

    ASSERT(get_global("y", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), false);

    teardown();
}

TEST(comparison_greater_equal) {
    setup();
    InterpretResult result = run_source("x = 5 >= 5\ny = 3 >= 5");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), true);

    ASSERT(get_global("y", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), false);

    teardown();
}

// ============================================================================
// Logical Operations Tests
// ============================================================================

TEST(logical_not) {
    setup();
    InterpretResult result = run_source("x = not true\ny = not false");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), false);

    ASSERT(get_global("y", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), true);

    teardown();
}

TEST(logical_and) {
    setup();
    InterpretResult result = run_source(
        "a = true and true\n"
        "b = true and false\n"
        "c = false and true\n"
        "d = false and false"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(value_is_truthy(*val));

    ASSERT(get_global("b", &val));
    ASSERT(!value_is_truthy(*val));

    ASSERT(get_global("c", &val));
    ASSERT(!value_is_truthy(*val));

    ASSERT(get_global("d", &val));
    ASSERT(!value_is_truthy(*val));

    teardown();
}

TEST(logical_or) {
    setup();
    InterpretResult result = run_source(
        "a = true or true\n"
        "b = true or false\n"
        "c = false or true\n"
        "d = false or false"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(value_is_truthy(*val));

    ASSERT(get_global("b", &val));
    ASSERT(value_is_truthy(*val));

    ASSERT(get_global("c", &val));
    ASSERT(value_is_truthy(*val));

    ASSERT(get_global("d", &val));
    ASSERT(!value_is_truthy(*val));

    teardown();
}

// ============================================================================
// Variable Tests
// ============================================================================

TEST(global_variable) {
    setup();
    InterpretResult result = run_source(
        "x = 10\n"
        "y = x + 5\n"
        "x = 20"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 20);

    ASSERT(get_global("y", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 15);

    teardown();
}

// ============================================================================
// Control Flow Tests
// ============================================================================

TEST(if_true) {
    setup();
    InterpretResult result = run_source(
        "x = 0\n"
        "if true {\n"
        "    x = 1\n"
        "}"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 1);

    teardown();
}

TEST(if_false) {
    setup();
    InterpretResult result = run_source(
        "x = 0\n"
        "if false {\n"
        "    x = 1\n"
        "}"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 0);

    teardown();
}

TEST(if_else_true) {
    setup();
    InterpretResult result = run_source(
        "x = 0\n"
        "if true {\n"
        "    x = 1\n"
        "} else {\n"
        "    x = 2\n"
        "}"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 1);

    teardown();
}

TEST(if_else_false) {
    setup();
    InterpretResult result = run_source(
        "x = 0\n"
        "if false {\n"
        "    x = 1\n"
        "} else {\n"
        "    x = 2\n"
        "}"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 2);

    teardown();
}

TEST(while_loop) {
    setup();
    InterpretResult result = run_source(
        "x = 0\n"
        "while x < 5 {\n"
        "    x = x + 1\n"
        "}"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 5);

    teardown();
}

TEST(nested_while) {
    setup();
    InterpretResult result = run_source(
        "sum = 0\n"
        "i = 1\n"
        "while i <= 4 {\n"
        "    sum = sum + i\n"
        "    i = i + 1\n"
        "}"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("sum", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 10);  // 1 + 2 + 3 + 4

    teardown();
}

// ============================================================================
// Function Tests
// ============================================================================

TEST(function_simple) {
    setup();
    InterpretResult result = run_source(
        "function add(a, b) {\n"
        "    return a + b\n"
        "}\n"
        "x = add(3, 4)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 7);

    teardown();
}

TEST(function_recursion) {
    setup();
    InterpretResult result = run_source(
        "function factorial(n) {\n"
        "    if n <= 1 {\n"
        "        return 1\n"
        "    }\n"
        "    return n * factorial(n - 1)\n"
        "}\n"
        "x = factorial(5)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 120);

    teardown();
}

TEST(function_fibonacci) {
    setup();
    InterpretResult result = run_source(
        "function fib(n) {\n"
        "    if n < 2 {\n"
        "        return n\n"
        "    }\n"
        "    return fib(n - 1) + fib(n - 2)\n"
        "}\n"
        "x = fib(10)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
