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
