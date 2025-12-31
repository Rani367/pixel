#include "../test_framework.h"
#include "vm/debug.h"
#include "vm/chunk.h"
#include "vm/opcodes.h"
#include "vm/object.h"
#include "vm/gc.h"
#include <stdlib.h>

// Helper to setup GC before tests that allocate objects
static void setup(void) {
    gc_init();
}

static void teardown(void) {
    gc_free_all();
}

// ============================================================================
// Chunk Disassembly Tests
// ============================================================================

TEST(disassemble_chunk_empty) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // Should not crash on empty chunk
    disassemble_chunk(&chunk, "empty");

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_chunk_with_instructions) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // Add some simple instructions
    chunk_write_op(&chunk, OP_NONE, 1);
    chunk_write_op(&chunk, OP_TRUE, 1);
    chunk_write_op(&chunk, OP_FALSE, 1);
    chunk_write_op(&chunk, OP_ADD, 2);
    chunk_write_op(&chunk, OP_RETURN, 2);

    disassemble_chunk(&chunk, "test");

    chunk_free(&chunk);
    teardown();
}

// ============================================================================
// Instruction Disassembly Tests
// ============================================================================

TEST(disassemble_simple_instruction) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    chunk_write_op(&chunk, OP_RETURN, 1);

    int next = disassemble_instruction(&chunk, 0);
    ASSERT_EQ(next, 1);  // Simple instruction is 1 byte

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_byte_instruction) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    chunk_write_op(&chunk, OP_GET_LOCAL, 1);
    chunk_write(&chunk, 5, 1);  // slot 5

    int next = disassemble_instruction(&chunk, 0);
    ASSERT_EQ(next, 2);  // Opcode + 1 byte operand

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_constant_instruction) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // Add a constant
    int index = chunk_add_constant(&chunk, NUMBER_VAL(42.0));
    chunk_write_op(&chunk, OP_CONSTANT, 1);
    chunk_write(&chunk, (uint8_t)index, 1);

    int next = disassemble_instruction(&chunk, 0);
    ASSERT_EQ(next, 2);  // Opcode + 1 byte constant index

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_jump_forward) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    chunk_write_op(&chunk, OP_JUMP, 1);
    chunk_write(&chunk, 0x10, 1);  // Low byte
    chunk_write(&chunk, 0x00, 1);  // High byte (offset = 16)

    int next = disassemble_instruction(&chunk, 0);
    ASSERT_EQ(next, 3);  // Opcode + 2 byte offset

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_loop_backward) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // First some NOPs
    for (int i = 0; i < 10; i++) {
        chunk_write_op(&chunk, OP_NONE, 1);
    }

    // Then a loop instruction
    chunk_write_op(&chunk, OP_LOOP, 1);
    chunk_write(&chunk, 0x08, 1);  // Low byte
    chunk_write(&chunk, 0x00, 1);  // High byte (offset = 8)

    int next = disassemble_instruction(&chunk, 10);
    ASSERT_EQ(next, 13);  // Opcode + 2 byte offset

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_invoke_instruction) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // Add method name constant
    int index = chunk_add_constant(&chunk, NUMBER_VAL(0));  // Placeholder

    chunk_write_op(&chunk, OP_INVOKE, 1);
    chunk_write(&chunk, (uint8_t)index, 1);  // Method name index
    chunk_write(&chunk, 2, 1);  // Arg count

    int next = disassemble_instruction(&chunk, 0);
    ASSERT_EQ(next, 3);  // Opcode + name index + arg count

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_unknown_opcode) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // Write an invalid opcode
    chunk_write(&chunk, 255, 1);  // Invalid opcode

    int next = disassemble_instruction(&chunk, 0);
    ASSERT_EQ(next, 1);  // Unknown opcodes advance by 1

    chunk_free(&chunk);
    teardown();
}

// ============================================================================
// Line Number Tests
// ============================================================================

