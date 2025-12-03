#include "../test_framework.h"
#include "vm/chunk.h"
#include "vm/object.h"
#include "vm/gc.h"
#include "vm/debug.h"
#include <string.h>
#include <unistd.h>

// Setup/teardown for string interning
static void setup(void) {
    gc_init();
    strings_init();
}

static void teardown(void) {
    // Free all objects created during test
    gc_free_all();
    strings_free();
}

// ============================================================================
// Opcode Tests
// ============================================================================

TEST(opcode_names) {
    ASSERT_STR_EQ(opcode_name(OP_CONSTANT), "OP_CONSTANT");
    ASSERT_STR_EQ(opcode_name(OP_ADD), "OP_ADD");
    ASSERT_STR_EQ(opcode_name(OP_RETURN), "OP_RETURN");
    ASSERT_STR_EQ(opcode_name(OP_JUMP), "OP_JUMP");
    ASSERT_STR_EQ(opcode_name(OP_PRINT), "OP_PRINT");
}

TEST(opcode_modes) {
    ASSERT_EQ(opcode_mode(OP_NONE), OP_MODE_SIMPLE);
    ASSERT_EQ(opcode_mode(OP_CONSTANT), OP_MODE_CONSTANT);
    ASSERT_EQ(opcode_mode(OP_CONSTANT_LONG), OP_MODE_LONG);
    ASSERT_EQ(opcode_mode(OP_JUMP), OP_MODE_SHORT);
    ASSERT_EQ(opcode_mode(OP_CALL), OP_MODE_BYTE);
    ASSERT_EQ(opcode_mode(OP_INVOKE), OP_MODE_INVOKE);
    ASSERT_EQ(opcode_mode(OP_CLOSURE), OP_MODE_CLOSURE);
}

// ============================================================================
// Chunk Basic Tests
// ============================================================================

TEST(chunk_init) {
    Chunk chunk;
    chunk_init(&chunk);

    ASSERT_NULL(chunk.code);
    ASSERT_EQ(chunk.count, 0);
    ASSERT_EQ(chunk.capacity, 0);
    ASSERT_EQ(chunk.constants.count, 0);
    ASSERT_NULL(chunk.lines);
    ASSERT_EQ(chunk.line_count, 0);

    chunk_free(&chunk);
}

TEST(chunk_write) {
    Chunk chunk;
    chunk_init(&chunk);

    chunk_write(&chunk, OP_RETURN, 1);
    ASSERT_EQ(chunk.count, 1);
    ASSERT_EQ(chunk.code[0], OP_RETURN);

    chunk_write(&chunk, OP_NONE, 1);
    ASSERT_EQ(chunk.count, 2);
    ASSERT_EQ(chunk.code[1], OP_NONE);

    chunk_free(&chunk);
}

TEST(chunk_write_op) {
    Chunk chunk;
    chunk_init(&chunk);

    chunk_write_op(&chunk, OP_ADD, 5);
    chunk_write_op(&chunk, OP_SUBTRACT, 5);
    chunk_write_op(&chunk, OP_MULTIPLY, 6);

    ASSERT_EQ(chunk.count, 3);
    ASSERT_EQ(chunk.code[0], OP_ADD);
    ASSERT_EQ(chunk.code[1], OP_SUBTRACT);
    ASSERT_EQ(chunk.code[2], OP_MULTIPLY);

    chunk_free(&chunk);
}

TEST(chunk_grows) {
    Chunk chunk;
    chunk_init(&chunk);

    // Write enough bytes to force growth
    for (int i = 0; i < 100; i++) {
        chunk_write(&chunk, OP_NONE, 1);
    }

    ASSERT_EQ(chunk.count, 100);
    ASSERT(chunk.capacity >= 100);

    chunk_free(&chunk);
}

// ============================================================================
// Constant Pool Tests
// ============================================================================

