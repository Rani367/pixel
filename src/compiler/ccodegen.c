#include "compiler/ccodegen.h"
#include <string.h>
#include <stdarg.h>

// ============================================================================
// Output Helpers
// ============================================================================

__attribute__((format(printf, 2, 3)))
static void emit(CCodegen* gen, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[4096];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    sb_append(&gen->output, buf);
}

static void emit_indent(CCodegen* gen) {
    for (int i = 0; i < gen->indent_level; i++) {
        sb_append(&gen->output, "    ");
    }
}

__attribute__((format(printf, 2, 3)))
static void emit_line(CCodegen* gen, const char* fmt, ...) {
    emit_indent(gen);
    va_list args;
    va_start(args, fmt);
    char buf[4096];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    sb_append(&gen->output, buf);
    sb_append(&gen->output, "\n");
}

static void emit_newline(CCodegen* gen) {
    sb_append(&gen->output, "\n");
}

// ============================================================================
// Initialization
// ============================================================================

void ccodegen_init(CCodegen* gen, Arena* arena, TypeChecker* tc, const char* filename) {
    gen->arena = arena;
    gen->typechecker = tc;
    gen->indent_level = 0;
    gen->temp_counter = 0;
    gen->closure_counter = 0;
    gen->filename = filename;
    gen->had_error = false;
    sb_init(&gen->output);
}

void ccodegen_free(CCodegen* gen) {
    sb_free(&gen->output);
}

const char* ccodegen_temp_var(CCodegen* gen) {
    char* buf = arena_alloc(gen->arena, 32);
    snprintf(buf, 32, "_tmp%d", gen->temp_counter++);
    return buf;
}

// ============================================================================
// Type to C Conversion
// ============================================================================

const char* ccodegen_type_to_c(CCodegen* gen, Type* type) {
    if (!type) return "void";

    switch (type->kind) {
        case TY_NUM:   return "double";
        case TY_INT:   return "int32_t";
        case TY_STR:   return "PxString*";
        case TY_BOOL:  return "bool";
        case TY_NONE:  return "void";
        case TY_ANY:   return "PxValue";
        case TY_ERROR: return "void";

        case TY_LIST: {
            // Generate specialized list type name
            char* buf = arena_alloc(gen->arena, 64);
            if (type->as.list.element->kind == TY_NUM) {
                snprintf(buf, 64, "PxList_num*");
            } else if (type->as.list.element->kind == TY_INT) {
                snprintf(buf, 64, "PxList_int*");
            } else if (type->as.list.element->kind == TY_STR) {
                snprintf(buf, 64, "PxList_str*");
            } else {
                snprintf(buf, 64, "PxList*");
            }
            return buf;
        }

        case TY_STRUCT: {
            char* buf = arena_alloc(gen->arena, type->as.struct_type.name_length + 8);
            snprintf(buf, type->as.struct_type.name_length + 8, "Px%.*s*",
                     type->as.struct_type.name_length, type->as.struct_type.name);
            return buf;
        }

        case TY_FUNC:
            // Function pointers are more complex, return generic for now
            return "PxClosure*";

        default:
            return "void";
    }
}

// ============================================================================
// Forward Declarations
// ============================================================================

static void gen_expr(CCodegen* gen, Expr* expr);
static void gen_stmt(CCodegen* gen, Stmt* stmt);

// ============================================================================
// Expression Code Generation
// ============================================================================

