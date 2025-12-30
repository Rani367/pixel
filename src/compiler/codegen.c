#include "compiler/codegen.h"
#include "core/strings.h"
#include "vm/debug.h"
#include "vm/opcodes.h"

#include <string.h>
#include <stdarg.h>

// ============================================================================
// Forward Declarations
// ============================================================================

static void compile_expr(Codegen* codegen, Expr* expr);
static void compile_stmt(Codegen* codegen, Stmt* stmt);

// ============================================================================
// Helper Functions
// ============================================================================

static Chunk* current_chunk(Codegen* codegen) {
    return codegen->current->function->chunk;
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 3, 4)))
#endif
static void error_at(Codegen* codegen, Span span, const char* format, ...) {
    if (codegen->panic_mode) return;
    codegen->panic_mode = true;
    codegen->had_error = true;

    if (codegen->error_count >= CODEGEN_MAX_ERRORS) return;

    va_list args;
    va_start(args, format);
    char message[256];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    SourceLocation loc = source_location_new(
        codegen->source_file,
        span.start_line,
        span.start_column,
        span.end_column - span.start_column
    );

    // Use ERR_TYPE_MISMATCH as a general compile error (could add ERR_COMPILE later)
    Error* error = error_new(ERR_TYPE_MISMATCH, loc, "%s", message);

    codegen->errors[codegen->error_count++] = error;
}

// ============================================================================
// Bytecode Emission
// ============================================================================

static void emit_byte(Codegen* codegen, uint8_t byte, int line) {
    chunk_write(current_chunk(codegen), byte, line);
}

static void emit_bytes(Codegen* codegen, uint8_t byte1, uint8_t byte2, int line) {
    emit_byte(codegen, byte1, line);
    emit_byte(codegen, byte2, line);
}

static void emit_op(Codegen* codegen, OpCode op, int line) {
    chunk_write_op(current_chunk(codegen), op, line);
}

static int emit_jump(Codegen* codegen, OpCode op, int line) {
    emit_op(codegen, op, line);
    emit_byte(codegen, 0xff, line);  // Placeholder
    emit_byte(codegen, 0xff, line);
    return current_chunk(codegen)->count - 2;
}

static void patch_jump(Codegen* codegen, int offset) {
    Chunk* chunk = current_chunk(codegen);
    int jump = chunk->count - offset - 2;

    if (jump > UINT16_MAX) {
        error_at(codegen, (Span){0, 0, 0, 0}, "Jump offset too large");
        return;
    }

    chunk->code[offset] = (jump >> 8) & 0xff;
    chunk->code[offset + 1] = jump & 0xff;
}

static void emit_loop(Codegen* codegen, int loop_start, int line) {
    emit_op(codegen, OP_LOOP, line);

    int offset = current_chunk(codegen)->count - loop_start + 2;
    if (offset > UINT16_MAX) {
        error_at(codegen, (Span){0, 0, 0, 0}, "Loop body too large");
    }

    emit_byte(codegen, (offset >> 8) & 0xff, line);
    emit_byte(codegen, offset & 0xff, line);
}

static uint8_t make_constant(Codegen* codegen, Value value, int line) {
    int index = chunk_add_constant(current_chunk(codegen), value);
    if (index > UINT8_MAX) {
        error_at(codegen, (Span){0, 0, 0, 0}, "Too many constants in one chunk");
        return 0;
    }
    return (uint8_t)index;
}

static void emit_constant(Codegen* codegen, Value value, int line) {
    uint8_t index = make_constant(codegen, value, line);
    emit_bytes(codegen, OP_CONSTANT, index, line);
}

static void emit_return(Codegen* codegen, int line) {
    emit_op(codegen, OP_NONE, line);
    emit_op(codegen, OP_RETURN, line);
}

// ============================================================================
// Scope Management
// ============================================================================

static void begin_scope(Codegen* codegen) {
    codegen->current->scope_depth++;
}

static void end_scope(Codegen* codegen, int line) {
    Compiler* compiler = codegen->current;
    compiler->scope_depth--;

    // Pop locals that are going out of scope
    while (compiler->local_count > 0 &&
           compiler->locals[compiler->local_count - 1].depth > compiler->scope_depth) {
        Local* local = &compiler->locals[compiler->local_count - 1];
        if (local->is_captured) {
            emit_op(codegen, OP_CLOSE_UPVALUE, line);
        } else {
            emit_op(codegen, OP_POP, line);
        }
        compiler->local_count--;
    }
}

