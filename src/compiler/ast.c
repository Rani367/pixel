
            printf("\n");
            print_indent(indent + 1);
            printf("body:\n");
            ast_print_stmt(e->body, indent + 2);
            break;
        }

        case EXPR_VEC2: {
            printf("Vec2\n");
            ExprVec2* e = (ExprVec2*)expr;
            print_indent(indent + 1);
            printf("x:\n");
            ast_print_expr(e->x, indent + 2);
            print_indent(indent + 1);
            printf("y:\n");
            ast_print_expr(e->y, indent + 2);
            break;
        }

        case EXPR_POSTFIX: {
            ExprPostfix* e = (ExprPostfix*)expr;
            printf("Postfix(%s)\n", token_type_name(e->op.type));
            ast_print_expr(e->operand, indent + 1);
            break;
        }

        case EXPR_COUNT:
            printf("(invalid)\n");
            break;
    }
}

void ast_print_stmt(Stmt* stmt, int indent) {
    if (!stmt) {
        print_indent(indent);
        printf("(null)\n");
        return;
    }

    print_indent(indent);

    switch (stmt->type) {
        case STMT_EXPRESSION: {
            printf("ExpressionStmt\n");
            StmtExpression* s = (StmtExpression*)stmt;
            ast_print_expr(s->expression, indent + 1);
            break;
        }

        case STMT_ASSIGNMENT: {
            printf("Assignment\n");
            StmtAssignment* s = (StmtAssignment*)stmt;
            print_indent(indent + 1);
            printf("target:\n");
            ast_print_expr(s->target, indent + 2);
            print_indent(indent + 1);
            printf("value:\n");
            ast_print_expr(s->value, indent + 2);
            break;
        }

        case STMT_BLOCK: {
            StmtBlock* s = (StmtBlock*)stmt;
            printf("Block(%d statements)\n", s->count);
            for (int i = 0; i < s->count; i++) {
                ast_print_stmt(s->statements[i], indent + 1);
            }
            break;
        }

        case STMT_IF: {
            printf("If\n");
            StmtIf* s = (StmtIf*)stmt;
            print_indent(indent + 1);
            printf("condition:\n");
            ast_print_expr(s->condition, indent + 2);
            print_indent(indent + 1);
            printf("then:\n");
            ast_print_stmt(s->then_branch, indent + 2);
            if (s->else_branch) {
                print_indent(indent + 1);
                printf("else:\n");
                ast_print_stmt(s->else_branch, indent + 2);
            }
            break;
        }

        case STMT_WHILE: {
            printf("While\n");
            StmtWhile* s = (StmtWhile*)stmt;
            print_indent(indent + 1);
            printf("condition:\n");
            ast_print_expr(s->condition, indent + 2);
            print_indent(indent + 1);
            printf("body:\n");
            ast_print_stmt(s->body, indent + 2);
            break;
        }

        case STMT_FOR: {
            StmtFor* s = (StmtFor*)stmt;
            printf("For(%.*s in)\n", s->name.length, s->name.start);
            print_indent(indent + 1);
            printf("iterable:\n");
            ast_print_expr(s->iterable, indent + 2);
            print_indent(indent + 1);
            printf("body:\n");
            ast_print_stmt(s->body, indent + 2);
            break;
        }

        case STMT_RETURN: {
            printf("Return\n");
            StmtReturn* s = (StmtReturn*)stmt;
            if (s->value) {
                ast_print_expr(s->value, indent + 1);
            }
            break;
        }

        case STMT_BREAK:
            printf("Break\n");
            break;

        case STMT_CONTINUE:
            printf("Continue\n");
            break;

        case STMT_FUNCTION: {
            StmtFunction* s = (StmtFunction*)stmt;
            printf("FunctionDecl(%.*s, %d params)\n",
                   s->name.length, s->name.start, s->param_count);
            print_indent(indent + 1);
            printf("params: ");
            for (int i = 0; i < s->param_count; i++) {
                if (i > 0) printf(", ");
                printf("%.*s", s->params[i].length, s->params[i].start);
            }
            printf("\n");
            print_indent(indent + 1);
            printf("body:\n");
            ast_print_stmt(s->body, indent + 2);
            break;
        }

        case STMT_STRUCT: {
            StmtStruct* s = (StmtStruct*)stmt;
            printf("Struct(%.*s, %d fields)\n",
                   s->name.length, s->name.start, s->field_count);
            print_indent(indent + 1);
            printf("fields: ");
            for (int i = 0; i < s->field_count; i++) {
                if (i > 0) printf(", ");
                printf("%.*s", s->fields[i].length, s->fields[i].start);
            }
            printf("\n");
            break;
        }

        case STMT_COUNT:
            printf("(invalid)\n");
            break;
    }
}
 start.start_line,
        .start_column = start.start_column,
        .end_line = end.end_line,
        .end_column = end.end_column,
    };
}

