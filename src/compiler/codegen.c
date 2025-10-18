odegen, codegen->current->break_jumps[i]);
    }
    PH_FREE(codegen->current->break_jumps);

    // Restore previous loop state
    codegen->current->loop_start = prev_loop_start;
    codegen->current->loop_depth = prev_loop_depth;
    codegen->current->break_jumps = prev_break_jumps;
    codegen->current->break_count = prev_break_count;
    codegen->current->break_capacity = prev_break_capacity;
}

static void compile_for_stmt(Codegen* codegen, StmtFor* stmt) {
    int line = stmt->base.span.start_line;

    // Create a new scope for the loop variable
    begin_scope(codegen);

    // Compile the iterable and get an iterator
    compile_expr(codegen, stmt->iterable);

    // We need a runtime iterator - for now, assume the iterable is a list
    // and we'll iterate with an index. In a full implementation, we'd call
    // an __iter__ method.

    // Store the iterable in a local
    add_local(codegen, "__iter__", 8);
    mark_initialized(codegen);

    // Create index counter initialized to 0
    emit_constant(codegen, NUMBER_VAL(0), line);
    add_local(codegen, "__index__", 9);
    mark_initialized(codegen);

    // Save loop state
    int prev_loop_start = codegen->current->loop_start;
    int prev_loop_depth = codegen->current->loop_depth;
    int* prev_break_jumps = codegen->current->break_jumps;
    int prev_break_count = codegen->current->break_count;
    int prev_break_capacity = codegen->current->break_capacity;

    int loop_start = current_chunk(codegen)->count;
    codegen->current->loop_start = loop_start;
    codegen->current->loop_depth = codegen->current->scope_depth;
    codegen->current->break_jumps = NULL;
    codegen->current->break_count = 0;
    codegen->current->break_capacity = 0;

    // Check if index < len(iterable)
    // Get the index
    int index_slot = codegen->current->local_count - 1;
    int iter_slot = codegen->current->local_count - 2;
    emit_bytes(codegen, OP_GET_LOCAL, (uint8_t)index_slot, line);
    // Get iterable and get its length
    emit_bytes(codegen, OP_GET_LOCAL, (uint8_t)iter_slot, line);
    // We need a len() call here - for now we'll use the list's count via a property
    // Actually, we need the runtime to support this. For now, emit a simple comparison
    // that will be handled by the VM.
    // Let's use a native 'len' function call pattern
    ObjString* len_name = string_intern("len", 3);
    uint8_t len_const = make_constant(codegen, OBJECT_VAL(len_name), line);
    emit_bytes(codegen, OP_GET_GLOBAL, len_const, line);
    emit_bytes(codegen, OP_GET_LOCAL, (uint8_t)iter_slot, line);
    emit_bytes(codegen, OP_CALL, 1, line);
    emit_op(codegen, OP_LESS, line);

    int exit_jump = emit_jump(codegen, OP_JUMP_IF_FALSE, line);
    emit_op(codegen, OP_POP, line);

    // Get current element: iterable[index]
    emit_bytes(codegen, OP_GET_LOCAL, (uint8_t)iter_slot, line);
    emit_bytes(codegen, OP_GET_LOCAL, (uint8_t)index_slot, line);
    emit_op(codegen, OP_INDEX_GET, line);

    // Create the loop variable
    add_local(codegen, stmt->name.start, stmt->name.length);
    mark_initialized(codegen);

    // Compile body
    compile_stmt(codegen, stmt->body);

    // Pop loop variable
    emit_op(codegen, OP_POP, line);
    codegen->current->local_count--;

    // Increment index
    emit_bytes(codegen, OP_GET_LOCAL, (uint8_t)index_slot, line);
    emit_constant(codegen, NUMBER_VAL(1), line);
    emit_op(codegen, OP_ADD, line);
    emit_bytes(codegen, OP_SET_LOCAL, (uint8_t)index_slot, line);
    emit_op(codegen, OP_POP, line);

    // Loop back
    emit_loop(codegen, loop_start, line);

    // Patch exit
    patch_jump(codegen, exit_jump);
    emit_op(codegen, OP_POP, line);

    // Patch breaks
    for (int i = 0; i < codegen->current->break_count; i++) {
        patch_jump(codegen, codegen->current->break_jumps[i]);
    }
    PH_FREE(codegen->current->break_jumps);

    // Restore loop state
    codegen->current->loop_start = prev_loop_start;
    codegen->current->loop_depth = prev_loop_depth;
    codegen->current->break_jumps = prev_break_jumps;
    codegen->current->break_count = prev_break_count;
    codegen->current->break_capacity = prev_break_capacity;

    end_scope(codegen, stmt->base.span.end_line);
}

