ode(OpCode op) {
    if (op >= 0 && op < OP_COUNT) {
        return opcode_modes[op];
    }
    return OP_MODE_SIMPLE;
}