static void gen_expr(CCodegen* gen, Expr* expr) {
    if (!expr) {
        emit(gen, "NULL");
        return;
    }

    switch (expr->type) {
        case EXPR_LITERAL_NULL:
            emit(gen, "PX_NONE");
            break;

        case EXPR_LITERAL_BOOL: {
            ExprLiteralBool* b = (ExprLiteralBool*)expr;
            emit(gen, b->value ? "true" : "false");
            break;
        }

        case EXPR_LITERAL_NUMBER: {
            ExprLiteralNumber* num = (ExprLiteralNumber*)expr;
            emit(gen, "%g", num->value);
            break;
        }

        case EXPR_LITERAL_STRING: {
            ExprLiteralString* str = (ExprLiteralString*)expr;
            emit(gen, "px_string_new(\"");
            // Escape string contents
            for (int i = 0; i < str->length; i++) {
                char c = str->value[i];
                switch (c) {
                    case '\n': emit(gen, "\\n"); break;
                    case '\r': emit(gen, "\\r"); break;
                    case '\t': emit(gen, "\\t"); break;
                    case '\\': emit(gen, "\\\\"); break;
                    case '"':  emit(gen, "\\\""); break;
                    default:   emit(gen, "%c", c); break;
                }
            }
            emit(gen, "\", %d)", str->length);
            break;
        }

        case EXPR_IDENTIFIER: {
            ExprIdentifier* ident = (ExprIdentifier*)expr;
            emit(gen, "%.*s", ident->name.length, ident->name.start);
            break;
        }

        case EXPR_UNARY: {
            ExprUnary* unary = (ExprUnary*)expr;
            switch (unary->operator) {
                case TOKEN_MINUS:
                    emit(gen, "(-");
                    gen_expr(gen, unary->operand);
                    emit(gen, ")");
                    break;
                case TOKEN_NOT:
                    emit(gen, "(!");
                    gen_expr(gen, unary->operand);
                    emit(gen, ")");
                    break;
                default:
                    gen_expr(gen, unary->operand);
                    break;
            }
            break;
        }

        case EXPR_BINARY: {
            ExprBinary* binary = (ExprBinary*)expr;

            // Handle string concatenation specially
            if (binary->operator == TOKEN_PLUS) {
                Type* left_type = typechecker_get_expr_type(gen->typechecker, binary->left);
                if (left_type && left_type->kind == TY_STR) {
                    emit(gen, "px_string_concat(");
                    gen_expr(gen, binary->left);
                    emit(gen, ", ");
                    gen_expr(gen, binary->right);
                    emit(gen, ")");
                    break;
                }
            }

            emit(gen, "(");
            gen_expr(gen, binary->left);

            switch (binary->operator) {
                case TOKEN_PLUS:
                    emit(gen, " + ");
                    break;
                case TOKEN_MINUS:       emit(gen, " - "); break;
                case TOKEN_STAR:        emit(gen, " * "); break;
                case TOKEN_SLASH:       emit(gen, " / "); break;
                case TOKEN_PERCENT:     emit(gen, " %% "); break;
                case TOKEN_LESS:        emit(gen, " < "); break;
                case TOKEN_LESS_EQUAL:  emit(gen, " <= "); break;
                case TOKEN_GREATER:     emit(gen, " > "); break;
                case TOKEN_GREATER_EQUAL: emit(gen, " >= "); break;
                case TOKEN_EQUAL_EQUAL: emit(gen, " == "); break;
                case TOKEN_BANG_EQUAL:  emit(gen, " != "); break;
                case TOKEN_AND:         emit(gen, " && "); break;
                case TOKEN_OR:          emit(gen, " || "); break;
                default:                emit(gen, " ? "); break;
            }

            gen_expr(gen, binary->right);
            emit(gen, ")");
            break;
        }

        case EXPR_CALL: {
            ExprCall* call = (ExprCall*)expr;
            // If callee is an identifier, prefix with px_
            if (call->callee->type == EXPR_IDENTIFIER) {
                ExprIdentifier* ident = (ExprIdentifier*)call->callee;

                // Special handling for print/println - convert argument to string
                bool is_print = (ident->name.length == 5 && memcmp(ident->name.start, "print", 5) == 0) ||
                               (ident->name.length == 7 && memcmp(ident->name.start, "println", 7) == 0);

                emit(gen, "px_%.*s(", ident->name.length, ident->name.start);

                if (is_print && call->arg_count == 1) {
                    // Convert non-string argument to string for print
                    Type* arg_type = typechecker_get_expr_type(gen->typechecker, call->arguments[0]);
                    if (arg_type && arg_type->kind == TY_NUM) {
                        emit(gen, "px_string_from_num(");
                        gen_expr(gen, call->arguments[0]);
                        emit(gen, ")");
                    } else if (arg_type && arg_type->kind == TY_INT) {
                        emit(gen, "px_string_from_int(");
                        gen_expr(gen, call->arguments[0]);
                        emit(gen, ")");
                    } else if (arg_type && arg_type->kind == TY_BOOL) {
                        emit(gen, "(");
                        gen_expr(gen, call->arguments[0]);
                        emit(gen, " ? px_string_new(\"true\", 4) : px_string_new(\"false\", 5))");
                    } else {
                        gen_expr(gen, call->arguments[0]);
                    }
                } else {
                    for (int i = 0; i < call->arg_count; i++) {
                        if (i > 0) emit(gen, ", ");
                        gen_expr(gen, call->arguments[i]);
                    }
                }
            } else {
                gen_expr(gen, call->callee);
                emit(gen, "(");
                for (int i = 0; i < call->arg_count; i++) {
                    if (i > 0) emit(gen, ", ");
                    gen_expr(gen, call->arguments[i]);
                }
            }
            emit(gen, ")");
            break;
        }

        case EXPR_GET: {
            ExprGet* get = (ExprGet*)expr;
            gen_expr(gen, get->object);
            emit(gen, "->%.*s", get->name.length, get->name.start);
            break;
        }

        case EXPR_SET: {
            ExprSet* set = (ExprSet*)expr;
            emit(gen, "(");
            gen_expr(gen, set->object);
            emit(gen, "->%.*s = ", set->name.length, set->name.start);
            gen_expr(gen, set->value);
            emit(gen, ")");
            break;
        }

        case EXPR_INDEX: {
            ExprIndex* index = (ExprIndex*)expr;
            Type* obj_type = typechecker_get_expr_type(gen->typechecker, index->object);
            if (obj_type && obj_type->kind == TY_LIST) {
                emit(gen, "PxList_get(");
                gen_expr(gen, index->object);
                emit(gen, ", ");
                gen_expr(gen, index->index);
                emit(gen, ")");
            } else {
                gen_expr(gen, index->object);
                emit(gen, "[(int)");
                gen_expr(gen, index->index);
                emit(gen, "]");
            }
            break;
        }

        case EXPR_INDEX_SET: {
            ExprIndexSet* set = (ExprIndexSet*)expr;
            Type* obj_type = typechecker_get_expr_type(gen->typechecker, set->object);
            if (obj_type && obj_type->kind == TY_LIST) {
                emit(gen, "PxList_set(");
                gen_expr(gen, set->object);
                emit(gen, ", ");
                gen_expr(gen, set->index);
                emit(gen, ", ");
                gen_expr(gen, set->value);
                emit(gen, ")");
            } else {
                gen_expr(gen, set->object);
                emit(gen, "[(int)");
                gen_expr(gen, set->index);
                emit(gen, "] = ");
                gen_expr(gen, set->value);
            }
            break;
        }

        case EXPR_LIST: {
            ExprList* list = (ExprList*)expr;
            Type* list_type = typechecker_get_expr_type(gen->typechecker, expr);
            const char* c_type = "PxList_any";
            if (list_type && list_type->kind == TY_LIST) {
                if (list_type->as.list.element->kind == TY_NUM) {
                    c_type = "PxList_num";
                } else if (list_type->as.list.element->kind == TY_INT) {
                    c_type = "PxList_int";
                } else if (list_type->as.list.element->kind == TY_STR) {
                    c_type = "PxList_str";
                }
            }
            emit(gen, "%s_from(%d", c_type, list->count);
            for (int i = 0; i < list->count; i++) {
                emit(gen, ", ");
                gen_expr(gen, list->elements[i]);
            }
            emit(gen, ")");
            break;
        }

        case EXPR_FUNCTION: {
            // Anonymous function - generate closure
            emit(gen, "/* anonymous function not yet supported in AOT */ NULL");
            break;
        }

        case EXPR_VEC2: {
            ExprVec2* vec = (ExprVec2*)expr;
            emit(gen, "px_vec2(");
            gen_expr(gen, vec->x);
            emit(gen, ", ");
            gen_expr(gen, vec->y);
            emit(gen, ")");
            break;
        }

        case EXPR_POSTFIX: {
            ExprPostfix* post = (ExprPostfix*)expr;
            gen_expr(gen, post->operand);
            emit(gen, post->op.type == TOKEN_PLUS_PLUS ? "++" : "--");
            break;
        }

        default:
            emit(gen, "/* unknown expr */");
            break;
    }
}