static void compile_return_stmt(Codegen* codegen, StmtReturn* stmt) {
    int line = stmt->base.span.start_line;

    if (codegen->current->type == TYPE_SCRIPT) {
        error_at(codegen, stmt->base.span, "Cannot return from top-level code");
        return;
    }

    if (stmt->value) {
        compile_expr(codegen, stmt->value);
    } else {
        emit_op(codegen, OP_NONE, line);
    }

    emit_op(codegen, OP_RETURN, line);
}

static void compile_break_stmt(Codegen* codegen, StmtBreak* stmt) {
    int line = stmt->base.span.start_line;

    if (codegen->current->loop_start == -1) {
        error_at(codegen, stmt->base.span, "Cannot break outside of a loop");
        return;
    }

    // Pop locals between current scope and loop scope
    Compiler* compiler = codegen->current;
    for (int i = compiler->local_count - 1;
         i >= 0 && compiler->locals[i].depth > compiler->loop_depth;
         i--) {
        if (compiler->locals[i].is_captured) {
            emit_op(codegen, OP_CLOSE_UPVALUE, line);
        } else {
            emit_op(codegen, OP_POP, line);
        }
    }

    // Emit jump to be patched later
    int jump = emit_jump(codegen, OP_JUMP, line);

    // Add to break jumps list
    if (compiler->break_count >= compiler->break_capacity) {
        int new_capacity = PH_GROW_CAPACITY(compiler->break_capacity);
        compiler->break_jumps = PH_REALLOC(compiler->break_jumps,
                                            new_capacity * sizeof(int));
        compiler->break_capacity = new_capacity;
    }
    compiler->break_jumps[compiler->break_count++] = jump;
}

static void compile_continue_stmt(Codegen* codegen, StmtContinue* stmt) {
    int line = stmt->base.span.start_line;

    if (codegen->current->loop_start == -1) {
        error_at(codegen, stmt->base.span, "Cannot continue outside of a loop");
        return;
    }

    // Pop locals between current scope and loop scope
    Compiler* compiler = codegen->current;
    for (int i = compiler->local_count - 1;
         i >= 0 && compiler->locals[i].depth > compiler->loop_depth;
         i--) {
        if (compiler->locals[i].is_captured) {
            emit_op(codegen, OP_CLOSE_UPVALUE, line);
        } else {
            emit_op(codegen, OP_POP, line);
        }
    }

    // Jump back to loop start
    emit_loop(codegen, codegen->current->loop_start, line);
}

static void compile_function(Codegen* codegen, Token name, Token* params,
                             int param_count, Stmt* body, FunctionType type, int line) {
    Compiler compiler;
    init_compiler(codegen, &compiler, type);
    codegen->current->function->name = string_intern(name.start, name.length);

    begin_scope(codegen);

    // Compile parameters
    for (int i = 0; i < param_count; i++) {
        add_local(codegen, params[i].start, params[i].length);
        mark_initialized(codegen);
    }
    codegen->current->function->arity = param_count;

    // Compile body
    if (body->type == STMT_BLOCK) {
        StmtBlock* block = (StmtBlock*)body;
        for (int i = 0; i < block->count; i++) {
            compile_stmt(codegen, block->statements[i]);
        }
    } else {
        compile_stmt(codegen, body);
    }

    ObjFunction* function = end_compiler(codegen, line);

    // Emit closure
    uint8_t constant = make_constant(codegen, OBJECT_VAL(function), line);
    emit_bytes(codegen, OP_CLOSURE, constant, line);

    // Emit upvalue info
    for (int i = 0; i < function->upvalue_count; i++) {
        emit_byte(codegen, compiler.upvalues[i].is_local ? 1 : 0, line);
        emit_byte(codegen, compiler.upvalues[i].index, line);
    }
}

