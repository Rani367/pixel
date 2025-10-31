ase OP_ADD:
        case OP_SUBTRACT:
        case OP_MULTIPLY:
        case OP_DIVIDE:
        case OP_MODULO:
        case OP_NEGATE:
        case OP_EQUAL:
        case OP_NOT_EQUAL:
        case OP_GREATER:
        case OP_GREATER_EQUAL:
        case OP_LESS:
        case OP_LESS_EQUAL:
        case OP_NOT:
        case OP_RETURN:
        case OP_CLOSE_UPVALUE:
        case OP_INDEX_GET:
        case OP_INDEX_SET:
        case OP_PRINT:
            return simple_instruction(name, offset);

        // Byte operand instructions
        case OP_POPN:
        case OP_GET_LOCAL:
        case OP_SET_LOCAL:
        case OP_GET_UPVALUE:
        case OP_SET_UPVALUE:
        case OP_CALL:
        case OP_LIST:
            return byte_instruction(name, chunk, offset);

        // Constant instructions
        case OP_CONSTANT:
        case OP_GET_GLOBAL:
        case OP_SET_GLOBAL:
        case OP_GET_PROPERTY:
        case OP_SET_PROPERTY:
        case OP_STRUCT:
        case OP_METHOD:
            return constant_instruction(name, chunk, offset);

        // Long constant instruction
        case OP_CONSTANT_LONG:
            return constant_long_instruction(name, chunk, offset);

        // Jump instructions
        case OP_JUMP:
        case OP_JUMP_IF_FALSE:
        case OP_JUMP_IF_TRUE:
            return jump_instruction(name, 1, chunk, offset);

        case OP_LOOP:
            return jump_instruction(name, -1, chunk, offset);

        // Invoke instruction
        case OP_INVOKE:
            return invoke_instruction(name, chunk, offset);

        // Closure instruction
        case OP_CLOSURE:
            return closure_instruction(chunk, offset);

        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
ant(Chunk* chunk, int index) {
    printf("'");
    value_print(chunk->constants.values[index]);
    printf("'");
}

// Simple instruction (no operands)
static int simple_instruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

// Byte instruction (1-byte operand)
static int byte_instruction(const char* name, Chunk* chunk, int offset) {
    uint8_t slot = chunk->code[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

// Short instruction (2-byte operand, typically for jumps)
static int jump_instruction(const char* name, int sign, Chunk* chunk, int offset) {
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] |
                               (chunk->code[offset + 2] << 8));
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

// Constant instruction (1-byte constant index)
static int constant_instruction(const char* name, Chunk* chunk, int offset) {
    uint8_t index = chunk->code[offset + 1];
    printf("%-16s %4d ", name, index);
    print_constant(chunk, index);
    printf("\n");
    return offset + 2;
}

// Long constant instruction (3-byte constant index)
static int constant_long_instruction(const char* name, Chunk* chunk, int offset) {
    uint32_t index = chunk->code[offset + 1] |
                     (chunk->code[offset + 2] << 8) |
                     (chunk->code[offset + 3] << 16);
    printf("%-16s %4d ", name, index);
    print_constant(chunk, (int)index);
    printf("\n");
    return offset + 4;
}

// Invoke instruction (name index + arg count)
static int invoke_instruction(const char* name, Chunk* chunk, int offset) {
    uint8_t name_index = chunk->code[offset + 1];
    uint8_t arg_count = chunk->code[offset + 2];
    printf("%-16s (%d args) %4d ", name, arg_count, name_index);
    print_constant(chunk, name_index);
    printf("\n");
    return offset + 3;
}

// Clos#include "vm/debug.h"
#include "vm/object.h"

void disassemble_chunk(Chunk* chunk, const char* name) {
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;) {
        offset = disassemble_instruction(chunk, offset);
    }
}

// Print a constant value
static void print_const