TEST(chunk_add_constant) {
    setup();
    Chunk chunk;
    chunk_init(&chunk);

    int idx1 = chunk_add_constant(&chunk, NUMBER_VAL(1.0));
    int idx2 = chunk_add_constant(&chunk, NUMBER_VAL(2.5));
    int idx3 = chunk_add_constant(&chunk, BOOL_VAL(true));

    ASSERT_EQ(idx1, 0);
    ASSERT_EQ(idx2, 1);
    ASSERT_EQ(idx3, 2);

    ASSERT(values_equal(chunk.constants.values[0], NUMBER_VAL(1.0)));
    ASSERT(values_equal(chunk.constants.values[1], NUMBER_VAL(2.5)));
    ASSERT(values_equal(chunk.constants.values[2], BOOL_VAL(true)));

    chunk_free(&chunk);
    teardown();
}

TEST(chunk_write_constant) {
    setup();
    Chunk chunk;
    chunk_init(&chunk);

    chunk_write_constant(&chunk, NUMBER_VAL(42.0), 1);

    // Should write OP_CONSTANT + index
    ASSERT_EQ(chunk.count, 2);
    ASSERT_EQ(chunk.code[0], OP_CONSTANT);
    ASSERT_EQ(chunk.code[1], 0);  // First constant

    chunk_free(&chunk);
    teardown();
}

TEST(chunk_write_constant_long) {
    setup();
    Chunk chunk;
    chunk_init(&chunk);

    // Add 256 constants to force long constant usage
    for (int i = 0; i < 256; i++) {
        chunk_add_constant(&chunk, NUMBER_VAL((double)i));
    }

    // This one should use OP_CONSTANT_LONG
    chunk_write_constant(&chunk, NUMBER_VAL(999.0), 1);

    // Should write OP_CONSTANT_LONG + 3-byte index
    ASSERT_EQ(chunk.code[0], OP_CONSTANT_LONG);
    // Index 256 in little-endian: 0x00, 0x01, 0x00
    ASSERT_EQ(chunk.code[1], 0);
    ASSERT_EQ(chunk.code[2], 1);
    ASSERT_EQ(chunk.code[3], 0);

    chunk_free(&chunk);
    teardown();
}

// ============================================================================
// Line Number Tests
// ============================================================================

TEST(chunk_line_simple) {
    Chunk chunk;
    chunk_init(&chunk);

    chunk_write(&chunk, OP_NONE, 10);
    chunk_write(&chunk, OP_RETURN, 10);

    ASSERT_EQ(chunk_get_line(&chunk, 0), 10);
    ASSERT_EQ(chunk_get_line(&chunk, 1), 10);

    chunk_free(&chunk);
}

TEST(chunk_line_multiple) {
    Chunk chunk;
    chunk_init(&chunk);

    chunk_write(&chunk, OP_NONE, 1);
    chunk_write(&chunk, OP_TRUE, 1);
    chunk_write(&chunk, OP_FALSE, 2);
    chunk_write(&chunk, OP_POP, 2);
    chunk_write(&chunk, OP_RETURN, 3);

    ASSERT_EQ(chunk_get_line(&chunk, 0), 1);
    ASSERT_EQ(chunk_get_line(&chunk, 1), 1);
    ASSERT_EQ(chunk_get_line(&chunk, 2), 2);
    ASSERT_EQ(chunk_get_line(&chunk, 3), 2);
    ASSERT_EQ(chunk_get_line(&chunk, 4), 3);

    chunk_free(&chunk);
}

TEST(chunk_line_run_length) {
    Chunk chunk;
    chunk_init(&chunk);

    // Many instructions on same line should be run-length encoded
    for (int i = 0; i < 50; i++) {
        chunk_write(&chunk, OP_NONE, 100);
    }

    // Should only have 2 entries (line, count) for all 50 instructions
    ASSERT_EQ(chunk.line_count, 2);
    ASSERT_EQ(chunk.lines[0], 100);  // Line number
    ASSERT_EQ(chunk.lines[1], 50);   // Count

    for (int i = 0; i < 50; i++) {
        ASSERT_EQ(chunk_get_line(&chunk, i), 100);
    }

    chunk_free(&chunk);
}

// ============================================================================
// Serialization Tests
// ============================================================================