// ============================================================================
// Expression Constructors
// ============================================================================

Expr* expr_literal_null(Arena* arena, Span span) {
    ExprLiteralNull* expr = arena_alloc(arena, sizeof(ExprLiteralNull));
    expr->base.type = EXPR_LITERAL_NULL;
    expr->base.span = span;
    return (Expr*)expr;
}

Expr* expr_literal_bool(Arena* arena, bool value, Span span) {
    ExprLiteralBool* expr = arena_alloc(arena, sizeof(ExprLiteralBool));
    expr->base.type = EXPR_LITERAL_BOOL;
    expr->base.span = span;
    expr->value = value;
    return (Expr*)expr;
}

Expr* expr_literal_number(Arena* arena, double value, Span span) {
    ExprLiteralNumber* expr = arena_alloc(arena, sizeof(ExprLiteralNumber));
    expr->base.type = EXPR_LITERAL_NUMBER;
    expr->base.span = span;
    expr->value = value;
    return (Expr*)expr;
}

Expr* expr_literal_string(Arena* arena, const char* value, int length, Span span) {
    ExprLiteralString* expr = arena_alloc(arena, sizeof(ExprLiteralString));
    expr->base.type = EXPR_LITERAL_STRING;
    expr->base.span = span;
    expr->length = length;
    // Copy string into arena
    expr->value = arena_alloc(arena, length + 1);
    memcpy(expr->value, value, length);
    expr->value[length] = '\0';
    return (Expr*)expr;
}

Expr* expr_identifier(Arena* arena, Token name) {
    ExprIdentifier* expr = arena_alloc(arena, sizeof(ExprIdentifier));
    expr->base.type = EXPR_IDENTIFIER;
    expr->base.span = span_from_token(name);
    expr->name = name;
    return (Expr*)expr;
}

Expr* expr_unary(Aren   stmt->base.type = STMT_IF;
    stmt->base.span = span;
    stmt->condition = condition;
    stmt->then_branch = then_branch;
    stmt->else_branch = else_branch;
    return (Stmt*)stmt;
}

Stmt* stmt_while(Arena* arena, Expr* condition, Stmt* body, Span span) {
    StmtWhile* stmt = arena_alloc(arena, sizeof(StmtWhile));
    stmt->base.type = STMT_WHILE;
    stmt->base.span = span;
    stmt->condition = condition;
    stmt->body = body;
    return (Stmt*)stmt;
}

Stmt* stmt_for(Arena* arena, Token name, Expr* iterable, Stmt* body, Span span) {
    StmtFor* stmt = arena_alloc(arena, sizeof(StmtFor));
    stmt->base.type = STMT_FOR;
    stmt->base.span = span;
    stmt->name = name;
    stmt->iterable = iterable;
    stmt->body = body;
    return (Stmt*)stmt;
}

Stmt* stmt_return(Arena* arena, Expr* value, Span span) {
    StmtReturn* stmt = arena_alloc(arena, sizeof(StmtReturn));
    stmt->base.type = STMT_RETURN;
    stmt->base.span = span;
    stmt->value = value;
    return (Stmt*)stmt;
}

Stmt* stmt_break(Arena* arena, Span span) {
    StmtBreak* stmt = arena_alloc(arena, sizeof(StmtBreak));
    stmt->base.type = STMT_BREAK;
    stmt->base.span = span;
    return (Stmt*)stmt;
}

Stmt* stmt_continue(Arena* arena, Span span) {
    StmtContinue* stmt = arena_alloc(arena, sizeof(StmtContinue));
    stmt->base.type = STMT_CONTINUE;
    stmt->base.span = span;
    return (Stmt*)stmt;
}

Stmt* stmt_function(Arena* arena, Token name, Token* params, int param_count, Stmt* body, Span span) {
    StmtFunction* stmt = arena_alloc(arena, sizeof(StmtFunction));
    stmt->base.type = STMT_FUNCTION;
    stmt->base.span = span;
    stmt->name = name;
    stmt->params = params;
    stmt->param_count = param_count;
    stmt->body = body;
    return (Stmt*)stmt;
}

