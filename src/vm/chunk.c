#include "vm/chunk.h"
#include "vm/object.h"
#include <string.h>

void chunk_init(Chunk* chunk) {
    chunk->code = NULL;
    chunk->count = 0;
    chunk->capacity = 0;
    value_array_init(&chunk->constants);
    chunk->lines = NULL;
    chunk->line_count = 0;
    chunk->line_capacity = 0;
}

void chunk_free(Chunk* chunk) {
    PH_FREE(chunk->code);
    value_array_free(&chunk->constants);
    PH_FREE(chunk->lines);
    chunk_init(chunk);
}

void chunk_write(Chunk* chunk, uint8_t byte, int line) {
    // Grow code array if needed
    if (chunk->count >= chunk->capacity) {
        int new_capacity = PH_GROW_CAPACITY(chunk->capacity);
        chunk->code = PH_REALLOC(chunk->code, new_capacity * sizeof(uint8_t));
        chunk->capacity = new_capacity;
    }

    chunk->code[chunk->count] = byte;
    chunk->count++;

    // Run-length encode line information
    // Lines array stores pairs: [line, count]
    if (chunk->line_count > 0 &&
        chunk->lines[chunk->line_count - 2] == line) {
        // Same line as previous, increment count
        chunk->lines[chunk->line_count - 1]++;
    } else {
        // New line, add a new entry
        if (chunk->line_count + 2 > chunk->line_capacity) {
            int new_capacity = PH_GROW_CAPACITY(chunk->line_capacity);
            chunk->lines = PH_REALLOC(chunk->lines, new_capacity * sizeof(int));
            chunk->line_capacity = new_capacity;
        }
        chunk->lines[chunk->line_count] = line;
        chunk->lines[chunk->line_count + 1] = 1;
        chunk->line_count += 2;
    }
}

void chunk_write_op(Chunk* chunk, OpCode op, int line) {
    chunk_write(chunk, (uint8_t)op, line);
}

int chunk_add_constant(Chunk* chunk, Value value) {
    value_array_write(&chunk->constants, value);
    return chunk->constants.count - 1;
}

void chunk_write_constant(Chunk* chunk, Value value, int line) {
    int index = chunk_add_constant(chunk, value);

    if (index < 256) {
        chunk_write_op(chunk, OP_CONSTANT, line);
        chunk_write(chunk, (uint8_t)index, line);
    } else {
        // Use long constant for indices >= 256
        chunk_write_op(chunk, OP_CONSTANT_LONG, line);
        chunk_write(chunk, (uint8_t)(index & 0xff), line);
        chunk_write(chunk, (uint8_t)((index >> 8) & 0xff), line);
        chunk_write(chunk, (uint8_t)((index >> 16) & 0xff), line);
    }
}

int chunk_get_line(Chunk* chunk, int offset) {
    // Decode run-length encoded lines
    int current_offset = 0;
    for (int i = 0; i < chunk->line_count; i += 2) {
        int line = chunk->lines[i];
        int count = chunk->lines[i + 1];
        current_offset += count;
        if (current_offset > offset) {
            return line;
        }
    }
    // Fallback (shouldn't happen if line info is correct)
    return 0;
}

// ============================================================================
// Bytecode Serialization
// ============================================================================

// Helper to write bytes to file
static bool write_bytes(FILE* file, const void* data, size_t size) {
    return fwrite(data, 1, size, file) == size;
}

// Helper to write a 32-bit integer
static bool write_u32(FILE* file, uint32_t value) {
    uint8_t bytes[4] = {
        (uint8_t)(value & 0xff),
        (uint8_t)((value >> 8) & 0xff),
        (uint8_t)((value >> 16) & 0xff),
        (uint8_t)((value >> 24) & 0xff)
    };
    return write_bytes(file, bytes, 4);
}

// Helper to write a double
static bool write_double(FILE* file, double value) {
    return write_bytes(file, &value, sizeof(double));
}

// Helper to read bytes from file
static bool read_bytes(FILE* file, void* data, size_t size) {
    return fread(data, 1, size, file) == size;
}

// Helper to read a 32-bit integer
static bool read_u32(FILE* file, uint32_t* value) {
    uint8_t bytes[4];
    if (!read_bytes(file, bytes, 4)) return false;
    *value = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    return true;
}

// Helper to read a double
static bool read_double(FILE* file, double* value) {
    return read_bytes(file, value, sizeof(double));
}

