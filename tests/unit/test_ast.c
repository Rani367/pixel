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

static v#include "../test_framework.h"
#include "compiler/ast.h"
#include "core/arena.h"

// Helper to create a dummy token
static Token make_test_token(TokenType type, const char* text, int line, int col) {
    return token_make(type, text, (int)strlen(text), line, col);
}

// ============================================================================
// Span Tests
// ============================================================================

TEST(span_from_token_basic) {
    Token tok = make_test_token(TOKEN_IDENTIFIER, "foo", 5, 10);
    Span span = span_from_token(tok);

    ASSERT_EQ(span.start_line, 5);
    ASSERT_EQ(span.start_column, 10);
    ASSERT_EQ(span.end_line, 5);
    ASSERT_EQ(span.end_column, 13);  // 10 + 3 (length of "foo")
}

TEST(span_merge_basic) {
    Span a = {.start_line = 1, .start_column = 5, .end_line = 1, .end_column = 10};
    Span b = {.start_line = 3, .start_column = 1, .end_line = 3, .end_column = 15};
    Span merged = span_merge(a, b);

    ASSERT_EQ(merged.start_line, 1);
    ASSERT_EQ(merged.start_column, 5);
    ASSERT_EQ(merged.end_line, 3);
    ASSERT_EQ(merged.end_column, 15);
}

// ============================================================================
// Expression Constructor Tests
// ============================================================================

TEST(expr_literal_null_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 5};

    Expr* expr = expr_literal_null(arena, span);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_LITERAL_NULL);
    ASSERT_EQ(expr->span.start_line, 1);

    arena_free(arena);
}

TEST(expr_literal_bool_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 5};

    Expr* expr_true = expr_literal_bool(arena, true, span);
    Expr* expr_false = expr_literal_bool(arena, false, span);

    ASSERT_EQ(expr_true->type, EXPR_LITERAL_BOOL);
    ASSERT_EQ(((ExprLiteralBool*)expr_true)->value, true);

    ASSERT_EQ(expr_false->type, EXPR_LITERAL_BOOL);
    ASSERT_EQ(((ExprLiteralBool*)expr_false)->value, false);

    arena_free(arena);
}

TEST(expr_literal_number_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 5};

    Expr* expr = expr_literal_number(arena, 42.5, span);

    ASSERT_EQ(expr->type, EXPR_LITERAL_NUMBER);
    ASSERT_EQ(((ExprLiteralNumber*)expr)->value, 42.5);

    arena_free(arena);
}

TEST(expr_literal_string_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 10};

    Expr* expr = expr_literal_string(arena, "hello", 5, span);

    ASSERT_EQ(expr->type, EXPR_LITERAL_STRING);
    ExprLiteralString* str = (ExprLiteralString*)expr;
    ASSERT_EQ(str->length, 5);
    ASSERT_STR_EQ(str->value, "hello");

    arena_free(arena);
}

TEST(expr_identifier_constructor) {
    Arena* arena = arena_new(0);
    Token name = make_test_token(TOKEN_IDENTIFIER, "myVar", 2, 5);

    Expr* expr = expr_identifier(arena, name);

    ASSERT_EQ(expr->type, EXPR_IDENTIFIER);
    ExprIdentifier* id = (ExprIdentifier*)expr;
    ASSERT_EQ(id->name.length, 5);
    ASSERT_EQ(expr->span.start_line, 2);
    ASSERT_EQ(expr->span.start_column, 5);

    arena_free(arena);
}

TEST(expr_unary_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 5};

    Expr* operand = expr_literal_number(arena, 42, span);
    Expr* unary = expr_unary(arena, TOKEN_MINUS, operand, span);

    ASSERT_EQ(unary->type, EXPR_UNARY);
    ExprUnary* u = (ExprUnary*)unary;
    ASSERT_EQ(u->operator, TOKEN_MINUS);
    ASSERT_EQ(u->operand, operand);

    arena_free(arena);
}