Stmt* stmt_struct(Arena* arena, Token name, Token* fields, int field_count,
                  Stmt** methods, int method_count, Span span) {
    StmtStruct* stmt = arena_alloc(arena, sizeof(StmtStruct));
    stmt->base.type = STMT_STRUCT;
    stmt->base.span = span;
    stmt->name = name;
    stmt->fields = fields;
    stmt->field_count = field_count;
    stmt->methods = methods;
    stmt->method_count = method_count;
    return (Stmt*)stmt;
}

// ============================================================================
// Visitor Pattern
// ============================================================================

void expr_accept(Expr* expr, ExprVisitor* visitor) {
    if (!expr || !visitor) return;

    ExprVisitFn fn = NULL;
    switch (expr->type) {
        case EXPR_LITERAL_NULL:   fn = visitor->visit_literal_null;   break;
        case EXPR_LITERAL_BOOL:   fn = visitor->visit_literal_bool;   break;
        case EXPR_LITERAL_NUMBER: fn = visitor->visit_literal_number; break;
        case EXPR_LITERAL_STRING: fn = visitor->visit_literal_string; break;
        case EXPR_IDENTIFIER:     fn = visitor->visit_identifier;     break;
        case EXPR_UNARY:          fn = visitor->visit_unary;          break;
        case EXPR_BINARY:         fn = visitor->visit_binary;         break;
        case EXPR_CALL:           fn = visitor->visit_call;           break;
        case EXPR_GET:            fn = visitor->visit_get;            break;
        case EXPR_SET:            fn = visitor->visit_set;            break;
        case EXPR_INDEX:          fn = visitor->visit_index;          break;
        case EXPR_INDEX_SET:      fn = visitor->visit_index_set;      break;
        case EXPR_LIST:           fn = visitor->visit_list;           break;
        case EXPR_FUNCTION:       fn = visitor->visit_function;       break;
        case EXPR_VEC2:           fn = visitor->visit_vec2;           break;
        case EXPR_POSTFIX:        fn = visitor->visit_postfix;        break;
        case EXPR_COUNT:          break;
    }

    if (fn) {
        fn(expr, visitor->context);
    }
}

void stmt_accept(Stmt* stmt, StmtVisitor* visitor) {
    if (!stmt || !visitor) return;

    StmtVisitFn fn = NULL;
    switch (stmt->type) {
        case STMT_EXPRESSION: fn = visitor->visit_expression; break;
        case STMT_ASSIGNMENT: fn = visitor->visit_assignment; break;
        case STMT_BLOCK:      fn = visitor->visit_block;      break;
        case STMT_IF:         fn = visitor->visit_if;         break;
        case STMT_WHILE:      fn = visitor->visit_while;      break;
        case STMT_FOR:        fn = visitor->visit_for;        break;
        case STMT_RETURN:     fn = visitor->visit_return;     break;
        case STMT_BREAK:      fn = visitor->visit_break;      break;
        case STMT_CONTINUE:   fn = visitor->visit_continue;   break;
        case STMT_FUNCTION:   fn = visitor->visit_function;   break;
        case STMT_STRUCT:     fn = visitor->visit_struct;     break;
        case STMT_COUNT:      break;
    }

    if (fn) {
        fn(stmt, visitor->context);
    }
}

// ============================================================================
// Debug Utilities
// ============================================================================

static const char* expr_type_names[] = {
    [EXPR_LITERAL_NULL]   = "LiteralNull",
    [EXPR_LITERAL_BOOL]   = "LiteralBool",
    [EXPR_LITERAL_NUMBER] = "LiteralNumber",
    [EXPR_LITERAL_STRING] = "LiteralString",
    [EXPR_IDENTIFIER]     = "Identifier",
    [EXPR_UNARY]          = "Unary",
    [EXPR_BINARY]         = "Binary",
    [EXPR_CALL]           = "Call",
    [EXPR_GET]            = "Get",
    [EXPR_SET]            = "Set",
    [EXPR_INDEX]          = "Index",
    [EXPR_INDEX_SET]      = "IndexSet",
    [EXPR_LIST]           = "List",
    [EXPR_FUNCTION]       = "Function",
    [EXPR_VEC2]           = "Vec2",
    [EXPR_POSTFIX]        = "Postfix",
};

static const char* stmt_type_names[] = {
    [STMT_EXPRESSION] = "Expression",
    [STMT_ASSIGNMENT] = "Assignment",
    [STMT_BLOCK]      = "Block",
    [STMT_IF]         = "If",
    [STMT_WHILE]      = "While",
    [STMT_FOR]        = "For",
    [STMT_RETURN]     = "Return",
    [STMT_BREAK]      = "Break",
    [STMT_CONTINUE]   = "Continue",
    [STMT_FUNCTION]   = "Function",
    [STMT_STRUCT]     = "Struct",
};