// ============================================================================
// Statement Code Generation
// ============================================================================

static void gen_stmt(CCodegen* gen, Stmt* stmt) {
    if (!stmt) return;

    switch (stmt->type) {
        case STMT_EXPRESSION: {
            StmtExpression* expr_stmt = (StmtExpression*)stmt;
            emit_indent(gen);
            gen_expr(gen, expr_stmt->expression);
            emit(gen, ";\n");
            break;
        }

        case STMT_ASSIGNMENT: {
            StmtAssignment* assign = (StmtAssignment*)stmt;
            emit_indent(gen);

            // Check if this is a new variable declaration
            if (assign->target->type == EXPR_IDENTIFIER) {
                ExprIdentifier* ident = (ExprIdentifier*)assign->target;
                Type* type = typechecker_lookup(gen->typechecker,
                                                ident->name.start, ident->name.length);
                if (type) {
                    // Variable already declared, just assign
                    gen_expr(gen, assign->target);
                    emit(gen, " = ");
                    gen_expr(gen, assign->value);
                    emit(gen, ";\n");
                } else {
                    // New variable - infer type from value
                    Type* val_type = typechecker_get_expr_type(gen->typechecker, assign->value);
                    const char* c_type = ccodegen_type_to_c(gen, val_type);
                    emit(gen, "%s ", c_type);
                    gen_expr(gen, assign->target);
                    emit(gen, " = ");
                    gen_expr(gen, assign->value);
                    emit(gen, ";\n");
                }
            } else {
                gen_expr(gen, assign->target);
                emit(gen, " = ");
                gen_expr(gen, assign->value);
                emit(gen, ";\n");
            }
            break;
        }

        case STMT_VAR_DECL: {
            StmtVarDecl* decl = (StmtVarDecl*)stmt;
            Type* type = typechecker_resolve_type_expr(gen->typechecker, decl->type);
            const char* c_type = ccodegen_type_to_c(gen, type);
            emit_indent(gen);
            emit(gen, "%s %.*s = ", c_type, decl->name.length, decl->name.start);
            gen_expr(gen, decl->initializer);
            emit(gen, ";\n");
            break;
        }

        case STMT_BLOCK: {
            StmtBlock* block = (StmtBlock*)stmt;
            emit_line(gen, "{");
            gen->indent_level++;
            for (int i = 0; i < block->count; i++) {
                gen_stmt(gen, block->statements[i]);
            }
            gen->indent_level--;
            emit_line(gen, "}");
            break;
        }

        case STMT_IF: {
            StmtIf* if_stmt = (StmtIf*)stmt;
            emit_indent(gen);
            emit(gen, "if (");
            gen_expr(gen, if_stmt->condition);
            emit(gen, ") ");

            if (if_stmt->then_branch->type == STMT_BLOCK) {
                gen_stmt(gen, if_stmt->then_branch);
            } else {
                emit(gen, "{\n");
                gen->indent_level++;
                gen_stmt(gen, if_stmt->then_branch);
                gen->indent_level--;
                emit_line(gen, "}");
            }

            if (if_stmt->else_branch) {
                emit_indent(gen);
                emit(gen, "else ");
                if (if_stmt->else_branch->type == STMT_BLOCK ||
                    if_stmt->else_branch->type == STMT_IF) {
                    gen_stmt(gen, if_stmt->else_branch);
                } else {
                    emit(gen, "{\n");
                    gen->indent_level++;
                    gen_stmt(gen, if_stmt->else_branch);
                    gen->indent_level--;
                    emit_line(gen, "}");
                }
            }
            break;
        }

        case STMT_WHILE: {
            StmtWhile* while_stmt = (StmtWhile*)stmt;
            emit_indent(gen);
            emit(gen, "while (");
            gen_expr(gen, while_stmt->condition);
            emit(gen, ") ");

            if (while_stmt->body->type == STMT_BLOCK) {
                gen_stmt(gen, while_stmt->body);
            } else {
                emit(gen, "{\n");
                gen->indent_level++;
                gen_stmt(gen, while_stmt->body);
                gen->indent_level--;
                emit_line(gen, "}");
            }
            break;
        }

        case STMT_FOR: {
            StmtFor* for_stmt = (StmtFor*)stmt;
            Type* iter_type = typechecker_get_expr_type(gen->typechecker, for_stmt->iterable);

            // Generate range-based for loop
            const char* iter_var = ccodegen_temp_var(gen);
            const char* len_var = ccodegen_temp_var(gen);

            emit_indent(gen);
            emit(gen, "{\n");
            gen->indent_level++;

            // Get iterable and its length
            emit_indent(gen);
            if (iter_type && iter_type->kind == TY_LIST) {
                const char* elem_type = ccodegen_type_to_c(gen, iter_type->as.list.element);
                emit(gen, "int %s = PxList_len(", len_var);
                gen_expr(gen, for_stmt->iterable);
                emit(gen, ");\n");

                emit_indent(gen);
                emit(gen, "for (int %s = 0; %s < %s; %s++) {\n",
                     iter_var, iter_var, len_var, iter_var);
                gen->indent_level++;

                emit_indent(gen);
                emit(gen, "%s %.*s = PxList_get(", elem_type,
                     for_stmt->name.length, for_stmt->name.start);
                gen_expr(gen, for_stmt->iterable);
                emit(gen, ", %s);\n", iter_var);
            } else {
                // Fallback for other iterables
                emit(gen, "/* for-in over non-list not fully supported */\n");
            }

            gen_stmt(gen, for_stmt->body);

            gen->indent_level--;
            emit_line(gen, "}");

            gen->indent_level--;
            emit_line(gen, "}");
            break;
        }

        case STMT_RETURN: {
            StmtReturn* ret = (StmtReturn*)stmt;
            emit_indent(gen);
            if (ret->value) {
                emit(gen, "return ");
                gen_expr(gen, ret->value);
                emit(gen, ";\n");
            } else {
                emit(gen, "return;\n");
            }
            break;
        }

        case STMT_BREAK:
            emit_line(gen, "break;");
            break;

        case STMT_CONTINUE:
            emit_line(gen, "continue;");
            break;

        case STMT_FUNCTION: {
            StmtFunction* fn = (StmtFunction*)stmt;
            Type* fn_type = typechecker_lookup(gen->typechecker,
                                               fn->name.start, fn->name.length);

            // Special handling for main function - always void px_main(void)
            bool is_main = (fn->name.length == 4 && memcmp(fn->name.start, "main", 4) == 0);

            // Get return type
            const char* ret_type = "void";
            if (!is_main && fn_type && fn_type->kind == TY_FUNC && fn_type->as.func.return_type) {
                Type* rt = fn_type->as.func.return_type;
                // Don't use "any" return type - default to void
                if (rt->kind != TY_ANY && rt->kind != TY_NONE) {
                    ret_type = ccodegen_type_to_c(gen, rt);
                }
            }

            emit_newline(gen);
            emit(gen, "%s px_%.*s(", ret_type, fn->name.length, fn->name.start);

            // Parameters (skip for main)
            if (!is_main) {
                for (int i = 0; i < fn->param_count; i++) {
                    if (i > 0) emit(gen, ", ");
                    const char* param_type = "PxValue";
                    if (fn_type && fn_type->kind == TY_FUNC && i < fn_type->as.func.param_count) {
                        param_type = ccodegen_type_to_c(gen, fn_type->as.func.param_types[i]);
                    }
                    emit(gen, "%s %.*s", param_type,
                         fn->params[i].length, fn->params[i].start);
                }
            } else if (fn->param_count == 0) {
                emit(gen, "void");
            }
            emit(gen, ") ");

            if (fn->body->type == STMT_BLOCK) {
                gen_stmt(gen, fn->body);
            } else {
                emit(gen, "{\n");
                gen->indent_level++;
                gen_stmt(gen, fn->body);
                gen->indent_level--;
                emit(gen, "}\n");
            }
            break;
        }

        case STMT_STRUCT: {
            StmtStruct* st = (StmtStruct*)stmt;

            emit_newline(gen);
            emit_line(gen, "// Struct: %.*s", st->name.length, st->name.start);

            // Generate struct definition
            emit_line(gen, "typedef struct Px%.*s {", st->name.length, st->name.start);
            gen->indent_level++;
            emit_line(gen, "PxHeader header;");

            for (int i = 0; i < st->field_count; i++) {
                Type* field_type = type_any();
                if (st->field_types && st->field_types[i]) {
                    field_type = typechecker_resolve_type_expr(gen->typechecker, st->field_types[i]);
                }
                const char* c_type = ccodegen_type_to_c(gen, field_type);
                emit_line(gen, "%s %.*s;", c_type,
                          st->fields[i].length, st->fields[i].start);
            }

            gen->indent_level--;
            emit_line(gen, "} Px%.*s;", st->name.length, st->name.start);
            emit_newline(gen);

            // Generate constructor
            emit(gen, "Px%.*s* Px%.*s_new(",
                 st->name.length, st->name.start,
                 st->name.length, st->name.start);

            for (int i = 0; i < st->field_count; i++) {
                if (i > 0) emit(gen, ", ");
                Type* field_type = type_any();
                if (st->field_types && st->field_types[i]) {
                    field_type = typechecker_resolve_type_expr(gen->typechecker, st->field_types[i]);
                }
                const char* c_type = ccodegen_type_to_c(gen, field_type);
                emit(gen, "%s %.*s", c_type,
                     st->fields[i].length, st->fields[i].start);
            }
            emit(gen, ") {\n");

            gen->indent_level++;
            emit_line(gen, "Px%.*s* self = px_alloc(sizeof(Px%.*s));",
                      st->name.length, st->name.start,
                      st->name.length, st->name.start);
            emit_line(gen, "self->header.refcount = 1;");

            for (int i = 0; i < st->field_count; i++) {
                emit_line(gen, "self->%.*s = %.*s;",
                          st->fields[i].length, st->fields[i].start,
                          st->fields[i].length, st->fields[i].start);
            }

            emit_line(gen, "return self;");
            gen->indent_level--;
            emit_line(gen, "}");

            // Generate methods
            for (int i = 0; i < st->method_count; i++) {
                gen_stmt(gen, st->methods[i]);
            }
            break;
        }

        default:
            emit_line(gen, "/* unknown stmt */");
            break;
    }
}

