codes.h"

// Bytecode chunk
// Contains the bytecode, constants, and line information for a function
typedef struct Chunk {
    // Bytecode
    uint8_t* code;
    int count;
    int capacity;

    // Constant pool
    ValueArray constants;

    // Line information (run-length encoded)
    // Each pair: [line_number, run_count]
    int* lines;
    int line_count;
    int line_capacity;
} Chunk;

// Initialize a chunk
void chunk_init(Chunk* chunk);

// Free a chunk's memory
void chunk_free(Chunk* chunk);

// Write a byte to the chunk
void chunk_write(Chunk* chunk, uint8_t byte, int line);

// Write an opcode to the chunk (convenience wrapper)
void chunk_write_op(Chunk* chunk, OpCode op, int line);

// Add a constant to the chunk, returns the index
int chunk_add_constant(Chunk* chunk, Value value);

// Write a constant instruction (handles long constants automatically)
void chunk_write_constant(Chunk* chunk, Value val#ifndef PH_CHUNK_H
#define PH_CHUNK_H

#include "core/common.h"
#include "vm/value.h"
#include "vm/opmber for a bytecode offset
int chunk_get_line(Chunk* chunk, int offset);

// ============================================================================
// Bytecode Serialization
// ============================================================================

// Magic number for bytecode files
#define CHUNK_MAGIC 0x504C4243  // "PLBC"

// Bytecode format version
#define CHUNK_VERSION 1

// Write chunk to file
// Returns true on success, false on failure
bool chunk_write_file(Chunk* chunk, const char* path);

// Read chunk from file
// Returns NULL on failure
Chunk* chunk_read_file(const char* path);

#endif // PH_CHUNK_H
ue, int line);

// Get the line nu