static void add_local(Codegen* codegen, const char* name, int length) {
    Compiler* compiler = codegen->current;
    if (compiler->local_count >= UINT8_COUNT) {
        error_at(codegen, (Span){0, 0, 0, 0},
                 "Too many local variables in function");
        return;
    }

    Local* local = &compiler->locals[compiler->local_count++];
    local->name = name;
    local->length = length;
    local->depth = -1;  // Mark as uninitialized
    local->is_captured = false;
}

static void mark_initialized(Codegen* codegen) {
    if (codegen->current->scope_depth == 0) return;
    codegen->current->locals[codegen->current->local_count - 1].depth =
        codegen->current->scope_depth;
}

static int resolve_local(Codegen* codegen, Compiler* compiler,
                         const char* name, int length) {
    for (int i = compiler->local_count - 1; i >= 0; i--) {
        Local* local = &compiler->locals[i];
        if (local->length == length &&
            memcmp(local->name, name, length) == 0) {
            if (local->depth == -1) {
                error_at(codegen, (Span){0, 0, 0, 0},
                         "Cannot read variable in its own initializer");
            }
            return i;
        }
    }
    return -1;
}

static int add_upvalue(Codegen* codegen, Compiler* compiler,
                       uint8_t index, bool is_local) {
    int upvalue_count = compiler->function->upvalue_count;

    // Check if we already have this upvalue
    for (int i = 0; i < upvalue_count; i++) {
        Upvalue* upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->is_local == is_local) {
            return i;
        }
    }

    if (upvalue_count >= MAX_UPVALUES) {
        error_at(codegen, (Span){0, 0, 0, 0}, "Too many closure variables");
        return 0;
    }

    compiler->upvalues[upvalue_count].is_local = is_local;
    compiler->upvalues[upvalue_count].index = index;
    return compiler->function->upvalue_count++;
}

static int resolve_upvalue(Codegen* codegen, Compiler* compiler,
                           const char* name, int length) {
    if (compiler->enclosing == NULL) return -1;

    // Look for local in enclosing function
    int local = resolve_local(codegen, compiler->enclosing, name, length);
    if (local != -1) {
        compiler->enclosing->locals[local].is_captured = true;
        return add_upvalue(codegen, compiler, (uint8_t)local, true);
    }

    // Look for upvalue in enclosing function
    int upvalue = resolve_upvalue(codegen, compiler->enclosing, name, length);
    if (upvalue != -1) {
        return add_upvalue(codegen, compiler, (uint8_t)upvalue, false);
    }

    return -1;
}

static uint8_t identifier_constant(Codegen* codegen, const char* name, int length) {
    ObjString* str = string_intern(name, length);
    return make_constant(codegen, OBJECT_VAL(str), 0);
}

static void declare_variable(Codegen* codegen, const char* name, int length, Span span) {
    if (codegen->current->scope_depth == 0) return;  // Global

    // Check for redeclaration in same scope
    Compiler* compiler = codegen->current;
    for (int i = compiler->local_count - 1; i >= 0; i--) {
        Local* local = &compiler->locals[i];
        if (local->depth != -1 && local->depth < compiler->scope_depth) {
            break;
        }
        if (local->length == length &&
            memcmp(local->name, name, length) == 0) {
            error_at(codegen, span,
                     "Variable '%.*s' already declared in this scope", length, name);
            return;
        }
    }

    add_local(codegen, name, length);
}

static void define_variable(Codegen* codegen, uint8_t global, int line) {
    if (codegen->current->scope_depth > 0) {
        mark_initialized(codegen);
        return;
    }
    emit_bytes(codegen, OP_SET_GLOBAL, global, line);
    emit_op(codegen, OP_POP, line);
}

static void named_variable(Codegen* codegen, const char* name, int length,
                           bool can_assign, int line) {
    uint8_t get_op, set_op;
    int arg = resolve_local(codegen, codegen->current, name, length);

    if (arg != -1) {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    } else if ((arg = resolve_upvalue(codegen, codegen->current, name, length)) != -1) {
        get_op = OP_GET_UPVALUE;
        set_op = OP_SET_UPVALUE;
    } else {
        arg = identifier_constant(codegen, name, length);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }

    PH_UNUSED(can_assign);
    PH_UNUSED(set_op);
    emit_bytes(codegen, get_op, (uint8_t)arg, line);
}

// ============================================================================
// Compiler State Management
// ============================================================================