const char* expr_type_name(ExprType type) {
    if (type >= 0 && type < EXPR_COUNT) {
        return expr_type_names[type];
    }
    return "Unknown";
}

const char* stmt_type_name(StmtType type) {
    if (type >= 0 && type < STMT_COUNT) {
        return stmt_type_names[type];
    }
    return "Unknown";
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void ast_print_expr(Expr* expr, int indent) {
    if (!expr) {
        print_indent(indent);
        printf("(null)\n");
        return;
    }

    print_indent(indent);

    switch (expr->type) {
        case EXPR_LITERAL_NULL:
            printf("LiteralNull\n");
            break;

        case EXPR_LITERAL_BOOL: {
            ExprLiteralBool* e = (ExprLiteralBool*)expr;
            printf("LiteralBool(%s)\n", e->value ? "true" : "false");
            break;
        }

        case EXPR_LITERAL_NUMBER: {
            ExprLiteralNumber* e = (ExprLiteralNumber*)expr;
            printf("LiteralNumber(%g)\n", e->value);
            break;
        }

        case EXPR_LITERAL_STRING: {
            ExprLiteralString* e = (ExprLiteralString*)expr;
            printf("LiteralString(\"%.*s\")\n", e->length, e->value);
            break;
        }

        case EXPR_IDENTIFIER: {
            ExprIdentifier* e = (ExprIdentifier*)expr;
            printf("Identifier(%.*s)\n", e->name.length, e->name.start);
            break;
        }

        case EXPR_UNARY: {
            ExprUnary* e = (ExprUnary*)expr;
            printf("Unary(%s)\n", token_type_name(e->operator));
            ast_print_expr(e->operand, indent + 1);
            break;
        }

        case EXPR_BINARY: {
            ExprBinary* e = (ExprBinary*)expr;
            printf("Binary(%s)\n", token_type_name(e->operator));
            ast_print_expr(e->left, indent + 1);
            ast_print_expr(e->right, indent + 1);
            break;
        }

        case EXPR_CALL: {
            ExprCall* e = (ExprCall*)expr;
            printf("Call\n");
            print_indent(indent + 1);
            printf("callee:\n");
            ast_print_expr(e->callee, indent + 2);
            print_indent(indent + 1);
            printf("arguments: %d\n", e->arg_count);
            for (int i = 0; i < e->arg_count; i++) {
                ast_print_expr(e->arguments[i], indent + 2);
            }
            break;
        }

        case EXPR_GET: {
            ExprGet* e = (ExprGet*)expr;
            printf("Get(.%.*s)\n", e->name.length, e->name.start);
            ast_print_expr(e->object, indent + 1);
            break;
        }

        case EXPR_SET: {
            ExprSet* e = (ExprSet*)expr;
            printf("Set(.%.*s)\n", e->name.length, e->name.start);
            print_indent(indent + 1);
            printf("object:\n");
            ast_print_expr(e->object, indent + 2);
            print_indent(indent + 1);
            printf("value:\n");
            ast_print_expr(e->value, indent + 2);
            break;
        }

        case EXPR_INDEX: {
            printf("Index\n");
            ExprIndex* e = (ExprIndex*)expr;
            print_indent(indent + 1);
            printf("object:\n");
            ast_print_expr(e->object, indent + 2);
            print_indent(indent + 1);
            printf("index:\n");
            ast_print_expr(e->index, indent + 2);
            break;
        }

        case EXPR_INDEX_SET: {
            printf("IndexSet\n");
            ExprIndexSet* e = (ExprIndexSet*)expr;
            print_indent(indent + 1);
            printf("object:\n");
            ast_print_expr(e->object, indent + 2);
            print_indent(indent + 1);
            printf("index:\n");
            ast_print_expr(e->index, indent + 2);
            print_indent(indent + 1);
            printf("value:\n");
            ast_print_expr(e->value, indent + 2);
            break;
        }

        case EXPR_LIST: {
            ExprList* e = (ExprList*)expr;
            printf("List(%d elements)\n", e->count);
            for (int i = 0; i < e->count; i++) {
                ast_print_expr(e->elements[i], indent + 1);
            }
            break;
        }

        case EXPR_FUNCTION: {
            ExprFunction* e = (ExprFunction*)expr;
            printf("Function(%d params)\n", e->param_count);
            print_indent(indent + 1);
            printf("params: ");
            for (int i = 0; i < e->param_count; i++) {
                if (i > 0) printf(", ");
                printf("%.*s", e->params[i].length, e->params[i].start);
            }