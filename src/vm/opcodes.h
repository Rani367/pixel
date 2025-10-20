unctions
    OP_CALL,            // Call function (8-bit arg count)
    OP_RETURN,          // Return from function
    OP_CLOSURE,         // Create closure (8-bit constant index)
    OP_CLOSE_UPVALUE,   // Close upvalue at top of stack

    // Objects
    OP_GET_PROPERTY,    // Get property (8-bit constant index for name)
    OP_SET_PROPERTY,    // Set property (8-bit constant index for name)
    OP_STRUCT,          // Create struct instance (8-bit constant index)
    OP_METHOD,          // Define method (8-bit constant index for name)
    OP_INVOKE,          // Optimized method call (8-bit name index, 8-bit arg count)

    // Collections
    OP_LIST,            // Create list (8-bit#ifndef PH_OPCODES_H
#define PH_OPCODES_H

#include "core/common.h"

// Bytecode opcodes
typedef enum {
    // Constants
    OP_CONSTANT,        // Push constant from pool (8-bit index)
    OP_CONSTANT_LONG,   // Push constant (24-bit index)

    // Literals
    OP_NONE,            // Push none
    OP_TRUE,            // Push true
    OP_FALSE,           // Push false

    // Stack operations
    OP_POP,             // Pop one value
    OP_POPN,            // Pop N values (8-bit count)
    OP_DUP,             // Duplicate top of stack

    // Variables
    OP_GET_LOCAL,       // Get local variable (8-bit slot)
    OP_SET_LOCAL,       // Set local variable (8-bit slot)
    OP_GET_GLOBAL,      // Get global variable (8-bit constant index for name)
    OP_SET_GLOBAL,      // Set global variable (8-bit constant index for name)
    OP_GET_UPVALUE,     // Get upvalue (8-bit index)
    OP_SET_UPVALUE,     // Set upvalue (8-bit index)

    // Arithmetic
    OP_ADD,             // a + b
    OP_SUBTRACT,        // a/ Unconditional jump (16-bit offset)
    OP_JUMP_IF_FALSE,   // Jump if top is falsey (16-bit offset)
    OP_JUMP_IF_TRUE,    // Jump if top is truthy (16-bit offset)
    OP_LOOP,            // Loop backward (16-bit offset)

    // F - b
    OP_MULTIPLY,        // a * b
    OP_DIVIDE,          // a / b
    OP_MODULO,          // a % b
    OP_NEGATE,          // -a

    // Comparison
    OP_EQUAL,           // a == b
    OP_NOT_EQUAL,       // a != b
    OP_GREATER,         // a > b
    OP_GREATER_EQUAL,   // a >= b
    OP_LESS,            // a < b
    OP_LESS_EQUAL,      // a <= b

    // Logic
    OP_NOT,             // !a

    // Control flow
    OP_JUMP,            /