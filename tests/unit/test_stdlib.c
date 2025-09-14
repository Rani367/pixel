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
    ASSERT_EQ(r