// Write a value to file
static bool write_value(FILE* file, Value value) {
    // Write type tag
    uint8_t type = (uint8_t)value.type;
    if (!write_bytes(file, &type, 1)) return false;

    switch (value.type) {
        case VAL_NONE:
            // No additional data
            return true;

        case VAL_BOOL:
            {
                uint8_t b = AS_BOOL(value) ? 1 : 0;
                return write_bytes(file, &b, 1);
            }

        case VAL_NUMBER:
            return write_double(file, AS_NUMBER(value));

        case VAL_OBJECT:
            {
                Object* obj = AS_OBJECT(value);
                uint8_t obj_type = (uint8_t)obj->type;
                if (!write_bytes(file, &obj_type, 1)) return false;

                switch (obj->type) {
                    case OBJ_STRING:
                        {
                            ObjString* str = (ObjString*)obj;
                            if (!write_u32(file, str->length)) return false;
                            return write_bytes(file, str->chars, str->length);
                        }

                    // Other object types are not serializable yet
                    // Functions will be handled when we have nested chunks
                    default:
                        return false;
                }
            }

        default:
            return false;
    }
}

// Read a value from file
static bool read_value(FILE* file, Value* value) {
    uint8_t type;
    if (!read_bytes(file, &type, 1)) return false;

    switch (type) {
        case VAL_NONE:
            *value = NONE_VAL;
            return true;

        case VAL_BOOL:
            {
                uint8_t b;
                if (!read_bytes(file, &b, 1)) return false;
                *value = BOOL_VAL(b != 0);
                return true;
            }

        case VAL_NUMBER:
            {
                double n;
                if (!read_double(file, &n)) return false;
                *value = NUMBER_VAL(n);
                return true;
            }

        case VAL_OBJECT:
            {
                uint8_t obj_type;
                if (!read_bytes(file, &obj_type, 1)) return false;

                switch (obj_type) {
                    case OBJ_STRING:
                        {
                            uint32_t length;
                            if (!read_u32(file, &length)) return false;

                            char* chars = PH_ALLOC(length + 1);
                            if (!chars) return false;

                            if (!read_bytes(file, chars, length)) {
                                PH_FREE(chars);
                                return false;
                            }
                            chars[length] = '\0';

                            ObjString* str = string_take(chars, (int)length);
                            *value = OBJECT_VAL(str);
                            return true;
                        }

                    default:
                        return false;
                }
            }

        default:
            return false;
    }
}

bool chunk_write_file(Chunk* chunk, const char* path) {
    FILE* file = fopen(path, "wb");
    if (!file) return false;

    bool success = true;

    // Write header
    success = success && write_u32(file, CHUNK_MAGIC);
    success = success && write_u32(file, CHUNK_VERSION);

    // Write bytecode
    success = success && write_u32(file, (uint32_t)chunk->count);
    success = success && write_bytes(file, chunk->code, chunk->count);

    // Write constants
    success = success && write_u32(file, (uint32_t)chunk->constants.count);
    for (int i = 0; i < chunk->constants.count && success; i++) {
        success = write_value(file, chunk->constants.values[i]);
    }

    // Write line info
    success = success && write_u32(file, (uint32_t)chunk->line_count);
    for (int i = 0; i < chunk->line_count && success; i++) {
        success = write_u32(file, (uint32_t)chunk->lines[i]);
    }

    fclose(file);
    return success;
}

Chunk* chunk_read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) return NULL;

    Chunk* chunk = PH_ALLOC(sizeof(Chunk));
    if (!chunk) {
        fclose(file);
        return NULL;
    }
    chunk_init(chunk);

    bool success = true;

    // Read and verify header
    uint32_t magic = 0, version = 0;
    success = success && read_u32(file, &magic);
    success = success && read_u32(file, &version);

    if (!success || magic != CHUNK_MAGIC || version != CHUNK_VERSION) {
        chunk_free(chunk);
        PH_FREE(chunk);
        fclose(file);
        return NULL;
    }

    // Read bytecode
    uint32_t code_count = 0;
    success = success && read_u32(file, &code_count);
    if (success && code_count > 0) {
        chunk->code = PH_ALLOC(code_count);
        chunk->count = (int)code_count;
        chunk->capacity = (int)code_count;
        success = read_bytes(file, chunk->code, code_count);
    }

    // Read constants
    uint32_t const_count = 0;
    success = success && read_u32(file, &const_count);
    for (uint32_t i = 0; i < const_count && success; i++) {
        Value value;
        success = read_value(file, &value);
        if (success) {
            value_array_write(&chunk->constants, value);
        }
    }

    // Read line info
    uint32_t line_count = 0;
    success = success && read_u32(file, &line_count);
    if (success && line_count > 0) {
        chunk->lines = PH_ALLOC(line_count * sizeof(int));
        chunk->line_count = (int)line_count;
        chunk->line_capacity = (int)line_count;
        for (uint32_t i = 0; i < line_count && success; i++) {
            uint32_t val = 0;
            success = read_u32(file, &val);
            if (success) {
                chunk->lines[i] = (int)val;
            }
        }
    }

    fclose(file);

    if (!success) {
        chunk_free(chunk);
        PH_FREE(chunk);
        return NULL;
    }

    return chunk;
}
