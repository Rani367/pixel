#include "vm/opcodes.h"

// Opcode names for disassembly
static const char* opcode_names[] = {
    [OP_CONSTANT]       = "OP_CONSTANT",
    [OP_CONSTANT_LONG]  = "OP_CONSTANT_LONG",
    [OP_NONE]            = "OP_NONE",
    [OP_TRUE]           = "OP_TRUE",
    [OP_FALSE]          = "OP_FALSE",
    [OP_POP]            = "OP_POP",
    [OP_POPN]           = "OP_POPN",
    [OP_DUP]            = "OP_DUP",
    [OP_GET_LOCAL]      = "OP_GET_LOCAL",
    [OP_SET_LOCAL]      = "OP_SET_LOCAL",
    [OP_GET_GLOBAL]     = "OP_GET_GLOBAL",
    [OP_SET_GLOBAL]     = "OP_SET_GLOBAL",
    [OP_GET_UPVALUE]    = "OP_GET_UPVALUE",
    [OP_SET_UPVALUE]    = "OP_SET_UPVALUE",
    [OP_ADD]            = "OP_ADD",
    [OP_SUBTRACT]       = "OP_SUBTRACT",
    [OP_MULTIPLY]       = "OP_MULTIPLY",
    [OP_DIVIDE]         = "OP_DIVIDE",
    [OP_MODULO]         = "OP_MODULO",
    [OP_NEGATE]         = "OP_NEGATE",
    [OP_EQUAL]          = "OP_EQUAL",
    [OP_NOT_EQUAL]      = "OP_NOT_EQUAL",
    [OP_GREATER]        = "OP_GREATER",
    [OP_GREATER_EQUAL]  = "OP_GREATER_EQUAL",
    [OP_LESS]           = "OP_LESS",
    [OP_LESS_EQUAL]     = "OP_LESS_EQUAL",
    [OP_NOT]            = "OP_NOT",
    [OP_JUMP]           = "OP_JUMP",
    [OP_JUMP_IF_FALSE]  = "OP_JUMP_IF_FALSE",
    [OP_JUMP_IF_TRUE]   = "OP_JUMP_IF_TRUE",
    [OP_LOOP]           = "OP_LOOP",
    [OP_CALL]           = "OP_CALL",
    [OP_RETURN]         = "OP_RETURN",
    [OP_CLOSURE]        = "OP_CLOSURE",
    [OP_CLOSE_UPVALUE]  = "OP_CLOSE_UPVALUE",
    [OP_GET_PROPERTY]   = "OP_GET_PROPERTY",
    [OP_SET_PROPERTY]   = "OP_SET_PROPERTY",
    [OP_STRUCT]         = "OP_STRUCT",
    [OP_METHOD]         = "OP_METHOD",
    [OP_INVOKE]         = "OP_INVOKE",
    [OP_LIST]           = "OP_LIST",
    [OP_INDEX_GET]      = "OP_INDEX_GET",
    [OP_INDEX_SET]      = "OP_INDEX_SET",
    [OP_PRINT]          = "OP_PRINT",
};

// Opcode modes for disassembly
static const OpMode opcode_modes[] = {
    [OP_CONSTANT]       = OP_MODE_CONSTANT,
    [OP_CONSTANT_LONG]  = OP_MODE_LONG,
    [OP_NONE]            = OP_MODE_SIMPLE,
    [OP_TRUE]           = OP_MODE_SIMPLE,
    [OP_FALSE]          = OP_MODE_SIMPLE,
    [OP_POP]            = OP_MODE_SIMPLE,
    [OP_POPN]           = OP_MODE_BYTE,
    [OP_DUP]            = OP_MODE_SIMPLE,
    [OP_GET_LOCAL]      = OP_MODE_BYTE,
    [OP_SET_LOCAL]      = OP_MODE_BYTE,
    [OP_GET_GLOBAL]     = OP_MODE_CONSTANT,
    [OP_SET_GLOBAL]     = OP_MODE_CONSTANT,
    [OP_GET_UPVALUE]    = OP_MODE_BYTE,
    [OP_SET_UPVALUE]    = OP_MODE_BYTE,
    [OP_ADD]            = OP_MODE_SIMPLE,
    [OP_SUBTRACT]       = OP_MODE_SIMPLE,
    [OP_MULTIPLY]       = OP_MODE_SIMPLE,
    [OP_DIVIDE]         = OP_MODE_SIMPLE,
    [OP_MODULO]         = OP_MODE_SIMPLE,
    [OP_NEGATE]         = OP_MODE_SIMPLE,
    [OP_EQUAL]          = OP_MODE_SIMPLE,
    [OP_NOT_EQUAL]      = OP_MODE_SIMPLE,
    [OP_GREATER]        = OP_MODE_SIMPLE,
    [OP_GREATER_EQUAL]  = OP_MODE_SIMPLE,
    [OP_LESS]           = OP_MODE_SIMPLE,
    [OP_LESS_EQUAL]     = OP_MODE_SIMPLE,
    [OP_NOT]            = OP_MODE_SIMPLE,
    [OP_JUMP]           = OP_MODE_SHORT,
    [OP_JUMP_IF_FALSE]  = OP_MODE_SHORT,
    [OP_JUMP_IF_TRUE]   = OP_MODE_SHORT,
    [OP_LOOP]           = OP_MODE_SHORT,
    [OP_CALL]           = OP_MODE_BYTE,
    [OP_RETURN]         = OP_MODE_SIMPLE,
    [OP_CLOSURE]        = OP_MODE_CLOSURE,
    [OP_CLOSE_UPVALUE]  = OP_MODE_SIMPLE,
    [OP_GET_PROPERTY]   = OP_MODE_CONSTANT,
    [OP_SET_PROPERTY]   = OP_MODE_CONSTANT,
    [OP_STRUCT]         = OP_MODE_CONSTANT,
    [OP_METHOD]         = OP_MODE_CONSTANT,
    [OP_INVOKE]         = OP_MODE_INVOKE,
    [OP_LIST]           = OP_MODE_BYTE,
    [OP_INDEX_GET]      = OP_MODE_SIMPLE,
    [OP_INDEX_SET]      = OP_MODE_SIMPLE,
    [OP_PRINT]          = OP_MODE_SIMPLE,
};

const char* opcode_name(OpCode op) {
    if (op >= 0 && op < OP_COUNT) {
        return opcode_names[op];
    }
    return "OP_UNKNOWN";
}

OpMode opcode_mode(OpCode op) {
    if (op >= 0 && op < OP_COUNT) {
        return opcode_modes[op];
    }
    return OP_MODE_SIMPLE;
}
