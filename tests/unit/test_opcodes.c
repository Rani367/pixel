#include "../test_framework.h"
#include "vm/opcodes.h"

// ============================================================================
// Opcode Name Tests
// ============================================================================

TEST(opcode_name_all_opcodes) {
    // Test all opcode names
    ASSERT_STR_EQ(opcode_name(OP_CONSTANT), "OP_CONSTANT");
    ASSERT_STR_EQ(opcode_name(OP_CONSTANT_LONG), "OP_CONSTANT_LONG");
    ASSERT_STR_EQ(opcode_name(OP_NONE), "OP_NONE");
    ASSERT_STR_EQ(opcode_name(OP_TRUE), "OP_TRUE");
    ASSERT_STR_EQ(opcode_name(OP_FALSE), "OP_FALSE");
    ASSERT_STR_EQ(opcode_name(OP_POP), "OP_POP");
    ASSERT_STR_EQ(opcode_name(OP_POPN), "OP_POPN");
    ASSERT_STR_EQ(opcode_name(OP_DUP), "OP_DUP");
    ASSERT_STR_EQ(opcode_name(OP_GET_LOCAL), "OP_GET_LOCAL");
    ASSERT_STR_EQ(opcode_name(OP_SET_LOCAL), "OP_SET_LOCAL");
    ASSERT_STR_EQ(opcode_name(OP_GET_GLOBAL), "OP_GET_GLOBAL");
    ASSERT_STR_EQ(opcode_name(OP_SET_GLOBAL), "OP_SET_GLOBAL");
    ASSERT_STR_EQ(opcode_name(OP_GET_UPVALUE), "OP_GET_UPVALUE");
    ASSERT_STR_EQ(opcode_name(OP_SET_UPVALUE), "OP_SET_UPVALUE");
    ASSERT_STR_EQ(opcode_name(OP_ADD), "OP_ADD");
    ASSERT_STR_EQ(opcode_name(OP_SUBTRACT), "OP_SUBTRACT");
    ASSERT_STR_EQ(opcode_name(OP_MULTIPLY), "OP_MULTIPLY");
    ASSERT_STR_EQ(opcode_name(OP_DIVIDE), "OP_DIVIDE");
    ASSERT_STR_EQ(opcode_name(OP_MODULO), "OP_MODULO");
    ASSERT_STR_EQ(opcode_name(OP_NEGATE), "OP_NEGATE");
    ASSERT_STR_EQ(opcode_name(OP_EQUAL), "OP_EQUAL");
    ASSERT_STR_EQ(opcode_name(OP_NOT_EQUAL), "OP_NOT_EQUAL");
    ASSERT_STR_EQ(opcode_name(OP_GREATER), "OP_GREATER");
    ASSERT_STR_EQ(opcode_name(OP_GREATER_EQUAL), "OP_GREATER_EQUAL");
    ASSERT_STR_EQ(opcode_name(OP_LESS), "OP_LESS");
    ASSERT_STR_EQ(opcode_name(OP_LESS_EQUAL), "OP_LESS_EQUAL");
    ASSERT_STR_EQ(opcode_name(OP_NOT), "OP_NOT");
    ASSERT_STR_EQ(opcode_name(OP_JUMP), "OP_JUMP");
    ASSERT_STR_EQ(opcode_name(OP_JUMP_IF_FALSE), "OP_JUMP_IF_FALSE");
    ASSERT_STR_EQ(opcode_name(OP_JUMP_IF_TRUE), "OP_JUMP_IF_TRUE");
    ASSERT_STR_EQ(opcode_name(OP_LOOP), "OP_LOOP");
    ASSERT_STR_EQ(opcode_name(OP_CALL), "OP_CALL");
    ASSERT_STR_EQ(opcode_name(OP_RETURN), "OP_RETURN");
    ASSERT_STR_EQ(opcode_name(OP_CLOSURE), "OP_CLOSURE");
    ASSERT_STR_EQ(opcode_name(OP_CLOSE_UPVALUE), "OP_CLOSE_UPVALUE");
    ASSERT_STR_EQ(opcode_name(OP_GET_PROPERTY), "OP_GET_PROPERTY");
    ASSERT_STR_EQ(opcode_name(OP_SET_PROPERTY), "OP_SET_PROPERTY");
    ASSERT_STR_EQ(opcode_name(OP_STRUCT), "OP_STRUCT");
    ASSERT_STR_EQ(opcode_name(OP_METHOD), "OP_METHOD");
    ASSERT_STR_EQ(opcode_name(OP_INVOKE), "OP_INVOKE");
    ASSERT_STR_EQ(opcode_name(OP_LIST), "OP_LIST");
    ASSERT_STR_EQ(opcode_name(OP_INDEX_GET), "OP_INDEX_GET");
    ASSERT_STR_EQ(opcode_name(OP_INDEX_SET), "OP_INDEX_SET");
    ASSERT_STR_EQ(opcode_name(OP_PRINT), "OP_PRINT");
}

