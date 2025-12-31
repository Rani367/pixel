#include "../test_framework.h"
#include "core/arena.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/analyzer.h"
#include "compiler/codegen.h"
#include "vm/vm.h"
#include "vm/object.h"
#include "runtime/stdlib.h"
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

    // Register standard library function names so the analyzer doesn't
    // report them as undefined
    static const char* stdlib_names[] = {
        "print", "println", "type", "to_string", "to_number",
        "abs", "floor", "ceil", "round", "min", "max", "clamp",
        "sqrt", "pow", "sin", "cos", "tan", "atan2",
        "random", "random_range", "random_int",
        "len", "push", "pop", "insert", "remove", "contains", "index_of",
        "substring", "split", "join", "upper", "lower",
        "range", "time", "clock",
        NULL
    };
    for (int i = 0; stdlib_names[i] != NULL; i++) {
        analyzer_declare_global(&analyzer, stdlib_names[i]);
    }

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

// Helper to get a global variable value
static bool get_global(const char* name, Value** out) {
    return table_get(&vm.globals, name, strlen(name), (void**)out);
}

// Setup/teardown
static void setup(void) {
    vm_init(&vm);
    stdlib_init(&vm);
}

static void teardown(void) {
    vm_free(&vm);
}

// ============================================================================
// I/O Function Tests
// ============================================================================

TEST(print_basic) {
    setup();
    InterpretResult result = run_source("print(42)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(println_basic) {
    setup();
    InterpretResult result = run_source("println(\"hello\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

// ============================================================================
// Type Function Tests
// ============================================================================

TEST(type_number) {
    setup();
    InterpretResult result = run_source("x = type(42)");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "number");

    teardown();
}

TEST(type_string) {
    setup();
    InterpretResult result = run_source("x = type(\"hello\")");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "string");

    teardown();
}

TEST(type_bool) {
    setup();
    InterpretResult result = run_source("x = type(true)");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "bool");

    teardown();
}

TEST(type_none) {
    setup();
    InterpretResult result = run_source("x = type(null)");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "none");

    teardown();
}

TEST(type_list) {
    setup();
    InterpretResult result = run_source("x = type([1, 2, 3])");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "list");

    teardown();
}

TEST(to_string_number) {
    setup();
    InterpretResult result = run_source("x = to_string(42)");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "42");

    teardown();
}