TEST(expr_binary_constructor) {
    Arena* arena = arena_new(0);
    Span span1 = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 2};
    Span span2 = {.start_line = 1, .start_column = 5, .end_line = 1, .end_column = 6};

    Expr* left = expr_literal_number(arena, 1, span1);
    Expr* right = expr_literal_number(arena, 2, span2);
    Expr* binary = expr_binary(arena, left, TOKEN_PLUS, right);

    ASSERT_EQ(binary->type, EXPR_BINARY);
    ExprBinary* b = (ExprBinary*)binary;
    ASSERT_EQ(b->operator, TOKEN_PLUS);
    ASSERT_EQ(b->left, left);
    ASSERT_EQ(b->right, right);
    // Span should merge left and right
    ASSERT_EQ(binary->span.start_line, 1);
    ASSERT_EQ(binary->span.start_column, 1);
    ASSERT_EQ(binary->span.end_column, 6);

    arena_free(arena);
}

TEST(expr_call_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 20};

    Token name = make_test_token(TOKEN_IDENTIFIER, "print", 1, 1);
    Expr* callee = expr_identifier(arena, name);

    Expr** args = arena_alloc(arena, sizeof(Expr*) * 2);
    args[0] = expr_literal_number(arena, 1, span);
    args[1] = expr_literal_number(arena, 2, span);

    Expr* call = expr_call(arena, callee, args, 2, span);

    ASSERT_EQ(call->type, EXPR_CALL);
    ExprCall* c = (ExprCall*)call;
    ASSERT_EQ(c->callee, callee);
    ASSERT_EQ(c->arg_count, 2);
    ASSERT_EQ(c->arguments[0]->type, EXPR_LITERAL_NUMBER);
    ASSERT_EQ(c->arguments[1]->type, EXPR_LITERAL_NUMBER);

    arena_free(arena);
}

TEST(expr_list_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 10};

    Expr** elements = arena_alloc(arena, sizeof(Expr*) * 3);
    elements[0] = expr_literal_number(arena, 1, span);
    elements[1] = expr_literal_number(arena, 2, span);
    elements[2] = expr_literal_number(arena, 3, span);

    Expr* list = expr_list(arena, elements, 3, span);

    ASSERT_EQ(list->type, EXPR_LIST);
    ExprList* l = (ExprList*)list;
    ASSERT_EQ(l->count, 3);

    arena_free(arena);
}

TEST(expr_vec2_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 10};

    Expr* x = expr_literal_number(arena, 10, span);
    Expr* y = expr_literal_number(arena, 20, span);
    Expr* vec = expr_vec2(arena, x, y, span);

    ASSERT_EQ(vec->type, EXPR_VEC2);
    ExprVec2* v = (ExprVec2*)vec;
    ASSERT_EQ(v->x, x);
    ASSERT_EQ(v->y, y);

    arena_free(arena);
}

// ============================================================================
// Statement Constructor Tests
// ============================================================================

TEST(stmt_expression_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 5};

    Expr* expr = expr_literal_number(arena, 42, span);
    Stmt* stmt = stmt_expression(arena, expr);

    ASSERT_EQ(stmt->type, STMT_EXPRESSION);
    StmtExpression* s = (StmtExpression*)stmt;
    ASSERT_EQ(s->expression, expr);

    arena_free(arena);
}

TEST(stmt_block_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 5, .end_column = 1};

    Stmt** stmts = arena_alloc(arena, sizeof(Stmt*) * 2);
    Expr* e1 = expr_literal_number(arena, 1, span);
    Expr* e2 = expr_literal_number(arena, 2, span);
    stmts[0] = stmt_expression(arena, e1);
    stmts[1] = stmt_expression(arena, e2);

    Stmt* block = stmt_block(arena, stmts, 2, span);

    ASSERT_EQ(block->type, STMT_BLOCK);
    StmtBlock* b = (StmtBlock*)block;
    ASSERT_EQ(b->count, 2);

    arena_free(arena);
}

TEST(stmt_if_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 5, .end_column = 1};

    Expr* condition = expr_literal_bool(arena, true, span);
    Expr* then_expr = expr_literal_number(arena, 1, span);
    Stmt* then_branch = stmt_expression(arena, then_expr);
    Expr* else_expr = expr_literal_number(ar