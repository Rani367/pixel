#include "compiler/typechecker.h"
#include "vm/object.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// Error Reporting
// ============================================================================

static void error_at(TypeChecker* tc, Span span, const char* message) {
    tc->had_error = true;
    fprintf(stderr, "[line %d] Type error: %s\n", span.start_line, message);
}

static void error_type_mismatch(TypeChecker* tc, Span span, Type* expected, Type* actual) {
    tc->had_error = true;
    const char* exp_str = type_to_string(tc->arena, expected);
    const char* act_str = type_to_string(tc->arena, actual);
    fprintf(stderr, "[line %d] Type error: expected '%s', got '%s'\n",
            span.start_line, exp_str, act_str);
}

// ============================================================================
// Type Checker Initialization
// ============================================================================

void typechecker_init(TypeChecker* tc, Arena* arena, const char* filename, const char* source) {
    tc->arena = arena;
    tc->filename = filename;
    tc->source = source;
    tc->had_error = false;
    table_init(&tc->type_table);
    table_init(&tc->struct_types);
}

void typechecker_free(TypeChecker* tc) {
    table_free(&tc->type_table);
    table_free(&tc->struct_types);
}

// ============================================================================
// Symbol Table Operations
// ============================================================================

static void declare_symbol(TypeChecker* tc, const char* name, int length, Type* type) {
    TypeInfo* info = arena_alloc(tc->arena, sizeof(TypeInfo));
    info->type = type;
    info->is_mutable = true;
    table_set(&tc->type_table, name, (size_t)length, info);
}

Type* typechecker_lookup(TypeChecker* tc, const char* name, int length) {
    TypeInfo* info = NULL;
    if (table_get(&tc->type_table, name, (size_t)length, (void**)&info)) {
        return info->type;
    }
    return NULL;
}

void typechecker_declare_builtin(TypeChecker* tc, const char* name, Type* type) {
    declare_symbol(tc, name, strlen(name), type);
}

