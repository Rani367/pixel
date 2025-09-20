
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

Expr* expr_unary(Aren