TEST(chunk_serialize_empty) {
    Chunk chunk;
    chunk_init(&chunk);

    const char* path = "/tmp/test_chunk_empty.plbc";
    ASSERT(chunk_write_file(&chunk, path));

    Chunk* loaded = chunk_read_file(path);
    ASSERT_NOT_NULL(loaded);
    ASSERT_EQ(loaded->count, 0);
    ASSERT_EQ(loaded->constants.count, 0);

    chunk_free(loaded);
    PH_FREE(loaded);
    chunk_free(&chunk);
    unlink(path);
}

TEST(chunk_serialize_simple) {
    setup();
    Chunk chunk;
    chunk_init(&chunk);

    // Write some instructions
    chunk_write_constant(&chunk, NUMBER_VAL(1.0), 1);
    chunk_write_constant(&chunk, NUMBER_VAL(2.0), 1);
    chunk_write_op(&chunk, OP_ADD, 1);
    chunk_write_op(&chunk, OP_RETURN, 2);

    const char* path = "/tmp/test_chunk_simple.plbc";
    ASSERT(chunk_write_file(&chunk, path));

    Chunk* loaded = chunk_read_file(path);
    ASSERT_NOT_NULL(loaded);

    // Verify bytecode
    ASSERT_EQ(loaded->count, chunk.count);
    for (int i = 0; i < chunk.count; i++) {
        ASSERT_EQ(loaded->code[i], chunk.code[i]);
    }

    // Verify constants
    ASSERT_EQ(loaded->constants.count, chunk.constants.count);
    ASSERT(values_equal(loaded->constants.values[0], NUMBER_VAL(1.0)));
    ASSERT(values_equal(loaded->constants.values[1], NUMBER_VAL(2.0)));

    // Verify lines
    ASSERT_EQ(chunk_get_line(loaded, 0), 1);
    ASSERT_EQ(chunk_get_line(loaded, loaded->count - 1), 2);

    chunk_free(loaded);
    PH_FREE(loaded);
    chunk_free(&chunk);
    teardown();
    unlink(path);
}

TEST(chunk_serialize_strings) {
    setup();
    Chunk chunk;
    chunk_init(&chunk);

    ObjString* str1 = string_copy("hello", 5);
    ObjString* str2 = string_copy("world", 5);

    chunk_write_constant(&chunk, OBJECT_VAL(str1), 1);
    chunk_write_constant(&chunk, OBJECT_VAL(str2), 2);
    chunk_write_op(&chunk, OP_RETURN, 3);

    const char* path = "/tmp/test_chunk_strings.plbc";
    ASSERT(chunk_write_file(&chunk, path));

    Chunk* loaded = chunk_read_file(path);
    ASSERT_NOT_NULL(loaded);

    // Verify string constants
    ASSERT_EQ(loaded->constants.count, 2);

    Value v1 = loaded->constants.values[0];
    Value v2 = loaded->constants.values[1];

    ASSERT(IS_STRING(v1));
    ASSERT(IS_STRING(v2));

    ASSERT_STR_EQ(AS_CSTRING(v1), "hello");
    ASSERT_STR_EQ(AS_CSTRING(v2), "world");

    chunk_free(loaded);
    PH_FREE(loaded);
    chunk_free(&chunk);
    teardown();
    unlink(path);
}

TEST(chunk_serialize_all_value_types) {
    setup();
    Chunk chunk;
    chunk_init(&chunk);

    chunk_add_constant(&chunk, NONE_VAL);
    chunk_add_constant(&chunk, BOOL_VAL(true));
    chunk_add_constant(&chunk, BOOL_VAL(false));
    chunk_add_constant(&chunk, NUMBER_VAL(42.5));
    chunk_add_constant(&chunk, OBJECT_VAL(string_copy("test", 4)));

    const char* path = "/tmp/test_chunk_types.plbc";
    ASSERT(chunk_write_file(&chunk, path));

    Chunk* loaded = chunk_read_file(path);
    ASSERT_NOT_NULL(loaded);

    ASSERT_EQ(loaded->constants.count, 5);
    ASSERT(IS_NONE(loaded->constants.values[0]));
    ASSERT(IS_BOOL(loaded->constants.values[1]) && AS_BOOL(loaded->constants.values[1]));
    ASSERT(IS_BOOL(loaded->constants.values[2]) && !AS_BOOL(loaded->constants.values[2]));
    ASSERT(IS_NUMBER(loaded->constants.values[3]) && AS_NUMBER(loaded->constants.values[3]) == 42.5);
    ASSERT(IS_STRING(loaded->constants.values[4]));

    chunk_free(loaded);
    PH_FREE(loaded);
    chunk_free(&chunk);
    teardown();
    unlink(path);
}