void typechecker_register_builtins(TypeChecker* tc) {
    // I/O functions
    Type** print_params = arena_alloc(tc->arena, sizeof(Type*));
    print_params[0] = type_any();
    typechecker_declare_builtin(tc, "print", type_func(tc->arena, print_params, 1, type_none()));
    typechecker_declare_builtin(tc, "println", type_func(tc->arena, print_params, 1, type_none()));

    // Input
    typechecker_declare_builtin(tc, "input", type_func(tc->arena, NULL, 0, type_str()));

    Type** input_prompt_params = arena_alloc(tc->arena, sizeof(Type*));
    input_prompt_params[0] = type_str();
    typechecker_declare_builtin(tc, "input_prompt", type_func(tc->arena, input_prompt_params, 1, type_str()));

    // Type conversion
    Type** to_str_params = arena_alloc(tc->arena, sizeof(Type*));
    to_str_params[0] = type_any();
    typechecker_declare_builtin(tc, "str", type_func(tc->arena, to_str_params, 1, type_str()));

    Type** to_num_params = arena_alloc(tc->arena, sizeof(Type*));
    to_num_params[0] = type_any();
    typechecker_declare_builtin(tc, "num", type_func(tc->arena, to_num_params, 1, type_num()));

    Type** to_int_params = arena_alloc(tc->arena, sizeof(Type*));
    to_int_params[0] = type_any();
    typechecker_declare_builtin(tc, "int", type_func(tc->arena, to_int_params, 1, type_int()));

    Type** to_bool_params = arena_alloc(tc->arena, sizeof(Type*));
    to_bool_params[0] = type_any();
    typechecker_declare_builtin(tc, "bool", type_func(tc->arena, to_bool_params, 1, type_bool()));

    // String functions
    Type** len_params = arena_alloc(tc->arena, sizeof(Type*));
    len_params[0] = type_any();  // Works on strings and lists
    typechecker_declare_builtin(tc, "len", type_func(tc->arena, len_params, 1, type_int()));

    Type** split_params = arena_alloc(tc->arena, sizeof(Type*) * 2);
    split_params[0] = type_str();
    split_params[1] = type_str();
    typechecker_declare_builtin(tc, "split", type_func(tc->arena, split_params, 2, type_list(tc->arena, type_str())));

    Type** join_params = arena_alloc(tc->arena, sizeof(Type*) * 2);
    join_params[0] = type_list(tc->arena, type_str());
    join_params[1] = type_str();
    typechecker_declare_builtin(tc, "join", type_func(tc->arena, join_params, 2, type_str()));

    Type** upper_params = arena_alloc(tc->arena, sizeof(Type*));
    upper_params[0] = type_str();
    typechecker_declare_builtin(tc, "upper", type_func(tc->arena, upper_params, 1, type_str()));

    Type** lower_params = arena_alloc(tc->arena, sizeof(Type*));
    lower_params[0] = type_str();
    typechecker_declare_builtin(tc, "lower", type_func(tc->arena, lower_params, 1, type_str()));

    Type** trim_params = arena_alloc(tc->arena, sizeof(Type*));
    trim_params[0] = type_str();
    typechecker_declare_builtin(tc, "trim", type_func(tc->arena, trim_params, 1, type_str()));

    Type** contains_params = arena_alloc(tc->arena, sizeof(Type*) * 2);
    contains_params[0] = type_str();
    contains_params[1] = type_str();
    typechecker_declare_builtin(tc, "contains", type_func(tc->arena, contains_params, 2, type_bool()));

    Type** replace_params = arena_alloc(tc->arena, sizeof(Type*) * 3);
    replace_params[0] = type_str();
    replace_params[1] = type_str();
    replace_params[2] = type_str();
    typechecker_declare_builtin(tc, "replace", type_func(tc->arena, replace_params, 3, type_str()));

    Type** substring_params = arena_alloc(tc->arena, sizeof(Type*) * 3);
    substring_params[0] = type_str();
    substring_params[1] = type_int();
    substring_params[2] = type_int();
    typechecker_declare_builtin(tc, "substring", type_func(tc->arena, substring_params, 3, type_str()));

    // List functions
    Type** push_params = arena_alloc(tc->arena, sizeof(Type*) * 2);
    push_params[0] = type_list(tc->arena, type_any());
    push_params[1] = type_any();
    typechecker_declare_builtin(tc, "push", type_func(tc->arena, push_params, 2, type_none()));

    Type** pop_params = arena_alloc(tc->arena, sizeof(Type*));
    pop_params[0] = type_list(tc->arena, type_any());
    typechecker_declare_builtin(tc, "pop", type_func(tc->arena, pop_params, 1, type_any()));

    Type** insert_params = arena_alloc(tc->arena, sizeof(Type*) * 3);
    insert_params[0] = type_list(tc->arena, type_any());
    insert_params[1] = type_int();
    insert_params[2] = type_any();
    typechecker_declare_builtin(tc, "insert", type_func(tc->arena, insert_params, 3, type_none()));

    Type** remove_params = arena_alloc(tc->arena, sizeof(Type*) * 2);
    remove_params[0] = type_list(tc->arena, type_any());
    remove_params[1] = type_int();
    typechecker_declare_builtin(tc, "remove", type_func(tc->arena, remove_params, 2, type_any()));

    // Math functions
    typechecker_declare_builtin(tc, "abs", type_func(tc->arena, (Type*[]){type_num()}, 1, type_num()));
    typechecker_declare_builtin(tc, "floor", type_func(tc->arena, (Type*[]){type_num()}, 1, type_int()));
    typechecker_declare_builtin(tc, "ceil", type_func(tc->arena, (Type*[]){type_num()}, 1, type_int()));
    typechecker_declare_builtin(tc, "round", type_func(tc->arena, (Type*[]){type_num()}, 1, type_int()));
    typechecker_declare_builtin(tc, "sqrt", type_func(tc->arena, (Type*[]){type_num()}, 1, type_num()));
    typechecker_declare_builtin(tc, "sin", type_func(tc->arena, (Type*[]){type_num()}, 1, type_num()));
    typechecker_declare_builtin(tc, "cos", type_func(tc->arena, (Type*[]){type_num()}, 1, type_num()));
    typechecker_declare_builtin(tc, "tan", type_func(tc->arena, (Type*[]){type_num()}, 1, type_num()));
    typechecker_declare_builtin(tc, "atan2", type_func(tc->arena, (Type*[]){type_num(), type_num()}, 2, type_num()));

    Type** min_params = arena_alloc(tc->arena, sizeof(Type*) * 2);
    min_params[0] = type_num();
    min_params[1] = type_num();
    typechecker_declare_builtin(tc, "min", type_func(tc->arena, min_params, 2, type_num()));
    typechecker_declare_builtin(tc, "max", type_func(tc->arena, min_params, 2, type_num()));

    Type** clamp_params = arena_alloc(tc->arena, sizeof(Type*) * 3);
    clamp_params[0] = type_num();
    clamp_params[1] = type_num();
    clamp_params[2] = type_num();
    typechecker_declare_builtin(tc, "clamp", type_func(tc->arena, clamp_params, 3, type_num()));

    // Random
    typechecker_declare_builtin(tc, "random", type_func(tc->arena, NULL, 0, type_num()));

    Type** random_int_params = arena_alloc(tc->arena, sizeof(Type*) * 2);
    random_int_params[0] = type_int();
    random_int_params[1] = type_int();
    typechecker_declare_builtin(tc, "random_int", type_func(tc->arena, random_int_params, 2, type_int()));

    // Type checking
    Type** typeof_params = arena_alloc(tc->arena, sizeof(Type*));
    typeof_params[0] = type_any();
    typechecker_declare_builtin(tc, "typeof", type_func(tc->arena, typeof_params, 1, type_str()));
}

