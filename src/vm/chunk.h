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
void chunk_write_constant(Chunk* chunk, Value val