TEST(chunk_read_invalid_file) {
    // Non-existent file
    Chunk* loaded = chunk_read_file("/tmp/nonexistent_file.plbc");
    ASSERT_NULL(loaded);

    // Create file with invalid magic
    const char* path = "/tmp/test_invalid.plbc";
    FILE* f = fopen(path, "wb");
    uint32_t bad_magic = 0x12345678;
    fwrite(&bad_magic, 4, 1, f);
    fclose(f);

    loaded = chunk_read_file(path);
    ASSERT_NULL(loaded);

    unlink(path);
}

// ============================================================================
// Disassembly Tests (visual verification)
// ============================================================================

TEST(disassemble_simple) {
    setup();
    Chunk chunk;
    chunk_init(&chunk);

    chunk_write_constant(&chunk, NUMBER_VAL(1.0), 1);
    chunk_write_constant(&chunk, NUMBER_VAL(2.0), 1);
    chunk_write_op(&chunk, OP_ADD, 1);
    chunk_write_op(&chunk, OP_RETURN, 2);

    printf("\n  Disassembly output:\n  ");
    disassemble_chunk(&chunk, "test");

    chunk_free(&chunk);
    teardown();
}

TEST(disassemble_jumps) {
    Chunk chunk;
    chunk_init(&chunk);

    // Simulate an if statement
    chunk_write_op(&chunk, OP_TRUE, 1);
    chunk_write_op(&chunk, OP_JUMP_IF_FALSE, 1);
    chunk_write(&chunk, 3, 1);  // Jump offset low
    chunk_write(&chunk, 0, 1);  // Jump offset high
    chunk_write_op(&chunk, OP_NONE, 2);
    chunk_write_op(&chunk, OP_POP, 2);
    chunk_write_op(&chunk, OP_RETURN, 3);

    printf("\n  Disassembly with jumps:\n  ");
    disassemble_chunk(&chunk, "jumps");

    chunk_free(&chunk);
}

TEST(disassemble_loop) {
    Chunk chunk;
    chunk_init(&chunk);

    // Simulate a loop
    chunk_write_op(&chunk, OP_NONE, 1);     // Loop body
    chunk_write_op(&chunk, OP_POP, 1);
    chunk_write_op(&chunk, OP_LOOP, 1);
    chunk_write(&chunk, 4, 1);  // Jump back 4 bytes
    chunk_write(&chunk, 0, 1);
    chunk_write_op(&chunk, OP_RETURN, 2);

    printf("\n  Disassembly with loop:\n  ");
    disassemble_chunk(&chunk, "loop");

    chunk_free(&chunk);
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Opcodes");
    RUN_TEST(opcode_names);
    RUN_TEST(opcode_modes);

    TEST_SUITE("Chunk - Basic");
    RUN_TEST(chunk_init);
    RUN_TEST(chunk_write);
    RUN_TEST(chunk_write_op);
    RUN_TEST(chunk_grows);

    TEST_SUITE("Chunk - Constants");
    RUN_TEST(chunk_add_constant);
    RUN_TEST(chunk_write_constant);
    RUN_TEST(chunk_write_constant_long);

    TEST_SUITE("Chunk - Line Numbers");
    RUN_TEST(chunk_line_simple);
    RUN_TEST(chunk_line_multiple);
    RUN_TEST(chunk_line_run_length);

    TEST_SUITE("Chunk - Serialization");
    RUN_TEST(chunk_serialize_empty);
    RUN_TEST(chunk_serialize_simple);
    RUN_TEST(chunk_serialize_strings);
    RUN_TEST(chunk_serialize_all_value_types);
    RUN_TEST(chunk_read_invalid_file);

    TEST_SUITE("Chunk - Disassembly");
    RUN_TEST(disassemble_simple);
    RUN_TEST(disassemble_jumps);
    RUN_TEST(disassemble_loop);

    TEST_SUMMARY();
}