// ============================================================================
// Type Resolution from TypeExpr AST
// ============================================================================

Type* typechecker_resolve_type_expr(TypeChecker* tc, TypeExpr* type_expr) {
    if (!type_expr) return type_any();  // No annotation = dynamic

    switch (type_expr->kind) {
        case TYPE_EXPR_PRIMITIVE: {
            TypeExprPrimitive* prim = (TypeExprPrimitive*)type_expr;
            switch (prim->primitive_type) {
                case TOKEN_TYPE_NUM:  return type_num();
                case TOKEN_TYPE_INT:  return type_int();
                case TOKEN_TYPE_STR:  return type_str();
                case TOKEN_TYPE_BOOL: return type_bool();
                case TOKEN_TYPE_NONE: return type_none();
                default:
                    error_at(tc, type_expr->span, "Unknown primitive type");
                    return type_error();
            }
        }

        case TYPE_EXPR_ANY:
            return type_any();

        case TYPE_EXPR_LIST: {
            TypeExprList* list = (TypeExprList*)type_expr;
            Type* element = typechecker_resolve_type_expr(tc, list->element_type);
            return type_list(tc->arena, element);
        }

        case TYPE_EXPR_FUNC: {
            TypeExprFunc* func = (TypeExprFunc*)type_expr;
            Type** param_types = NULL;
            if (func->param_count > 0) {
                param_types = arena_alloc(tc->arena, sizeof(Type*) * func->param_count);
                for (int i = 0; i < func->param_count; i++) {
                    param_types[i] = typechecker_resolve_type_expr(tc, func->param_types[i]);
                }
            }
            Type* return_type = typechecker_resolve_type_expr(tc, func->return_type);
            return type_func(tc->arena, param_types, func->param_count, return_type);
        }

        case TYPE_EXPR_STRUCT: {
            TypeExprStruct* st = (TypeExprStruct*)type_expr;
            // Look up struct type
            Type* struct_type = NULL;
            if (table_get(&tc->struct_types, st->name.start, (size_t)st->name.length,
                          (void**)&struct_type)) {
                return struct_type;
            }
            error_at(tc, type_expr->span, "Unknown struct type");
            return type_error();
        }

        default:
            error_at(tc, type_expr->span, "Invalid type expression");
            return type_error();
    }
}

