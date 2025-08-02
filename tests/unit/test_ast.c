ena, 2, span);
    Stmt* else_branch = stmt_expression(arena, else_expr);

    Stmt* if_stmt = stmt_if(arena, condition, then_branch, else_branch, span);

    ASSERT_EQ(if_stmt->type, STMT_IF);
    StmtIf* s = (StmtIf*)if_stmt;
    ASSERT_EQ(s->condition, condition);
    ASSERT_EQ(s->then_branch, then_branch);
    ASSERT_EQ(s->else_branch, else_branch);

    arena_free(arena);
}

TEST(stmt_if_no_else) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 5, .end_column = 1};

    Expr* condition = expr_literal_bool(arena, true, span);
    Stmt* then_branch = stmt_expression(arena, expr_literal_number(arena, 1, span));

    Stmt* if_stmt = stmt_if(arena, condition, then_branch, NULL, span);

    StmtIf* s = (StmtIf*)if_stmt;
    ASSERT_NULL(s->else_branch);

    arena_free(arena);
}

TEST(stmt_while_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 5, .end_column = 1};

    Expr* condition = expr_literal_bool(arena, true, span);
    Stmt* body = stmt_expression(arena, expr_literal_number(arena, 42, span));

    Stmt* while_stmt = stmt_while(arena, condition, body, span);

    ASSERT_EQ(while_stmt->type, STMT_WHILE);
    StmtWhile* w = (StmtWhile*)while_stmt;
    ASSERT_EQ(w->condition, condition);
    ASSERT_EQ(w->body, body);

    arena_free(arena);
}

TEST(stmt_return_with_value) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 10};

    Expr* value = expr_literal_number(arena, 42, span);
    Stmt* ret = stmt_return(arena, value, span);

    ASSERT_EQ(ret->type, STMT_RETURN);
    StmtReturn* r = (StmtReturn*)ret;
    ASSERT_EQ(r->value, value);

    arena_free(arena);
}

TEST(stmt_return_bare) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 7};

    Stmt* ret = stmt_return(arena, NULL, span);

    StmtReturn* r = (StmtReturn*)ret;
    ASSERT_NULL(r->value);

    arena_free(arena);
}

TEST(stmt_break_continue) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 6};

    Stmt* brk = stmt_break(arena, span);
    Stmt* cont = stmt_continue(arena, span);

    ASSERT_EQ(brk->type, STMT_BREAK);
    ASSERT_EQ(cont->type, STMT_CONTINUE);

    arena_free(arena);
}

TEST(stmt_function_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 5, .end_column = 1};

    Token name = make_test_token(TOKEN_IDENTIFIER, "myFunc", 1, 10);
    Token* params = arena_alloc(arena, sizeof(Token) * 2);
    params[0] = make_test_token(TOKEN_IDENTIFIER, "a", 1, 17);
    params[1] = make_test_token(TOKEN_IDENTIFIER, "b", 1, 20);

    Stmt** body_stmts = arena_alloc(arena, sizeof(Stmt*));
    body_stmts[0] = stmt_return(arena, expr_literal_number(arena, 42, span), span);
    Stmt* body = stmt_block(arena, body_stmts, 1, span);

    Stmt* func = stmt_function(arena, name, params, 2, body, span);

    ASSERT_EQ(func->type, STMT_FUNCTION);
    StmtFunction* f = (StmtFunction*)func;
    ASSERT_EQ(f->param_count, 2);
    ASSERT_EQ(f->body, body);

    arena_free(arena);
}

// ============================================================================
// Visitor Pattern Tests
// ============================================================================

typedef struct {
    int literal_count;
    int binary_count;
    int identifier_count;
} ExprCounterCtx;

static void count_literal_null(Expr* expr, void* ctx) {
    (void)expr;
    ((ExprCounterCtx*)ctx)->literal_count++;
}

static void count_literal_bool(Expr* expr, void* ctx) {
    (void)expr;
    ((ExprCounterCtx*)ctx)->literal_count++;
}

static void count_literal_number(Expr* expr, void* ctx) {
    (void)expr;
    ((ExprCounterCtx*)ctx)->literal_count++;
}

static void count_literal_string(Expr* expr, void* ctx) {
    (void)expr;
    ((ExprCounterCtx*)ctx)->literal_count++;
}

static v