// ============================================================================
// Main Generation Entry Points
// ============================================================================

static void emit_header(CCodegen* gen) {
    emit_line(gen, "// Generated by Pixel AOT Compiler");
    emit_line(gen, "// Source: %s", gen->filename);
    emit_newline(gen);
    emit_line(gen, "#include <stdint.h>");
    emit_line(gen, "#include <stdbool.h>");
    emit_line(gen, "#include <stdlib.h>");
    emit_line(gen, "#include \"px_runtime.h\"");
    emit_newline(gen);
}

static void emit_main_wrapper(CCodegen* gen) {
    emit_newline(gen);
    emit_line(gen, "int main(int argc, char** argv) {");
    gen->indent_level++;
    emit_line(gen, "px_init();");
    emit_line(gen, "px_main();");
    emit_line(gen, "px_shutdown();");
    emit_line(gen, "return 0;");
    gen->indent_level--;
    emit_line(gen, "}");
}

char* ccodegen_generate(CCodegen* gen, Stmt** stmts, int count) {
    emit_header(gen);

    // Check if user defined a main function
    bool has_user_main = false;
    for (int i = 0; i < count; i++) {
        if (stmts[i]->type == STMT_FUNCTION) {
            StmtFunction* fn = (StmtFunction*)stmts[i];
            if (fn->name.length == 4 && memcmp(fn->name.start, "main", 4) == 0) {
                has_user_main = true;
                break;
            }
        }
    }

    // First pass: generate struct definitions and function prototypes
    for (int i = 0; i < count; i++) {
        if (stmts[i]->type == STMT_STRUCT) {
            gen_stmt(gen, stmts[i]);
        }
    }

    // Second pass: generate function definitions
    for (int i = 0; i < count; i++) {
        if (stmts[i]->type == STMT_FUNCTION) {
            gen_stmt(gen, stmts[i]);
        }
    }

    // Third pass: generate top-level code (if any and if no user main)
    bool has_toplevel = false;
    for (int i = 0; i < count; i++) {
        if (stmts[i]->type != STMT_STRUCT && stmts[i]->type != STMT_FUNCTION) {
            has_toplevel = true;
            break;
        }
    }

    if (has_toplevel && !has_user_main) {
        emit_newline(gen);
        emit_line(gen, "void px_main(void) {");
        gen->indent_level++;

        for (int i = 0; i < count; i++) {
            if (stmts[i]->type != STMT_STRUCT && stmts[i]->type != STMT_FUNCTION) {
                gen_stmt(gen, stmts[i]);
            }
        }

        gen->indent_level--;
        emit_line(gen, "}");
    } else if (!has_user_main) {
        // No top-level code and no user main - generate empty px_main
        emit_newline(gen);
        emit_line(gen, "void px_main(void) {");
        emit_line(gen, "}");
    }
    // If has_user_main, their main() was already generated as px_main()

    emit_main_wrapper(gen);

    if (gen->had_error) {
        return NULL;
    }

    return sb_finish(&gen->output);
}

bool ccodegen_generate_to_file(CCodegen* gen, Stmt** stmts, int count,
                               const char* output_path) {
    char* code = ccodegen_generate(gen, stmts, count);
    if (!code) return false;

    FILE* f = fopen(output_path, "w");
    if (!f) {
        gen->had_error = true;
        return false;
    }

    fputs(code, f);
    fclose(f);
    free(code);
    return true;
}