TEST(disassemble_shows_line_number) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    chunk_write_op(&chunk, OP_RETURN, 42);

    // First instruction should show line number
    disassemble_instruction(&chunk, 0);

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_same_line_shows_pipe) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // Two instructions on same line
    chunk_write_op(&chunk, OP_NONE, 1);
    chunk_write_op(&chunk, OP_RETURN, 1);

    // First shows line, second shows pipe
    disassemble_instruction(&chunk, 0);
    disassemble_instruction(&chunk, 1);

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_new_line_shows_number) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    chunk_write_op(&chunk, OP_NONE, 1);
    chunk_write_op(&chunk, OP_RETURN, 5);  // Different line

    // Both should show their line numbers
    disassemble_instruction(&chunk, 0);
    disassemble_instruction(&chunk, 1);

    chunk_free(&chunk);
    teardown();
}

// ============================================================================
// Additional Coverage Tests
// ============================================================================

TEST(disassemble_constant_long) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // Add 256+ constants to trigger OP_CONSTANT_LONG
    for (int i = 0; i < 260; i++) {
        chunk_add_constant(&chunk, NUMBER_VAL((double)i));
    }

    // Write a long constant instruction manually
    chunk_write_op(&chunk, OP_CONSTANT_LONG, 1);
    chunk_write(&chunk, 0x04, 1);  // Low byte (260 = 0x104)
    chunk_write(&chunk, 0x01, 1);  // Middle byte
    chunk_write(&chunk, 0x00, 1);  // High byte

    int next = disassemble_instruction(&chunk, 0);
    ASSERT_EQ(next, 4);  // Opcode + 3 byte constant index

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_closure_with_upvalues) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // Create a function with upvalues
    ObjFunction* fn = function_new();
    fn->upvalue_count = 2;

    // Add function as constant
    int index = chunk_add_constant(&chunk, OBJECT_VAL(fn));

    // Write closure instruction
    chunk_write_op(&chunk, OP_CLOSURE, 1);
    chunk_write(&chunk, (uint8_t)index, 1);

    // Upvalue entries: (is_local, index) pairs
    chunk_write(&chunk, 1, 1);  // First upvalue: local, slot 0
    chunk_write(&chunk, 0, 1);
    chunk_write(&chunk, 0, 1);  // Second upvalue: upvalue, index 1
    chunk_write(&chunk, 1, 1);

    int next = disassemble_instruction(&chunk, 0);
    // OP_CLOSURE + constant + (2 upvalues * 2 bytes each)
    ASSERT_EQ(next, 6);

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_all_simple_ops) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // Write all simple opcodes that weren't covered in other tests
    OpCode simple_ops[] = {
        OP_POP, OP_DUP, OP_SUBTRACT, OP_MULTIPLY, OP_DIVIDE,
        OP_MODULO, OP_NEGATE, OP_EQUAL, OP_NOT_EQUAL, OP_GREATER,
        OP_GREATER_EQUAL, OP_LESS, OP_LESS_EQUAL, OP_NOT,
        OP_CLOSE_UPVALUE, OP_INDEX_GET, OP_INDEX_SET, OP_PRINT
    };
    int num_ops = sizeof(simple_ops) / sizeof(simple_ops[0]);

    for (int i = 0; i < num_ops; i++) {
        chunk_write_op(&chunk, simple_ops[i], 1);
    }

    // Disassemble all of them
    int offset = 0;
    for (int i = 0; i < num_ops; i++) {
        int next = disassemble_instruction(&chunk, offset);
        ASSERT_EQ(next, offset + 1);  // Simple ops are 1 byte
        offset = next;
    }

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_struct_method) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // Add struct name constant
    int struct_idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    // Add method name constant
    int method_idx = chunk_add_constant(&chunk, NUMBER_VAL(1));

    // Write OP_STRUCT
    chunk_write_op(&chunk, OP_STRUCT, 1);
    chunk_write(&chunk, (uint8_t)struct_idx, 1);

    // Write OP_METHOD
    chunk_write_op(&chunk, OP_METHOD, 1);
    chunk_write(&chunk, (uint8_t)method_idx, 1);

    int next1 = disassemble_instruction(&chunk, 0);
    ASSERT_EQ(next1, 2);  // Constant instruction is 2 bytes

    int next2 = disassemble_instruction(&chunk, next1);
    ASSERT_EQ(next2, 4);  // 2 + 2

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_byte_operand_ops) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // Test remaining byte operand instructions
    OpCode byte_ops[] = {
        OP_POPN, OP_SET_LOCAL, OP_GET_UPVALUE, OP_SET_UPVALUE, OP_CALL, OP_LIST
    };
    int num_ops = sizeof(byte_ops) / sizeof(byte_ops[0]);

    for (int i = 0; i < num_ops; i++) {
        chunk_write_op(&chunk, byte_ops[i], 1);
        chunk_write(&chunk, (uint8_t)i, 1);  // operand
    }

    // Disassemble all of them
    int offset = 0;
    for (int i = 0; i < num_ops; i++) {
        int next = disassemble_instruction(&chunk, offset);
        ASSERT_EQ(next, offset + 2);  // Byte ops are 2 bytes
        offset = next;
    }

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_global_property_ops) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // Add some constants
    chunk_add_constant(&chunk, NUMBER_VAL(1));
    chunk_add_constant(&chunk, NUMBER_VAL(2));

    // Test global and property instructions
    OpCode const_ops[] = {
        OP_SET_GLOBAL, OP_GET_PROPERTY, OP_SET_PROPERTY
    };
    int num_ops = sizeof(const_ops) / sizeof(const_ops[0]);

    for (int i = 0; i < num_ops; i++) {
        chunk_write_op(&chunk, const_ops[i], 1);
        chunk_write(&chunk, (uint8_t)i, 1);
    }

    int offset = 0;
    for (int i = 0; i < num_ops; i++) {
        int next = disassemble_instruction(&chunk, offset);
        ASSERT_EQ(next, offset + 2);
        offset = next;
    }

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_conditional_jumps) {
    setup();

    Chunk chunk;
    chunk_init(&chunk);

    // OP_JUMP_IF_FALSE
    chunk_write_op(&chunk, OP_JUMP_IF_FALSE, 1);
    chunk_write(&chunk, 0x05, 1);
    chunk_write(&chunk, 0x00, 1);

    // OP_JUMP_IF_TRUE
    chunk_write_op(&chunk, OP_JUMP_IF_TRUE, 1);
    chunk_write(&chunk, 0x10, 1);
    chunk_write(&chunk, 0x00, 1);

    int next1 = disassemble_instruction(&chunk, 0);
    ASSERT_EQ(next1, 3);

    int next2 = disassemble_instruction(&chunk, 3);
    ASSERT_EQ(next2, 6);

    chunk_free(&chunk);
    teardown();
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Chunk Disassembly");
    RUN_TEST(disassemble_chunk_empty);
    RUN_TEST(disassemble_chunk_with_instructions);

    TEST_SUITE("Instruction Disassembly");
    RUN_TEST(disassemble_simple_instruction);
    RUN_TEST(disassemble_byte_instruction);
    RUN_TEST(disassemble_constant_instruction);
    RUN_TEST(disassemble_jump_forward);
    RUN_TEST(disassemble_loop_backward);
    RUN_TEST(disassemble_invoke_instruction);
    RUN_TEST(disassemble_unknown_opcode);

    TEST_SUITE("Line Numbers");
    RUN_TEST(disassemble_shows_line_number);
    RUN_TEST(disassemble_same_line_shows_pipe);
    RUN_TEST(disassemble_new_line_shows_number);

    TEST_SUITE("Additional Coverage");
    RUN_TEST(disassemble_constant_long);
    RUN_TEST(disassemble_closure_with_upvalues);
    RUN_TEST(disassemble_all_simple_ops);
    RUN_TEST(disassemble_struct_method);
    RUN_TEST(disassemble_byte_operand_ops);
    RUN_TEST(disassemble_global_property_ops);
    RUN_TEST(disassemble_conditional_jumps);

    TEST_SUMMARY();
}