static void compile_function_stmt(Codegen* codegen, StmtFunction* stmt) {
    int line = stmt->base.span.start_line;

    // Declare the function name
    uint8_t global = 0;
    if (codegen->current->scope_depth > 0) {
        declare_variable(codegen, stmt->name.start, stmt->name.length, stmt->base.span);
    } else {
        global = identifier_constant(codegen, stmt->name.start, stmt->name.length);
    }

    // Mark as initialized before compiling body (for recursion)
    if (codegen->current->scope_depth > 0) {
        mark_initialized(codegen);
    }

    compile_function(codegen, stmt->name, stmt->params, stmt->param_count,
                     stmt->body, TYPE_FUNCTION, line);

    define_variable(codegen, global, line);
}

static void compile_function_expr(Codegen* codegen, ExprFunction* expr) {
    int line = expr->base.span.start_line;

    Compiler compiler;
    init_compiler(codegen, &compiler, TYPE_FUNCTION);
    // Anonymous function has NULL name
    codegen->current->function->name = NULL;

    begin_scope(codegen);

    // Compile parameters
    for (int i = 0; i < expr->param_count; i++) {
        add_local(codegen, expr->params[i].start, expr->params[i].length);
        mark_initialized(codegen);
    }
    codegen->current->function->arity = expr->param_count;

    // Compile body
    if (expr->body->type == STMT_BLOCK) {
        StmtBlock* block = (StmtBlock*)expr->body;
        for (int i = 0; i < block->count; i++) {
            compile_stmt(codegen, block->statements[i]);
        }
    } else {
        compile_stmt(codegen, expr->body);
    }

    ObjFunction* function = end_compiler(codegen, line);

    // Emit closure
    uint8_t constant = make_constant(codegen, OBJECT_VAL(function), line);
    emit_bytes(codegen, OP_CLOSURE, constant, line);

    // Emit upvalue info
    for (int i = 0; i < function->upvalue_count; i++) {
        emit_byte(codegen, compiler.upvalues[i].is_local ? 1 : 0, line);
        emit_byte(codegen, compiler.upvalues[i].index, line);
    }
}

static void compile_method(Codegen* codegen, StmtFunction* method_stmt) {
    int line = method_stmt->base.span.start_line;
    Token name = method_stmt->name;

    Compiler compiler;
    init_compiler(codegen, &compiler, TYPE_METHOD);
    codegen->current->function->name = string_intern(name.start, name.length);

    begin_scope(codegen);

    // Note: slot 0 is already reserved for 'this' by init_compiler

    // Compile parameters (start at slot 1)
    for (int i = 0; i < method_stmt->param_count; i++) {
        add_local(codegen, method_stmt->params[i].start, method_stmt->params[i].length);
        mark_initialized(codegen);
    }
    codegen->current->function->arity = method_stmt->param_count;

    // Compile body
    Stmt* body = method_stmt->body;
    if (body->type == STMT_BLOCK) {
        StmtBlock* block = (StmtBlock*)body;
        for (int i = 0; i < block->count; i++) {
            compile_stmt(codegen, block->statements[i]);
        }
    } else {
        compile_stmt(codegen, body);
    }

    ObjFunction* function = end_compiler(codegen, line);

    // Emit closure
    uint8_t constant = make_constant(codegen, OBJECT_VAL(function), line);
    emit_bytes(codegen, OP_CLOSURE, constant, line);

    // Emit upvalue info
    for (int i = 0; i < function->upvalue_count; i++) {
        emit_byte(codegen, compiler.upvalues[i].is_local ? 1 : 0, line);
        emit_byte(codegen, compiler.upvalues[i].index, line);
    }

    // Emit OP_METHOD to bind the closure to the struct def
    // The struct def is below the closure on the stack
    uint8_t method_name = identifier_constant(codegen, name.start, name.length);
    emit_bytes(codegen, OP_METHOD, method_name, line);
}

