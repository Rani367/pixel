#include "../test_framework.h"
#include "core/arena.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/analyzer.h"
#include "compiler/codegen.h"
#include "vm/debug.h"
#include "vm/object.h"
#include "vm/gc.h"
#include <string.h>

// Helper to compile source code and return the function
static ObjFunction* compile_source(const char* source) {
    Arena* arena = arena_new(1024 * 64);

    Parser parser;
    parser_init(&parser, source, arena);

    int count = 0;
    Stmt** statements = parser_parse(&parser, &count);
    if (statements == NULL || parser_had_error(&parser)) {
        arena_free(arena);
        return NULL;
    }

    Analyzer analyzer;
    analyzer_init(&analyzer, "test", source);
    bool analyze_ok = analyzer_analyze(&analyzer, statements, count);
    if (!analyze_ok) {
        analyzer_free(&analyzer);
        arena_free(arena);
        return NULL;
    }

    Codegen codegen;
    codegen_init(&codegen, "test", source);
    ObjFunction* function = codegen_compile(&codegen, statements, count);

    codegen_free(&codegen);
    analyzer_free(&analyzer);
    arena_free(arena);

    return function;
}

// Setup/teardown for string interning
static void setup(void) {
    gc_init();
    strings_init();
}

static void teardown(void) {
    gc_free_all();
    strings_free();
}

// ============================================================================
// Literal Compilation Tests
// ============================================================================

TEST(compile_number) {
    setup();
    ObjFunction* fn = compile_source("42");
    ASSERT_NOT_NULL(fn);
    ASSERT_EQ(fn->arity, 0);

    // Should have: OP_CONSTANT, OP_POP, OP_NONE, OP_RETURN
    Chunk* chunk = fn->chunk;
    ASSERT(chunk->count >= 4);
    ASSERT_EQ(chunk->code[0], OP_CONSTANT);
    ASSERT_EQ(chunk->code[2], OP_POP);

    teardown();
}

TEST(compile_string) {
    setup();
    ObjFunction* fn = compile_source("\"hello\"");
    ASSERT_NOT_NULL(fn);

    Chunk* chunk = fn->chunk;
    ASSERT_EQ(chunk->code[0], OP_CONSTANT);

    // Constant should be a string
    Value constant = chunk->constants.values[chunk->code[1]];
    ASSERT(IS_STRING(constant));
    ASSERT_STR_EQ(AS_CSTRING(constant), "hello");

    teardown();
}

TEST(compile_boolean) {
    setup();
    ObjFunction* fn = compile_source("true");
    ASSERT_NOT_NULL(fn);

    Chunk* chunk = fn->chunk;
    ASSERT_EQ(chunk->code[0], OP_TRUE);

    fn = compile_source("false");
    ASSERT_NOT_NULL(fn);
    chunk = fn->chunk;
    ASSERT_EQ(chunk->code[0], OP_FALSE);

    teardown();
}

TEST(compile_null) {
    setup();
    ObjFunction* fn = compile_source("null");
    ASSERT_NOT_NULL(fn);

    Chunk* chunk = fn->chunk;
    ASSERT_EQ(chunk->code[0], OP_NONE);

    teardown();
}

// ============================================================================
// Expression Compilation Tests
// ============================================================================

TEST(compile_binary_arithmetic) {
    setup();
    ObjFunction* fn = compile_source("1 + 2");
    ASSERT_NOT_NULL(fn);

    Chunk* chunk = fn->chunk;
    // OP_CONSTANT 1, OP_CONSTANT 2, OP_ADD, OP_POP, OP_NONE, OP_RETURN
    ASSERT_EQ(chunk->code[0], OP_CONSTANT);
    ASSERT_EQ(chunk->code[2], OP_CONSTANT);
    ASSERT_EQ(chunk->code[4], OP_ADD);

    teardown();
}

TEST(compile_binary_comparison) {
    setup();
    ObjFunction* fn = compile_source("1 < 2");
    ASSERT_NOT_NULL(fn);

    Chunk* chunk = fn->chunk;
    ASSERT_EQ(chunk->code[4], OP_LESS);

    fn = compile_source("1 == 2");
    ASSERT_NOT_NULL(fn);
    chunk = fn->chunk;
    ASSERT_EQ(chunk->code[4], OP_EQUAL);

    teardown();
}

TEST(compile_unary) {
    setup();
    ObjFunction* fn = compile_source("-42");
    ASSERT_NOT_NULL(fn);

    Chunk* chunk = fn->chunk;
    ASSERT_EQ(chunk->code[0], OP_CONSTANT);
    ASSERT_EQ(chunk->code[2], OP_NEGATE);

    fn = compile_source("not true");
    ASSERT_NOT_NULL(fn);
    chunk = fn->chunk;
    ASSERT_EQ(chunk->code[0], OP_TRUE);
    ASSERT_EQ(chunk->code[1], OP_NOT);

    teardown();
}

