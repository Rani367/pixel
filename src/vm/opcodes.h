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
    OP_LIST,            // Create list (8-bit