TEST(to_string_bool) {
    setup();
    InterpretResult result = run_source(
        "a = to_string(true)\n"
        "b = to_string(false)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "true");

    ASSERT(get_global("b", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "false");

    teardown();
}

TEST(to_number_valid) {
    setup();
    InterpretResult result = run_source(
        "a = to_number(\"42\")\n"
        "b = to_number(\"3.14\")\n"
        "c = to_number(\"-100\")"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 42);

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT(fabs(AS_NUMBER(*val) - 3.14) < 0.001);

    ASSERT(get_global("c", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), -100);

    teardown();
}

TEST(to_number_invalid) {
    setup();
    InterpretResult result = run_source("x = to_number(\"hello\")");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NONE(*val));

    teardown();
}

// ============================================================================
// Math Function Tests
// ============================================================================

TEST(math_abs) {
    setup();
    InterpretResult result = run_source(
        "a = abs(-5)\n"
        "b = abs(5)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 5);

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 5);

    teardown();
}

TEST(math_floor_ceil_round) {
    setup();
    InterpretResult result = run_source(
        "a = floor(3.7)\n"
        "b = ceil(3.2)\n"
        "c = round(3.5)\n"
        "d = round(3.4)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 3);

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 4);

    ASSERT(get_global("c", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 4);

    ASSERT(get_global("d", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 3);

    teardown();
}

TEST(math_min_max) {
    setup();
    InterpretResult result = run_source(
        "a = min(3, 7)\n"
        "b = max(3, 7)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 3);

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 7);

    teardown();
}

TEST(math_clamp) {
    setup();
    InterpretResult result = run_source(
        "a = clamp(5, 0, 10)\n"
        "b = clamp(-5, 0, 10)\n"
        "c = clamp(15, 0, 10)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 5);

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 0);

    ASSERT(get_global("c", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 10);

    teardown();
}

TEST(math_sqrt_pow) {
    setup();
    InterpretResult result = run_source(
        "a = sqrt(16)\n"
        "b = pow(2, 8)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 4);

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 256);

    teardown();
}

TEST(math_trig) {
    setup();
    InterpretResult result = run_source(
        "a = sin(0)\n"
        "b = cos(0)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT(fabs(AS_NUMBER(*val)) < 0.0001);  // sin(0) = 0

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT(fabs(AS_NUMBER(*val) - 1) < 0.0001);  // cos(0) = 1

    teardown();
}

TEST(math_random) {
    setup();
    InterpretResult result = run_source(
        "a = random()\n"
        "b = random()"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT(AS_NUMBER(*val) >= 0 && AS_NUMBER(*val) <= 1);

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT(AS_NUMBER(*val) >= 0 && AS_NUMBER(*val) <= 1);

    teardown();
}

TEST(math_random_range) {
    setup();
    InterpretResult result = run_source("x = random_range(10, 20)");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT(AS_NUMBER(*val) >= 10 && AS_NUMBER(*val) <= 20);

    teardown();
}

TEST(math_random_int) {
    setup();
    InterpretResult result = run_source("x = random_int(1, 6)");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    double n = AS_NUMBER(*val);
    ASSERT(n >= 1 && n <= 6);
    ASSERT(n == (int)n);  // Should be integer

    teardown();
}

// ============================================================================
// List Function Tests
// ============================================================================

TEST(list_len) {
    setup();
    InterpretResult result = run_source(
        "a = len([1, 2, 3])\n"
        "b = len([])"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 3);

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 0);

    teardown();
}

TEST(list_push_pop) {
    setup();
    InterpretResult result = run_source(
        "arr = [1, 2]\n"
        "push(arr, 3)\n"
        "a = len(arr)\n"
        "b = pop(arr)\n"
        "c = len(arr)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 3);

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 3);

    ASSERT(get_global("c", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 2);

    teardown();
}

TEST(list_insert_remove) {
    setup();
    InterpretResult result = run_source(
        "arr = [1, 3]\n"
        "insert(arr, 1, 2)\n"
        "a = arr[1]\n"
        "removed = remove(arr, 0)\n"
        "b = arr[0]"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 2);

    ASSERT(get_global("removed", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 1);

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 2);

    teardown();
}

TEST(list_contains) {
    setup();
    InterpretResult result = run_source(
        "arr = [1, 2, 3]\n"
        "a = contains(arr, 2)\n"
        "b = contains(arr, 5)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), true);

    ASSERT(get_global("b", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), false);

    teardown();
}

TEST(list_index_of) {
    setup();
    InterpretResult result = run_source(
        "arr = [10, 20, 30]\n"
        "a = index_of(arr, 20)\n"
        "b = index_of(arr, 99)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 1);

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), -1);

    teardown();
}

// ============================================================================
// String Function Tests
// ============================================================================

TEST(string_len) {
    setup();
    InterpretResult result = run_source(
        "a = len(\"hello\")\n"
        "b = len(\"\")"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 5);

    ASSERT(get_global("b", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 0);

    teardown();
}

TEST(string_substring) {
    setup();
    InterpretResult result = run_source(
        "a = substring(\"hello world\", 0, 5)\n"
        "b = substring(\"hello world\", 6, 11)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "hello");

    ASSERT(get_global("b", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "world");

    teardown();
}

TEST(string_split) {
    setup();
    InterpretResult result = run_source(
        "parts = split(\"a,b,c\", \",\")\n"
        "a = parts[0]\n"
        "b = parts[1]\n"
        "c = parts[2]\n"
        "count = len(parts)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "a");

    ASSERT(get_global("b", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "b");

    ASSERT(get_global("c", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "c");

    ASSERT(get_global("count", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 3);

    teardown();
}

TEST(string_join) {
    setup();
    InterpretResult result = run_source(
        "arr = [\"a\", \"b\", \"c\"]\n"
        "x = join(arr, \"-\")"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "a-b-c");

    teardown();
}

TEST(string_upper_lower) {
    setup();
    InterpretResult result = run_source(
        "a = upper(\"hello\")\n"
        "b = lower(\"WORLD\")"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "HELLO");

    ASSERT(get_global("b", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "world");

    teardown();
}

// ============================================================================
// Utility Function Tests
// ============================================================================

TEST(range_one_arg) {
    setup();
    InterpretResult result = run_source(
        "arr = range(5)\n"
        "count = len(arr)\n"
        "first = arr[0]\n"
        "last = arr[4]"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("count", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 5);

    ASSERT(get_global("first", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 0);

    ASSERT(get_global("last", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 4);

    teardown();
}

TEST(range_two_args) {
    setup();
    InterpretResult result = run_source(
        "arr = range(2, 5)\n"
        "count = len(arr)\n"
        "first = arr[0]\n"
        "last = arr[2]"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("count", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 3);

    ASSERT(get_global("first", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 2);

    ASSERT(get_global("last", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 4);

    teardown();
}

TEST(range_three_args) {
    setup();
    InterpretResult result = run_source(
        "arr = range(0, 10, 2)\n"
        "count = len(arr)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("count", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 5);  // 0, 2, 4, 6, 8

    teardown();
}

TEST(time_and_clock) {
    setup();
    InterpretResult result = run_source(
        "t = time()\n"
        "c = clock()"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("t", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT(AS_NUMBER(*val) > 0);  // Time should be positive (Unix timestamp)

    ASSERT(get_global("c", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT(AS_NUMBER(*val) >= 0);  // Clock should be non-negative

    teardown();
}

// ============================================================================
// Error Path and Edge Case Tests
// ============================================================================

TEST(to_string_none) {
    setup();
    InterpretResult result = run_source("x = to_string(null)");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "none");

    teardown();
}

TEST(to_string_string) {
    setup();
    InterpretResult result = run_source("x = to_string(\"already a string\")");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "already a string");

    teardown();
}

TEST(to_string_list) {
    setup();
    InterpretResult result = run_source("x = to_string([1, 2, 3])");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    // List to_string returns "[list]" or similar
    ASSERT(AS_STRING(*val)->length > 0);

    teardown();
}

TEST(to_string_function) {
    setup();
    InterpretResult result = run_source(
        "function foo() { }\n"
        "x = to_string(foo)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT(AS_STRING(*val)->length > 0);

    teardown();
}

TEST(pop_empty_list) {
    setup();
    InterpretResult result = run_source(
        "arr = []\n"
        "x = pop(arr)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NONE(*val));  // pop on empty returns none

    teardown();
}

TEST(remove_out_of_bounds) {
    setup();
    InterpretResult result = run_source(
        "arr = [1, 2, 3]\n"
        "x = remove(arr, 10)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NONE(*val));  // out of bounds returns none

    teardown();
}

TEST(insert_at_end) {
    setup();
    InterpretResult result = run_source(
        "arr = [1, 2]\n"
        "insert(arr, 2, 3)\n"
        "x = arr[2]"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 3);

    teardown();
}

TEST(substring_bounds) {
    setup();
    InterpretResult result = run_source(
        "a = substring(\"hello\", 0, 100)\n"  // end > length should clamp
        "b = substring(\"hello\", 10, 20)"    // start > length returns empty
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_STRING(*val));

    ASSERT(get_global("b", &val));
    ASSERT(IS_STRING(*val));

    teardown();
}

TEST(split_no_delimiter) {
    setup();
    InterpretResult result = run_source(
        "parts = split(\"hello\", \"x\")\n"
        "count = len(parts)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("count", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 1);  // No split found, returns original

    teardown();
}

TEST(join_empty_list) {
    setup();
    InterpretResult result = run_source("x = join([], \",\")");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "");

    teardown();
}

TEST(join_single_element) {
    setup();
    InterpretResult result = run_source("x = join([\"only\"], \",\")");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    ASSERT_STR_EQ(AS_CSTRING(*val), "only");

    teardown();
}

TEST(range_reverse) {
    setup();
    // range(5, 0) with default step=1 should return empty
    InterpretResult result = run_source(
        "arr = range(5, 0)\n"
        "count = len(arr)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("count", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 0);

    teardown();
}

TEST(range_negative_step) {
    setup();
    InterpretResult result = run_source(
        "arr = range(5, 0, -1)\n"
        "count = len(arr)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("count", &val));
    ASSERT(IS_NUMBER(*val));
    // 5, 4, 3, 2, 1 = 5 elements
    ASSERT_EQ(AS_NUMBER(*val), 5);

    teardown();
}

TEST(math_tan) {
    setup();
    InterpretResult result = run_source("x = tan(0)");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT(fabs(AS_NUMBER(*val)) < 0.0001);  // tan(0) = 0

    teardown();
}

TEST(math_atan2) {
    setup();
    InterpretResult result = run_source("x = atan2(1, 1)");
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_NUMBER(*val));
    // atan2(1, 1) = pi/4 ≈ 0.785
    ASSERT(fabs(AS_NUMBER(*val) - 0.785398) < 0.001);

    teardown();
}

TEST(type_function) {
    setup();
    InterpretResult result = run_source(
        "function foo() { }\n"
        "x = type(foo)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("x", &val));
    ASSERT(IS_STRING(*val));
    // User-defined functions become closures
    ASSERT_STR_EQ(AS_CSTRING(*val), "closure");

    teardown();
}

TEST(contains_empty_list) {
    setup();
    InterpretResult result = run_source(
        "a = contains([], 42)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_BOOL(*val));
    ASSERT_EQ(AS_BOOL(*val), false);

    teardown();
}

TEST(index_of_not_found) {
    setup();
    InterpretResult result = run_source(
        "a = index_of([1, 2, 3], 99)"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("a", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), -1);  // Not found

    teardown();
}

// ============================================================================
// Type Error Tests
// ============================================================================

TEST(error_abs_non_number) {
    setup();
    InterpretResult result = run_source("x = abs(\"hello\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_floor_non_number) {
    setup();
    InterpretResult result = run_source("x = floor(\"hello\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_ceil_non_number) {
    setup();
    InterpretResult result = run_source("x = ceil(\"hello\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_round_non_number) {
    setup();
    InterpretResult result = run_source("x = round(\"hello\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_sqrt_non_number) {
    setup();
    InterpretResult result = run_source("x = sqrt(\"hello\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_pow_non_numbers) {
    setup();
    InterpretResult result = run_source("x = pow(\"a\", 2)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_min_non_numbers) {
    setup();
    InterpretResult result = run_source("x = min(\"a\", 2)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_max_non_numbers) {
    setup();
    InterpretResult result = run_source("x = max(1, \"b\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_clamp_non_numbers) {
    setup();
    InterpretResult result = run_source("x = clamp(\"a\", 0, 10)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_sin_non_number) {
    setup();
    InterpretResult result = run_source("x = sin(\"hello\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_cos_non_number) {
    setup();
    InterpretResult result = run_source("x = cos(\"hello\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_tan_non_number) {
    setup();
    InterpretResult result = run_source("x = tan(\"hello\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_atan2_non_numbers) {
    setup();
    InterpretResult result = run_source("x = atan2(\"a\", 1)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_random_range_non_numbers) {
    setup();
    InterpretResult result = run_source("x = random_range(\"a\", 10)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_random_int_non_numbers) {
    setup();
    InterpretResult result = run_source("x = random_int(\"a\", 10)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_push_non_list) {
    setup();
    InterpretResult result = run_source("x = push(42, 1)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_pop_non_list) {
    setup();
    InterpretResult result = run_source("x = pop(42)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_insert_non_list) {
    setup();
    InterpretResult result = run_source("x = insert(42, 0, \"a\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_remove_non_list) {
    setup();
    InterpretResult result = run_source("x = remove(42, 0)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_upper_non_string) {
    setup();
    InterpretResult result = run_source("x = upper(42)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_lower_non_string) {
    setup();
    InterpretResult result = run_source("x = lower(42)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_substring_non_string) {
    setup();
    InterpretResult result = run_source("x = substring(42, 0, 1)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_split_non_string) {
    setup();
    InterpretResult result = run_source("x = split(42, \",\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_join_non_list) {
    setup();
    InterpretResult result = run_source("x = join(42, \",\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_to_number_non_string) {
    setup();
    InterpretResult result = run_source("x = to_number(42)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_contains_non_list) {
    setup();
    InterpretResult result = run_source("x = contains(42, 1)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_index_of_non_list) {
    setup();
    InterpretResult result = run_source("x = index_of(42, 1)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_insert_index_type) {
    setup();
    InterpretResult result = run_source("arr = [1, 2, 3]\ninsert(arr, \"one\", 99)");
    ASSERT_EQ(result, INTERPRET_OK);  // Returns none on error
    teardown();
}

TEST(error_insert_out_of_bounds) {
    setup();
    InterpretResult result = run_source("arr = [1, 2, 3]\ninsert(arr, 100, 99)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_remove_index_type) {
    setup();
    InterpretResult result = run_source("arr = [1, 2, 3]\nremove(arr, \"one\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_substring_index_type) {
    setup();
    InterpretResult result = run_source("s = substring(\"hello\", \"a\", 3)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_join_separator_type) {
    setup();
    InterpretResult result = run_source("arr = [\"a\", \"b\"]\nresult = join(arr, 42)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_join_non_string_elements) {
    setup();
    InterpretResult result = run_source("arr = [1, 2, 3]\nresult = join(arr, \",\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_range_wrong_types) {
    setup();
    InterpretResult result = run_source("r = range(\"a\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_range_two_args_wrong_types) {
    setup();
    InterpretResult result = run_source("r = range(1, \"b\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_range_three_args_wrong_types) {
    setup();
    InterpretResult result = run_source("r = range(1, 10, \"step\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(to_string_vec2) {
    setup();
    // Vec2 needs vec2 function which isn't available in test context
    // Skip for now
    teardown();
}

TEST(to_string_instance) {
    setup();
    InterpretResult result = run_source(
        "struct Point { x, y }\n"
        "p = Point(1, 2)\n"
        "s = to_string(p)"
    );
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(to_string_struct_def) {
    setup();
    InterpretResult result = run_source(
        "struct Point { x, y }\n"
        "s = to_string(Point)"
    );
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(to_string_native_fn) {
    setup();
    InterpretResult result = run_source("s = to_string(abs)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(error_len_wrong_type) {
    setup();
    InterpretResult result = run_source("l = len(42)");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

TEST(split_empty_delimiter) {
    setup();
    InterpretResult result = run_source("parts = split(\"hello\", \"\")");
    ASSERT_EQ(result, INTERPRET_OK);
    teardown();
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Stdlib - I/O");
    RUN_TEST(print_basic);
    RUN_TEST(println_basic);

    TEST_SUITE("Stdlib - Type Functions");
    RUN_TEST(type_number);
    RUN_TEST(type_string);
    RUN_TEST(type_bool);
    RUN_TEST(type_none);
    RUN_TEST(type_list);
    RUN_TEST(to_string_number);
    RUN_TEST(to_string_bool);
    RUN_TEST(to_number_valid);
    RUN_TEST(to_number_invalid);

    TEST_SUITE("Stdlib - Math Functions");
    RUN_TEST(math_abs);
    RUN_TEST(math_floor_ceil_round);
    RUN_TEST(math_min_max);
    RUN_TEST(math_clamp);
    RUN_TEST(math_sqrt_pow);
    RUN_TEST(math_trig);
    RUN_TEST(math_random);
    RUN_TEST(math_random_range);
    RUN_TEST(math_random_int);

    TEST_SUITE("Stdlib - List Functions");
    RUN_TEST(list_len);
    RUN_TEST(list_push_pop);
    RUN_TEST(list_insert_remove);
    RUN_TEST(list_contains);
    RUN_TEST(list_index_of);

    TEST_SUITE("Stdlib - String Functions");
    RUN_TEST(string_len);
    RUN_TEST(string_substring);
    RUN_TEST(string_split);
    RUN_TEST(string_join);
    RUN_TEST(string_upper_lower);

    TEST_SUITE("Stdlib - Utility Functions");
    RUN_TEST(range_one_arg);
    RUN_TEST(range_two_args);
    RUN_TEST(range_three_args);
    RUN_TEST(time_and_clock);

    TEST_SUITE("Stdlib - Error Paths and Edge Cases");
    RUN_TEST(to_string_none);
    RUN_TEST(to_string_string);
    RUN_TEST(to_string_list);
    RUN_TEST(to_string_function);
    RUN_TEST(pop_empty_list);
    RUN_TEST(remove_out_of_bounds);
    RUN_TEST(insert_at_end);
    RUN_TEST(substring_bounds);
    RUN_TEST(split_no_delimiter);
    RUN_TEST(join_empty_list);
    RUN_TEST(join_single_element);
    RUN_TEST(range_reverse);
    RUN_TEST(range_negative_step);
    RUN_TEST(math_tan);
    RUN_TEST(math_atan2);
    RUN_TEST(type_function);
    RUN_TEST(contains_empty_list);
    RUN_TEST(index_of_not_found);

    TEST_SUITE("Stdlib - Type Errors");
    RUN_TEST(error_abs_non_number);
    RUN_TEST(error_floor_non_number);
    RUN_TEST(error_ceil_non_number);
    RUN_TEST(error_round_non_number);
    RUN_TEST(error_sqrt_non_number);
    RUN_TEST(error_pow_non_numbers);
    RUN_TEST(error_min_non_numbers);
    RUN_TEST(error_max_non_numbers);
    RUN_TEST(error_clamp_non_numbers);
    RUN_TEST(error_sin_non_number);
    RUN_TEST(error_cos_non_number);
    RUN_TEST(error_tan_non_number);
    RUN_TEST(error_atan2_non_numbers);
    RUN_TEST(error_random_range_non_numbers);
    RUN_TEST(error_random_int_non_numbers);
    RUN_TEST(error_push_non_list);
    RUN_TEST(error_pop_non_list);
    RUN_TEST(error_insert_non_list);
    RUN_TEST(error_remove_non_list);
    RUN_TEST(error_upper_non_string);
    RUN_TEST(error_lower_non_string);
    RUN_TEST(error_substring_non_string);
    RUN_TEST(error_split_non_string);
    RUN_TEST(error_join_non_list);
    RUN_TEST(error_to_number_non_string);
    RUN_TEST(error_contains_non_list);
    RUN_TEST(error_index_of_non_list);
    RUN_TEST(error_insert_index_type);
    RUN_TEST(error_insert_out_of_bounds);
    RUN_TEST(error_remove_index_type);
    RUN_TEST(error_substring_index_type);
    RUN_TEST(error_join_separator_type);
    RUN_TEST(error_join_non_string_elements);
    RUN_TEST(error_range_wrong_types);
    RUN_TEST(error_range_two_args_wrong_types);
    RUN_TEST(error_range_three_args_wrong_types);
    RUN_TEST(to_string_vec2);
    RUN_TEST(to_string_instance);
    RUN_TEST(to_string_struct_def);
    RUN_TEST(to_string_native_fn);
    RUN_TEST(error_len_wrong_type);
    RUN_TEST(split_empty_delimiter);

    TEST_SUMMARY();
}