static void compile_struct_stmt(Codegen* codegen, StmtStruct* stmt) {
    int line = stmt->base.span.start_line;

    // Create struct definition
    ObjString* name = string_intern(stmt->name.start, stmt->name.length);
    ObjStructDef* struct_def = struct_def_new(name, stmt->field_count);

    // Store field names
    for (int i = 0; i < stmt->field_count; i++) {
        struct_def->fields[i] = string_intern(stmt->fields[i].start, stmt->fields[i].length);
    }

    // Emit constant for struct definition
    uint8_t constant = make_constant(codegen, OBJECT_VAL(struct_def), line);
    emit_bytes(codegen, OP_CONSTANT, constant, line);

    // Compile and bind each method
    for (int i = 0; i < stmt->method_count; i++) {
        compile_method(codegen, (StmtFunction*)stmt->methods[i]);
    }

    // Bind struct to global name
    uint8_t global = identifier_constant(codegen, stmt->name.start, stmt->name.length);
    emit_bytes(codegen, OP_SET_GLOBAL, global, line);
    emit_op(codegen, OP_POP, line);
}

static void compile_stmt(Codegen* codegen, Stmt* stmt) {
    switch (stmt->type) {
        case STMT_EXPRESSION:
            compile_expression_stmt(codegen, (StmtExpression*)stmt);
            break;
        case STMT_ASSIGNMENT:
            compile_assignment_stmt(codegen, (StmtAssignment*)stmt);
            break;
        case STMT_BLOCK:
            compile_block_stmt(codegen, (StmtBlock*)stmt);
            break;
        case STMT_IF:
            compile_if_stmt(codegen, (StmtIf*)stmt);
            break;
        case STMT_WHILE:
            compile_while_stmt(codegen, (StmtWhile*)stmt);
            break;
        case STMT_FOR:
            compile_for_stmt(codegen, (StmtFor*)stmt);
            break;
        case STMT_RETURN:
            compile_return_stmt(codegen, (StmtReturn*)stmt);
            break;
        case STMT_BREAK:
            compile_break_stmt(codegen, (StmtBreak*)stmt);
            break;
        case STMT_CONTINUE:
            compile_continue_stmt(codegen, (StmtContinue*)stmt);
            break;
        case STMT_FUNCTION:
            compile_function_stmt(codegen, (StmtFunction*)stmt);
            break;
        case STMT_STRUCT:
            compile_struct_stmt(codegen, (StmtStruct*)stmt);
            break;
        default:
            error_at(codegen, stmt->span, "Unknown statement type");
    }
}

// ============================================================================
// Public API
// ============================================================================

void codegen_init(Codegen* codegen, const char* source_file, const char* source) {
    codegen->current = NULL;
    codegen->error_count = 0;
    codegen->had_error = false;
    codegen->panic_mode = false;
    codegen->source_file = source_file;
    codegen->source = source;
}

void codegen_free(Codegen* codegen) {
    for (int i = 0; i < codegen->error_count; i++) {
        error_free(codegen->errors[i]);
    }
}

ObjFunction* codegen_compile(Codegen* codegen, Stmt** statements, int count) {
    Compiler compiler;
    init_compiler(codegen, &compiler, TYPE_SCRIPT);

    for (int i = 0; i < count; i++) {
        compile_stmt(codegen, statements[i]);
    }

    ObjFunction* function = end_compiler(codegen, 1);

    return codegen->had_error ? NULL : function;
}

int codegen_error_count(Codegen* codegen) {
    return codegen->error_count;
}

Error* codegen_get_error(Codegen* codegen, int index) {
    if (index < 0 || index >= codegen->error_count) return NULL;
    return codegen->errors[index];
}

void codegen_print_errors(Codegen* codegen, FILE* out) {
    for (int i = 0; i < codegen->error_count; i++) {
        error_print_pretty(codegen->errors[i], codegen->source, out);
    }
}