static void init_compiler(Codegen* codegen, Compiler* compiler, FunctionType type) {
    compiler->enclosing = codegen->current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->local_count = 0;
    compiler->scope_depth = 0;
    compiler->loop_start = -1;
    compiler->loop_depth = 0;
    compiler->break_jumps = NULL;
    compiler->break_count = 0;
    compiler->break_capacity = 0;
    compiler->function = function_new();

    // Allocate the chunk for this function
    compiler->function->chunk = PH_ALLOC(sizeof(Chunk));
    chunk_init(compiler->function->chunk);

    codegen->current = compiler;

    // Reserve slot 0: for methods it's 'this', for functions it's the function itself
    Local* local = &compiler->locals[compiler->local_count++];
    local->depth = 0;
    local->is_captured = false;
    if (type == TYPE_METHOD || type == TYPE_INITIALIZER) {
        local->name = "this";
        local->length = 4;
    } else {
        local->name = "";
        local->length = 0;
    }
}

static ObjFunction* end_compiler(Codegen* codegen, int line) {
    emit_return(codegen, line);
    ObjFunction* function = codegen->current->function;

    // Clean up break jump array
    if (codegen->current->break_jumps) {
        PH_FREE(codegen->current->break_jumps);
    }

#ifdef DEBUG_PRINT_CODE
    if (!codegen->had_error) {
        disassemble_chunk(current_chunk(codegen),
                          function->name != NULL ? function->name->chars : "<script>");
    }
#endif

    codegen->current = codegen->current->enclosing;
    return function;
}

// ============================================================================
// Expression Compilation
// ============================================================================

static void compile_literal_null(Codegen* codegen, ExprLiteralNull* expr) {
    emit_op(codegen, OP_NONE, expr->base.span.start_line);
}

static void compile_literal_bool(Codegen* codegen, ExprLiteralBool* expr) {
    emit_op(codegen, expr->value ? OP_TRUE : OP_FALSE, expr->base.span.start_line);
}

static void compile_literal_number(Codegen* codegen, ExprLiteralNumber* expr) {
    emit_constant(codegen, NUMBER_VAL(expr->value), expr->base.span.start_line);
}

static void compile_literal_string(Codegen* codegen, ExprLiteralString* expr) {
    ObjString* str = string_intern(expr->value, expr->length);
    emit_constant(codegen, OBJECT_VAL(str), expr->base.span.start_line);
}

static void compile_identifier(Codegen* codegen, ExprIdentifier* expr) {
    named_variable(codegen, expr->name.start, expr->name.length, false,
                   expr->base.span.start_line);
}

static void compile_unary(Codegen* codegen, ExprUnary* expr) {
    int line = expr->base.span.start_line;

    // Compile operand first
    compile_expr(codegen, expr->operand);

    // Emit operator
    switch (expr->operator) {
        case TOKEN_MINUS:
            emit_op(codegen, OP_NEGATE, line);
            break;
        case TOKEN_NOT:
        case TOKEN_BANG:
            emit_op(codegen, OP_NOT, line);
            break;
        default:
            error_at(codegen, expr->base.span, "Unknown unary operator");
    }
}

static void compile_binary(Codegen* codegen, ExprBinary* expr) {
    int line = expr->base.span.start_line;

    // Short-circuit logic operators
    if (expr->operator == TOKEN_AND) {
        compile_expr(codegen, expr->left);
        int end_jump = emit_jump(codegen, OP_JUMP_IF_FALSE, line);
        emit_op(codegen, OP_POP, line);
        compile_expr(codegen, expr->right);
        patch_jump(codegen, end_jump);
        return;
    }

    if (expr->operator == TOKEN_OR) {
        compile_expr(codegen, expr->left);
        int else_jump = emit_jump(codegen, OP_JUMP_IF_FALSE, line);
        int end_jump = emit_jump(codegen, OP_JUMP, line);
        patch_jump(codegen, else_jump);
        emit_op(codegen, OP_POP, line);
        compile_expr(codegen, expr->right);
        patch_jump(codegen, end_jump);
        return;
    }

    // Compile both operands
    compile_expr(codegen, expr->left);
    compile_expr(codegen, expr->right);

    // Emit operator
    switch (expr->operator) {
        case TOKEN_PLUS:
            emit_op(codegen, OP_ADD, line);
            break;
        case TOKEN_MINUS:
            emit_op(codegen, OP_SUBTRACT, line);
            break;
        case TOKEN_STAR:
            emit_op(codegen, OP_MULTIPLY, line);
            break;
        case TOKEN_SLASH:
            emit_op(codegen, OP_DIVIDE, line);
            break;
        case TOKEN_PERCENT:
            emit_op(codegen, OP_MODULO, line);
            break;
        case TOKEN_EQUAL_EQUAL:
            emit_op(codegen, OP_EQUAL, line);
            break;
        case TOKEN_BANG_EQUAL:
            emit_op(codegen, OP_NOT_EQUAL, line);
            break;
        case TOKEN_LESS:
            emit_op(codegen, OP_LESS, line);
            break;
        case TOKEN_LESS_EQUAL:
            emit_op(codegen, OP_LESS_EQUAL, line);
            break;
        case TOKEN_GREATER:
            emit_op(codegen, OP_GREATER, line);
            break;
        case TOKEN_GREATER_EQUAL:
            emit_op(codegen, OP_GREATER_EQUAL, line);
            break;
        default:
            error_at(codegen, expr->base.span, "Unknown binary operator");
    }
}