TEST(compile_and) {
    setup();
    ObjFunction* fn = compile_source("true and false");
    ASSERT_NOT_NULL(fn);

    Chunk* chunk = fn->chunk;
    // OP_TRUE, OP_JUMP_IF_FALSE, ..., OP_POP, OP_FALSE
    ASSERT_EQ(chunk->code[0], OP_TRUE);
    ASSERT_EQ(chunk->code[1], OP_JUMP_IF_FALSE);

    teardown();
}

TEST(compile_or) {
    setup();
    ObjFunction* fn = compile_source("true or false");
    ASSERT_NOT_NULL(fn);

    Chunk* chunk = fn->chunk;
    ASSERT_EQ(chunk->code[0], OP_TRUE);
    ASSERT_EQ(chunk->code[1], OP_JUMP_IF_FALSE);
    // OP_JUMP follows for short-circuit

    teardown();
}

// ============================================================================
// Variable Compilation Tests
// ============================================================================

TEST(compile_global_assignment) {
    setup();
    ObjFunction* fn = compile_source("x = 42");
    ASSERT_NOT_NULL(fn);

    Chunk* chunk = fn->chunk;
    // OP_CONSTANT 42, OP_SET_GLOBAL, OP_POP
    ASSERT_EQ(chunk->code[0], OP_CONSTANT);
    ASSERT_EQ(chunk->code[2], OP_SET_GLOBAL);

    teardown();
}

TEST(compile_global_read) {
    setup();
    ObjFunction* fn = compile_source("x = 1\nx");
    ASSERT_NOT_NULL(fn);

    Chunk* chunk = fn->chunk;
    // After assignment, we have OP_GET_GLOBAL
    bool found = false;
    for (int i = 0; i < chunk->count; i++) {
        if (chunk->code[i] == OP_GET_GLOBAL) {
            found = true;
            break;
        }
    }
    ASSERT(found);

  wn();
}

// ============================================================================
// Struct Compilation Tests
// ============================================================================

TEST(compile_struct) {
    setup();
    ObjFunction* fn = compile_source("struct Point { x, y }");
    ASSERT_NOT_NULL(fn);

    // Should have OP_CONSTANT (struct def), OP_SET_GLOBAL
    Chunk* chunk = fn->chunk;
    ASSERT_EQ(chunk->code[0], OP_CONSTANT);

    // Check that the constant is a struct definition
    Value constant = chunk->constants.values[chunk->code[1]];
    ASSERT(IS_STRUCT_DEF(constant));

    ObjStructDef* def = AS_STRUCT_DEF(constant);
    ASSERT_EQ(def->field_count, 2);
    ASSERT_STR_EQ(def->name->chars, "Point");

    teardown();
}

// ============================================================================
// Disassembly Tests (visual verification)
// ============================================================================

TEST(disassemble_compiled_code) {
    setup();

    const char* source =
        "function factorial(n) {\n"
        "    if n <= 1 {\n"
        "        return 1\n"
        "    }\n"
        "    return n * factorial(n - 1)\n"
        "}\n"
        "factorial(5)";

    ObjFunction* fn = compile_source(source);
    ASSERT_NOT_NULL(fn);

    printf("\n  Compiled factorial:\n  ");
    disassemble_chunk(fn->chunk, "<script>");

    // Find and disassemble the factorial function
    for (int i = 0; i < fn->chunk->constants.count; i++) {
        Value val = fn->chunk->constants.values[i];
        if (IS_FUNCTION(val)) {
            ObjFunction* inner = AS_FUNCTION(val);
            if (inner->name != NULL) {
                printf("\n  Function %s:\n  ", inner->name->chars);
                disassemble_chunk(inner->chunk, inner->name->chars);
            }
        }
    }

    teardown();
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Codegen - Literals");
    RUN_TEST(compile_number);
    RUN_TEST(compile_string);
    RUN_TEST(compile_boolean);
    RUN_TEST(compile_null);

    TEST_SUITE("Codegen - Expressions");
    RUN_TEST(compile_binary_arithmetic);
    RUN_TEST(compile_binary_comparison);
    RUN_TEST(compile_unary);
    RUN_TEST(compile_and);
    RUN_TEST(compile_or);

    TEST_SUITE("Codegen - Variables");
    RUN_TEST(compile_global_assignment);
    RUN_TEST(compile_global_read);

    TEST_SUITE("Codegen - Control Flow");
    RUN_TEST(compile_if);
    RUN_TEST(compile_if_else);
    RUN_TEST(compile_while);

    TEST_SUITE("Codegen - Functions");
    RUN_TEST(compile_function_decl);
    RUN_TEST(compile_function_with_params);
    RUN_TEST(compile_function_call);
    RUN_TEST(compile_return);

    TEST_SUITE("Codegen - Closures");
    RUN_TEST(compile_closure_simple);

    TEST_SUITE("Codegen - Collections");
    RUN_TEST(compile_list);
    RUN_TEST(compile_index_get);
    RUN_TEST(compile_index_set);

    TEST_SUITE("Codegen - Properties");
    RUN_TEST(compile_property_get);
    RUN_TEST(compile_property_set);

    TEST_SUITE("Codegen - Structs");
    RUN_TEST(compile_struct);

    TEST_SUITE("Codegen - Disassembly");
    RUN_TEST(disassemble_compiled_code);

    TEST_SUMMARY();
}