TEST(opcode_name_invalid_returns_unknown) {
    ASSERT_STR_EQ(opcode_name((OpCode)-1), "OP_UNKNOWN");
    ASSERT_STR_EQ(opcode_name((OpCode)999), "OP_UNKNOWN");
    ASSERT_STR_EQ(opcode_name(OP_COUNT), "OP_UNKNOWN");
}

// ============================================================================
// Opcode Mode Tests
// ============================================================================

TEST(opcode_mode_simple) {
    // Simple instructions have no operands
    ASSERT_EQ(opcode_mode(OP_NONE), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_TRUE), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_FALSE), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_POP), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_DUP), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_ADD), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_SUBTRACT), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_MULTIPLY), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_DIVIDE), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_MODULO), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_NEGATE), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_NOT), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_RETURN), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_CLOSE_UPVALUE), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_INDEX_GET), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_INDEX_SET), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_PRINT), OP_MODE_SIMPLE);
}

TEST(opcode_mode_byte) {
    // Byte operand instructions
    ASSERT_EQ(opcode_mode(OP_POPN), OP_MODE_BYTE);
    ASSERT_EQ(opcode_mode(OP_GET_LOCAL), OP_MODE_BYTE);
    ASSERT_EQ(opcode_mode(OP_SET_LOCAL), OP_MODE_BYTE);
    ASSERT_EQ(opcode_mode(OP_GET_UPVALUE), OP_MODE_BYTE);
    ASSERT_EQ(opcode_mode(OP_SET_UPVALUE), OP_MODE_BYTE);
    ASSERT_EQ(opcode_mode(OP_CALL), OP_MODE_BYTE);
    ASSERT_EQ(opcode_mode(OP_LIST), OP_MODE_BYTE);
}

TEST(opcode_mode_constant) {
    // Constant instructions
    ASSERT_EQ(opcode_mode(OP_CONSTANT), OP_MODE_CONSTANT);
    ASSERT_EQ(opcode_mode(OP_GET_GLOBAL), OP_MODE_CONSTANT);
    ASSERT_EQ(opcode_mode(OP_SET_GLOBAL), OP_MODE_CONSTANT);
    ASSERT_EQ(opcode_mode(OP_GET_PROPERTY), OP_MODE_CONSTANT);
    ASSERT_EQ(opcode_mode(OP_SET_PROPERTY), OP_MODE_CONSTANT);
    ASSERT_EQ(opcode_mode(OP_STRUCT), OP_MODE_CONSTANT);
    ASSERT_EQ(opcode_mode(OP_METHOD), OP_MODE_CONSTANT);
}

TEST(opcode_mode_jump) {
    // Jump instructions (short/2-byte operands)
    ASSERT_EQ(opcode_mode(OP_JUMP), OP_MODE_SHORT);
    ASSERT_EQ(opcode_mode(OP_JUMP_IF_FALSE), OP_MODE_SHORT);
    ASSERT_EQ(opcode_mode(OP_JUMP_IF_TRUE), OP_MODE_SHORT);
    ASSERT_EQ(opcode_mode(OP_LOOP), OP_MODE_SHORT);
}

TEST(opcode_mode_long) {
    // Long constant instruction
    ASSERT_EQ(opcode_mode(OP_CONSTANT_LONG), OP_MODE_LONG);
}

TEST(opcode_mode_closure) {
    // Closure instruction
    ASSERT_EQ(opcode_mode(OP_CLOSURE), OP_MODE_CLOSURE);
}

TEST(opcode_mode_invoke) {
    // Invoke instruction
    ASSERT_EQ(opcode_mode(OP_INVOKE), OP_MODE_INVOKE);
}

TEST(opcode_mode_invalid_defaults_simple) {
    // Invalid opcodes default to simple mode
    ASSERT_EQ(opcode_mode((OpCode)-1), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode((OpCode)999), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_COUNT), OP_MODE_SIMPLE);
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Opcode Names");
    RUN_TEST(opcode_name_all_opcodes);
    RUN_TEST(opcode_name_invalid_returns_unknown);

    TEST_SUITE("Opcode Modes");
    RUN_TEST(opcode_mode_simple);
    RUN_TEST(opcode_mode_byte);
    RUN_TEST(opcode_mode_constant);
    RUN_TEST(opcode_mode_jump);
    RUN_TEST(opcode_mode_long);
    RUN_TEST(opcode_mode_closure);
    RUN_TEST(opcode_mode_invoke);
    RUN_TEST(opcode_mode_invalid_defaults_simple);

    TEST_SUMMARY();
}