// ============================================================================
// Expression Type Inference
// ============================================================================

// Forward declarations
static Type* infer_expr(TypeChecker* tc, Expr* expr);
static void check_stmt(TypeChecker* tc, Stmt* stmt);

static Type* infer_expr(TypeChecker* tc, Expr* expr) {
    if (!expr) return type_error();

    switch (expr->type) {
        case EXPR_LITERAL_NULL:
            return type_none();

        case EXPR_LITERAL_BOOL:
            return type_bool();

        case EXPR_LITERAL_NUMBER: {
            ExprLiteralNumber* num = (ExprLiteralNumber*)expr;
            // Check if it's an integer (no decimal point)
            double val = num->value;
            if (val == (int32_t)val && val >= INT32_MIN && val <= INT32_MAX) {
                return type_num();  // Default to num for now
            }
            return type_num();
        }

        case EXPR_LITERAL_STRING:
            return type_str();

        case EXPR_IDENTIFIER: {
            ExprIdentifier* ident = (ExprIdentifier*)expr;
            Type* type = typechecker_lookup(tc, ident->name.start, ident->name.length);
            if (!type) {
                error_at(tc, expr->span, "Undefined variable");
                return type_error();
            }
            return type;
        }

        case EXPR_UNARY: {
            ExprUnary* unary = (ExprUnary*)expr;
            Type* operand_type = infer_expr(tc, unary->operand);

            switch (unary->operator) {
                case TOKEN_MINUS:
                    if (!type_is_numeric(operand_type)) {
                        error_at(tc, expr->span, "Unary '-' requires numeric operand");
                        return type_error();
                    }
                    return operand_type;

                case TOKEN_NOT:
                    return type_bool();

                default:
                    return type_error();
            }
        }

        case EXPR_BINARY: {
            ExprBinary* binary = (ExprBinary*)expr;
            Type* left = infer_expr(tc, binary->left);
            Type* right = infer_expr(tc, binary->right);

            switch (binary->operator) {
                // Arithmetic operators
                case TOKEN_PLUS:
                    // String concatenation
                    if (left->kind == TY_STR && right->kind == TY_STR) {
                        return type_str();
                    }
                    // Numeric addition
                    if (type_is_numeric(left) && type_is_numeric(right)) {
                        // If either is num, result is num
                        if (left->kind == TY_NUM || right->kind == TY_NUM) {
                            return type_num();
                        }
                        return type_int();
                    }
                    error_at(tc, expr->span, "'+' requires numeric or string operands");
                    return type_error();

                case TOKEN_MINUS:
                case TOKEN_STAR:
                case TOKEN_SLASH:
                case TOKEN_PERCENT:
                    if (type_is_numeric(left) && type_is_numeric(right)) {
                        if (left->kind == TY_NUM || right->kind == TY_NUM) {
                            return type_num();
                        }
                        return type_int();
                    }
                    error_at(tc, expr->span, "Arithmetic operators require numeric operands");
                    return type_error();

                // Comparison operators
                case TOKEN_LESS:
                case TOKEN_LESS_EQUAL:
                case TOKEN_GREATER:
                case TOKEN_GREATER_EQUAL:
                    if (type_is_numeric(left) && type_is_numeric(right)) {
                        return type_bool();
                    }
                    if (left->kind == TY_STR && right->kind == TY_STR) {
                        return type_bool();
                    }
                    error_at(tc, expr->span, "Comparison requires comparable operands");
                    return type_error();

                // Equality operators
                case TOKEN_EQUAL_EQUAL:
                case TOKEN_BANG_EQUAL:
                    return type_bool();

                // Logical operators
                case TOKEN_AND:
                case TOKEN_OR:
                    return type_bool();

                default:
                    return type_error();
            }
        }

        case EXPR_CALL: {
            ExprCall* call = (ExprCall*)expr;
            Type* callee_type = infer_expr(tc, call->callee);

            if (callee_type->kind == TY_ANY) {
                // Dynamic call - return any
                return type_any();
            }

            if (callee_type->kind != TY_FUNC) {
                error_at(tc, expr->span, "Cannot call non-function");
                return type_error();
            }

            // Check argument count
            if (call->arg_count != callee_type->as.func.param_count) {
                error_at(tc, expr->span, "Wrong number of arguments");
                return type_error();
            }

            // Check argument types
            for (int i = 0; i < call->arg_count; i++) {
                Type* param_type = callee_type->as.func.param_types[i];
                Type* arg_type = infer_expr(tc, call->arguments[i]);
                if (!types_compatible(param_type, arg_type)) {
                    error_type_mismatch(tc, call->arguments[i]->span, param_type, arg_type);
                }
            }

            return callee_type->as.func.return_type;
        }

        case EXPR_GET: {
            ExprGet* get = (ExprGet*)expr;
            Type* object_type = infer_expr(tc, get->object);

            if (object_type->kind == TY_ANY) {
                return type_any();
            }

            if (object_type->kind != TY_STRUCT) {
                error_at(tc, expr->span, "Cannot access property of non-struct");
                return type_error();
            }

            // Look up field in struct
            for (int i = 0; i < object_type->as.struct_type.field_count; i++) {
                const char* field_name = object_type->as.struct_type.field_names[i];
                if (strlen(field_name) == (size_t)get->name.length &&
                    memcmp(field_name, get->name.start, get->name.length) == 0) {
                    return object_type->as.struct_type.field_types[i];
                }
            }

            error_at(tc, expr->span, "Unknown field");
            return type_error();
        }

        case EXPR_SET: {
            ExprSet* set = (ExprSet*)expr;
            Type* value_type = infer_expr(tc, set->value);
            // The type of a set expression is the value type
            return value_type;
        }

        case EXPR_INDEX: {
            ExprIndex* index = (ExprIndex*)expr;
            Type* object_type = infer_expr(tc, index->object);
            Type* index_type = infer_expr(tc, index->index);

            if (object_type->kind == TY_ANY) {
                return type_any();
            }

            // String indexing returns str (single character)
            if (object_type->kind == TY_STR) {
                if (!type_is_numeric(index_type)) {
                    error_at(tc, expr->span, "String index must be numeric");
                }
                return type_str();
            }

            // List indexing returns element type
            if (object_type->kind == TY_LIST) {
                if (!type_is_numeric(index_type)) {
                    error_at(tc, expr->span, "List index must be numeric");
                }
                return object_type->as.list.element;
            }

            error_at(tc, expr->span, "Cannot index this type");
            return type_error();
        }

        case EXPR_INDEX_SET: {
            ExprIndexSet* set = (ExprIndexSet*)expr;
            Type* value_type = infer_expr(tc, set->value);
            return value_type;
        }

        case EXPR_LIST: {
            ExprList* list = (ExprList*)expr;
            if (list->count == 0) {
                // Empty list - type is list<any>
                return type_list(tc->arena, type_any());
            }

            // Infer element type from first element
            Type* element_type = infer_expr(tc, list->elements[0]);

            // Check all elements have compatible types
            for (int i = 1; i < list->count; i++) {
                Type* elem_type = infer_expr(tc, list->elements[i]);
                if (!types_compatible(element_type, elem_type)) {
                    error_at(tc, list->elements[i]->span,
                             "List elements must have compatible types");
                }
            }

            return type_list(tc->arena, element_type);
        }

        case EXPR_FUNCTION: {
            ExprFunction* fn = (ExprFunction*)expr;

            // Resolve parameter types
            Type** param_types = NULL;
            if (fn->param_count > 0) {
                param_types = arena_alloc(tc->arena, sizeof(Type*) * fn->param_count);
                for (int i = 0; i < fn->param_count; i++) {
                    if (fn->param_types && fn->param_types[i]) {
                        param_types[i] = typechecker_resolve_type_expr(tc, fn->param_types[i]);
                    } else {
                        param_types[i] = type_any();
                    }
                }
            }

            // Resolve return type
            Type* return_type = fn->return_type ?
                typechecker_resolve_type_expr(tc, fn->return_type) : type_any();

            return type_func(tc->arena, param_types, fn->param_count, return_type);
        }

        case EXPR_VEC2:
            // vec2 is a struct with x, y fields of type num
            return type_any();  // For now, treat as any

        case EXPR_POSTFIX: {
            ExprPostfix* postfix = (ExprPostfix*)expr;
            Type* operand_type = infer_expr(tc, postfix->operand);
            if (!type_is_numeric(operand_type)) {
                error_at(tc, expr->span, "Postfix operator requires numeric operand");
            }
            return operand_type;
        }

        default:
            return type_error();
    }
}