static void compile_call(Codegen* codegen, ExprCall* expr) {
    int line = expr->base.span.start_line;

    // Check for method invocation optimization
    if (expr->callee->type == EXPR_GET) {
        ExprGet* get = (ExprGet*)expr->callee;

        // Compile the object
        compile_expr(codegen, get->object);

        // Compile arguments
        for (int i = 0; i < expr->arg_count; i++) {
            compile_expr(codegen, expr->arguments[i]);
        }

        // Emit invoke with method name
        uint8_t name = identifier_constant(codegen, get->name.start, get->name.length);
        emit_bytes(codegen, OP_INVOKE, name, line);
        emit_byte(codegen, (uint8_t)expr->arg_count, line);
        return;
    }

    // Compile callee
    compile_expr(codegen, expr->callee);

    // Compile arguments
    if (expr->arg_count > 255) {
        error_at(codegen, expr->base.span, "Cannot have more than 255 arguments");
        return;
    }

    for (int i = 0; i < expr->arg_count; i++) {
        compile_expr(codegen, expr->arguments[i]);
    }

    emit_bytes(codegen, OP_CALL, (uint8_t)expr->arg_count, line);
}

static void compile_get(Codegen* codegen, ExprGet* expr) {
    int line = expr->base.span.start_line;
    compile_expr(codegen, expr->object);
    uint8_t name = identifier_constant(codegen, expr->name.start, expr->name.length);
    emit_bytes(codegen, OP_GET_PROPERTY, name, line);
}

static void compile_index(Codegen* codegen, ExprIndex* expr) {
    int line = expr->base.span.start_line;
    compile_expr(codegen, expr->object);
    compile_expr(codegen, expr->index);
    emit_op(codegen, OP_INDEX_GET, line);
}

static void compile_list(Codegen* codegen, ExprList* expr) {
    int line = expr->base.span.start_line;

    if (expr->count > 255) {
        error_at(codegen, expr->base.span, "Cannot have more than 255 list elements");
        return;
    }

    // Compile all elements
    for (int i = 0; i < expr->count; i++) {
        compile_expr(codegen, expr->elements[i]);
    }

    emit_bytes(codegen, OP_LIST, (uint8_t)expr->count, line);
}

static void compile_function_expr(Codegen* codegen, ExprFunction* expr);

static void compile_expr(Codegen* codegen, Expr* expr) {
    switch (expr->type) {
        case EXPR_LITERAL_NULL:
            compile_literal_null(codegen, (ExprLiteralNull*)expr);
            break;
        case EXPR_LITERAL_BOOL:
            compile_literal_bool(codegen, (ExprLiteralBool*)expr);
            break;
        case EXPR_LITERAL_NUMBER:
            compile_literal_number(codegen, (ExprLiteralNumber*)expr);
            break;
        case EXPR_LITERAL_STRING:
            compile_literal_string(codegen, (ExprLiteralString*)expr);
            break;
        case EXPR_IDENTIFIER:
            compile_identifier(codegen, (ExprIdentifier*)expr);
            break;
        case EXPR_UNARY:
            compile_unary(codegen, (ExprUnary*)expr);
            break;
        case EXPR_BINARY:
            compile_binary(codegen, (ExprBinary*)expr);
            break;
        case EXPR_CALL:
            compile_call(codegen, (ExprCall*)expr);
            break;
        case EXPR_GET:
            compile_get(codegen, (ExprGet*)expr);
            break;
        case EXPR_SET: {
            // obj.field = value
            ExprSet* set = (ExprSet*)expr;
            int line = set->base.span.start_line;
            compile_expr(codegen, set->object);
            compile_expr(codegen, set->value);
            uint8_t name = identifier_constant(codegen, set->name.start, set->name.length);
            emit_bytes(codegen, OP_SET_PROPERTY, name, line);
            break;
        }
        case EXPR_INDEX:
            compile_index(codegen, (ExprIndex*)expr);
            break;
        case EXPR_INDEX_SET: {
            // obj[idx] = value
            ExprIndexSet* set = (ExprIndexSet*)expr;
            int line = set->base.span.start_line;
            compile_expr(codegen, set->object);
            compile_expr(codegen, set->index);
            compile_expr(codegen, set->value);
            emit_op(codegen, OP_INDEX_SET, line);
            break;
        }
        case EXPR_LIST:
            compile_list(codegen, (ExprList*)expr);
            break;
        case EXPR_FUNCTION:
            compile_function_expr(codegen, (ExprFunction*)expr);
            break;
        case EXPR_VEC2: {
            // Compile as a call to vec2 constructor
            ExprVec2* v = (ExprVec2*)expr;
            int line = v->base.span.start_line;
            ObjString* name = string_intern("vec2", 4);
            uint8_t arg = make_constant(codegen, OBJECT_VAL(name), line);
            emit_bytes(codegen, OP_GET_GLOBAL, arg, line);
            compile_expr(codegen, v->x);
            compile_expr(codegen, v->y);
            emit_bytes(codegen, OP_CALL, 2, line);
            break;
        }
        case EXPR_POSTFIX: {
            ExprPostfix* post = (ExprPostfix*)expr;
            int line = post->base.span.start_line;
            bool is_increment = post->op.type == TOKEN_PLUS_PLUS;

            // Handle different operand types
            switch (post->operand->type) {
                case EXPR_IDENTIFIER: {
                    ExprIdentifier* id = (ExprIdentifier*)post->operand;
                    const char* name = id->name.start;
                    int length = id->name.length;

                    // Get current value
                    int arg = resolve_local(codegen, codegen->current, name, length);
                    if (arg != -1) {
                        emit_bytes(codegen, OP_GET_LOCAL, (uint8_t)arg, line);
                    } else if ((arg = resolve_upvalue(codegen, codegen->current, name, length)) != -1) {
                        emit_bytes(codegen, OP_GET_UPVALUE, (uint8_t)arg, line);
                    } else {
                        arg = identifier_constant(codegen, name, length);
                        emit_bytes(codegen, OP_GET_GLOBAL, (uint8_t)arg, line);
                    }

                    // Stack: [old_value]
                    emit_op(codegen, OP_DUP, line);
                    // Stack: [old_value, old_value]
                    emit_constant(codegen, NUMBER_VAL(1), line);
                    // Stack: [old_value, old_value, 1]
                    emit_op(codegen, is_increment ? OP_ADD : OP_SUBTRACT, line);
                    // Stack: [old_value, new_value]

                    // Store new value back
                    arg = resolve_local(codegen, codegen->current, name, length);
                    if (arg != -1) {
                        emit_bytes(codegen, OP_SET_LOCAL, (uint8_t)arg, line);
                    } else if ((arg = resolve_upvalue(codegen, codegen->current, name, length)) != -1) {
                        emit_bytes(codegen, OP_SET_UPVALUE, (uint8_t)arg, line);
                    } else {
                        arg = identifier_constant(codegen, name, length);
                        emit_bytes(codegen, OP_SET_GLOBAL, (uint8_t)arg, line);
                    }
                    // Stack: [old_value, new_value] (set peeks, doesn't pop)
                    emit_op(codegen, OP_POP, line);
                    // Stack: [old_value]
                    break;
                }
                default:
                    error_at(codegen, post->operand->span,
                             "Increment/decrement requires a variable");
            }
            break;
        }
        default:
            error_at(codegen, expr->span, "Unknown expression type");
    }
}

// ============================================================================
// Statement Compilation
// ============================================================================

static void compile_expression_stmt(Codegen* codegen, StmtExpression* stmt) {
    compile_expr(codegen, stmt->expression);
    emit_op(codegen, OP_POP, stmt->base.span.start_line);
}