Type* typechecker_get_expr_type(TypeChecker* tc, Expr* expr) {
    return infer_expr(tc, expr);
}

// ============================================================================
// Statement Type Checking
// ============================================================================

static void check_stmt(TypeChecker* tc, Stmt* stmt) {
    if (!stmt) return;

    switch (stmt->type) {
        case STMT_EXPRESSION: {
            StmtExpression* expr_stmt = (StmtExpression*)stmt;
            infer_expr(tc, expr_stmt->expression);
            break;
        }

        case STMT_ASSIGNMENT: {
            StmtAssignment* assign = (StmtAssignment*)stmt;
            Type* target_type = infer_expr(tc, assign->target);
            Type* value_type = infer_expr(tc, assign->value);

            if (target_type->kind != TY_ANY && !types_compatible(target_type, value_type)) {
                error_type_mismatch(tc, assign->value->span, target_type, value_type);
            }

            // If target is untyped identifier, declare it with the value's type
            if (assign->target->type == EXPR_IDENTIFIER) {
                ExprIdentifier* ident = (ExprIdentifier*)assign->target;
                Type* existing = typechecker_lookup(tc, ident->name.start, ident->name.length);
                if (!existing) {
                    declare_symbol(tc, ident->name.start, ident->name.length, value_type);
                }
            }
            break;
        }

        case STMT_VAR_DECL: {
            StmtVarDecl* decl = (StmtVarDecl*)stmt;
            Type* declared_type = typechecker_resolve_type_expr(tc, decl->type);
            Type* init_type = infer_expr(tc, decl->initializer);

            if (!types_compatible(declared_type, init_type)) {
                error_type_mismatch(tc, decl->initializer->span, declared_type, init_type);
            }

            declare_symbol(tc, decl->name.start, decl->name.length, declared_type);
            break;
        }

        case STMT_BLOCK: {
            StmtBlock* block = (StmtBlock*)stmt;
            for (int i = 0; i < block->count; i++) {
                check_stmt(tc, block->statements[i]);
            }
            break;
        }

        case STMT_IF: {
            StmtIf* if_stmt = (StmtIf*)stmt;
            Type* cond_type = infer_expr(tc, if_stmt->condition);
            if (cond_type->kind != TY_BOOL && cond_type->kind != TY_ANY) {
                error_at(tc, if_stmt->condition->span, "Condition must be boolean");
            }
            check_stmt(tc, if_stmt->then_branch);
            if (if_stmt->else_branch) {
                check_stmt(tc, if_stmt->else_branch);
            }
            break;
        }

        case STMT_WHILE: {
            StmtWhile* while_stmt = (StmtWhile*)stmt;
            Type* cond_type = infer_expr(tc, while_stmt->condition);
            if (cond_type->kind != TY_BOOL && cond_type->kind != TY_ANY) {
                error_at(tc, while_stmt->condition->span, "Condition must be boolean");
            }
            check_stmt(tc, while_stmt->body);
            break;
        }

        case STMT_FOR: {
            StmtFor* for_stmt = (StmtFor*)stmt;
            Type* iter_type = infer_expr(tc, for_stmt->iterable);

            // Iterable must be a list or string
            Type* element_type = type_any();
            if (iter_type->kind == TY_LIST) {
                element_type = iter_type->as.list.element;
            } else if (iter_type->kind == TY_STR) {
                element_type = type_str();
            } else if (iter_type->kind != TY_ANY) {
                error_at(tc, for_stmt->iterable->span, "For loop requires iterable");
            }

            // Declare loop variable
            declare_symbol(tc, for_stmt->name.start, for_stmt->name.length, element_type);
            check_stmt(tc, for_stmt->body);
            break;
        }

        case STMT_RETURN: {
            StmtReturn* ret = (StmtReturn*)stmt;
            if (ret->value) {
                infer_expr(tc, ret->value);
            }
            // Return type checking requires function context - skip for now
            break;
        }

        case STMT_BREAK:
        case STMT_CONTINUE:
            // No type checking needed
            break;

        case STMT_FUNCTION: {
            StmtFunction* fn = (StmtFunction*)stmt;

            // Resolve parameter types
            Type** param_types = NULL;
            if (fn->param_count > 0) {
                param_types = arena_alloc(tc->arena, sizeof(Type*) * fn->param_count);
                for (int i = 0; i < fn->param_count; i++) {
                    if (fn->param_types && fn->param_types[i]) {
                        param_types[i] = typechecker_resolve_type_expr(tc, fn->param_types[i]);
                    } else {
                        param_types[i] = type_any();
                    }
                }
            }

            // Resolve return type
            Type* return_type = fn->return_type ?
                typechecker_resolve_type_expr(tc, fn->return_type) : type_any();

            // Create function type and declare it
            Type* fn_type = type_func(tc->arena, param_types, fn->param_count, return_type);
            declare_symbol(tc, fn->name.start, fn->name.length, fn_type);

            // Declare parameters in function scope and check body
            for (int i = 0; i < fn->param_count; i++) {
                declare_symbol(tc, fn->params[i].start, fn->params[i].length, param_types[i]);
            }
            check_stmt(tc, fn->body);
            break;
        }

        case STMT_STRUCT: {
            StmtStruct* st = (StmtStruct*)stmt;

            // Resolve field types
            Type** field_types = arena_alloc(tc->arena, sizeof(Type*) * st->field_count);
            const char** field_names = arena_alloc(tc->arena, sizeof(char*) * st->field_count);

            for (int i = 0; i < st->field_count; i++) {
                // Copy field name
                char* name = arena_alloc(tc->arena, st->fields[i].length + 1);
                memcpy(name, st->fields[i].start, st->fields[i].length);
                name[st->fields[i].length] = '\0';
                field_names[i] = name;

                if (st->field_types && st->field_types[i]) {
                    field_types[i] = typechecker_resolve_type_expr(tc, st->field_types[i]);
                } else {
                    field_types[i] = type_any();
                }
            }

            // Create struct type
            Type* struct_type = type_struct(tc->arena, st->name.start, st->name.length,
                                            field_types, field_names, st->field_count);

            // Register struct type
            table_set(&tc->struct_types, st->name.start, (size_t)st->name.length, struct_type);

            // Struct constructor has function type: func(field_types...) -> StructType
            Type* constructor_type = type_func(tc->arena, field_types, st->field_count, struct_type);
            declare_symbol(tc, st->name.start, st->name.length, constructor_type);

            // Check methods
            for (int i = 0; i < st->method_count; i++) {
                check_stmt(tc, st->methods[i]);
            }
            break;
        }

        default:
            break;
    }
}

// ============================================================================
// Main Entry Point
// ============================================================================

bool typechecker_check(TypeChecker* tc, Stmt** stmts, int count) {
    for (int i = 0; i < count; i++) {
        check_stmt(tc, stmts[i]);
    }
    return !tc->had_error;
}