static void compile_assignment_stmt(Codegen* codegen, StmtAssignment* stmt) {
    int line = stmt->base.span.start_line;

    // Handle different assignment targets
    switch (stmt->target->type) {
        case EXPR_IDENTIFIER: {
            // For identifiers: compile value, then set
            compile_expr(codegen, stmt->value);

            ExprIdentifier* id = (ExprIdentifier*)stmt->target;
            const char* name = id->name.start;
            int length = id->name.length;

            int arg = resolve_local(codegen, codegen->current, name, length);
            if (arg != -1) {
                emit_bytes(codegen, OP_SET_LOCAL, (uint8_t)arg, line);
            } else if ((arg = resolve_upvalue(codegen, codegen->current, name, length)) != -1) {
                emit_bytes(codegen, OP_SET_UPVALUE, (uint8_t)arg, line);
            } else {
                // New global or existing global
                arg = identifier_constant(codegen, name, length);
                emit_bytes(codegen, OP_SET_GLOBAL, (uint8_t)arg, line);
            }
            emit_op(codegen, OP_POP, line);
            break;
        }
        case EXPR_GET: {
            // For property set: compile object, compile value, then SET_PROPERTY
            // Stack: [object, value]
            ExprGet* get = (ExprGet*)stmt->target;
            compile_expr(codegen, get->object);
            compile_expr(codegen, stmt->value);
            uint8_t name = identifier_constant(codegen, get->name.start, get->name.length);
            emit_bytes(codegen, OP_SET_PROPERTY, name, line);
            emit_op(codegen, OP_POP, line);
            break;
        }
        case EXPR_INDEX: {
            // For index set: compile object, compile index, compile value, then INDEX_SET
            // Stack: [object, index, value]
            ExprIndex* idx = (ExprIndex*)stmt->target;
            compile_expr(codegen, idx->object);
            compile_expr(codegen, idx->index);
            compile_expr(codegen, stmt->value);
            emit_op(codegen, OP_INDEX_SET, line);
            emit_op(codegen, OP_POP, line);
            break;
        }
        default:
            error_at(codegen, stmt->target->span, "Invalid assignment target");
    }
}

static void compile_block_stmt(Codegen* codegen, StmtBlock* stmt) {
    begin_scope(codegen);
    for (int i = 0; i < stmt->count; i++) {
        compile_stmt(codegen, stmt->statements[i]);
    }
    end_scope(codegen, stmt->base.span.end_line);
}

static void compile_if_stmt(Codegen* codegen, StmtIf* stmt) {
    int line = stmt->base.span.start_line;

    // Compile condition
    compile_expr(codegen, stmt->condition);

    // Jump past then branch if false
    int then_jump = emit_jump(codegen, OP_JUMP_IF_FALSE, line);
    emit_op(codegen, OP_POP, line);  // Pop condition

    // Compile then branch
    compile_stmt(codegen, stmt->then_branch);

    // Jump past else branch
    int else_jump = emit_jump(codegen, OP_JUMP, line);

    // Patch the then jump
    patch_jump(codegen, then_jump);
    emit_op(codegen, OP_POP, line);  // Pop condition

    // Compile else branch if present
    if (stmt->else_branch) {
        compile_stmt(codegen, stmt->else_branch);
    }

    patch_jump(codegen, else_jump);
}

static void compile_while_stmt(Codegen* codegen, StmtWhile* stmt) {
    int line = stmt->base.span.start_line;

    // Save previous loop state
    int prev_loop_start = codegen->current->loop_start;
    int prev_loop_depth = codegen->current->loop_depth;
    int* prev_break_jumps = codegen->current->break_jumps;
    int prev_break_count = codegen->current->break_count;
    int prev_break_capacity = codegen->current->break_capacity;

    // Set up new loop
    int loop_start = current_chunk(codegen)->count;
    codegen->current->loop_start = loop_start;
    codegen->current->loop_depth = codegen->current->scope_depth;
    codegen->current->break_jumps = NULL;
    codegen->current->break_count = 0;
    codegen->current->break_capacity = 0;

    // Compile condition
    compile_expr(codegen, stmt->condition);

    // Jump out if false
    int exit_jump = emit_jump(codegen, OP_JUMP_IF_FALSE, line);
    emit_op(codegen, OP_POP, line);  // Pop condition

    // Compile body
    compile_stmt(codegen, stmt->body);

    // Loop back
    emit_loop(codegen, loop_start, line);

    // Patch exit jump
    patch_jump(codegen, exit_jump);
    emit_op(codegen, OP_POP, line);  // Pop condition

    // Patch all break jumps
    for (int i = 0; i < codegen->current->break_count; i++) {
        patch_jump(codegen, codegen->current->break_jumps[i]);
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
