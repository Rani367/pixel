#include "../test_framework.h"
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

TEST(expr_get_constructor) {
    Arena* arena = arena_new(0);
    Token obj_name = make_test_token(TOKEN_IDENTIFIER, "obj", 1, 1);
    Token prop_name = make_test_token(TOKEN_IDENTIFIER, "field", 1, 5);

    Expr* object = expr_identifier(arena, obj_name);
    Expr* get = expr_get(arena, object, prop_name);

    ASSERT_EQ(get->type, EXPR_GET);
    ExprGet* g = (ExprGet*)get;
    ASSERT_EQ(g->object, object);
    ASSERT_EQ(g->name.length, 5);

    arena_free(arena);
}

TEST(expr_set_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 10};
    Token obj_name = make_test_token(TOKEN_IDENTIFIER, "obj", 1, 1);
    Token prop_name = make_test_token(TOKEN_IDENTIFIER, "field", 1, 5);

    Expr* object = expr_identifier(arena, obj_name);
    Expr* value = expr_literal_number(arena, 42, span);
    Expr* set = expr_set(arena, object, prop_name, value);

    ASSERT_EQ(set->type, EXPR_SET);
    ExprSet* s = (ExprSet*)set;
    ASSERT_EQ(s->object, object);
    ASSERT_EQ(s->value, value);

    arena_free(arena);
}

TEST(expr_index_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 10};
    Token arr_name = make_test_token(TOKEN_IDENTIFIER, "arr", 1, 1);

    Expr* object = expr_identifier(arena, arr_name);
    Expr* index = expr_literal_number(arena, 0, span);
    Expr* idx = expr_index(arena, object, index, span);

    ASSERT_EQ(idx->type, EXPR_INDEX);
    ExprIndex* i = (ExprIndex*)idx;
    ASSERT_EQ(i->object, object);
    ASSERT_EQ(i->index, index);

    arena_free(arena);
}

TEST(expr_index_set_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 10};
    Token arr_name = make_test_token(TOKEN_IDENTIFIER, "arr", 1, 1);

    Expr* object = expr_identifier(arena, arr_name);
    Expr* index = expr_literal_number(arena, 0, span);
    Expr* value = expr_literal_number(arena, 99, span);
    Expr* idx_set = expr_index_set(arena, object, index, value);

    ASSERT_EQ(idx_set->type, EXPR_INDEX_SET);
    ExprIndexSet* is = (ExprIndexSet*)idx_set;
    ASSERT_EQ(is->object, object);
    ASSERT_EQ(is->index, index);
    ASSERT_EQ(is->value, value);

    arena_free(arena);
}

TEST(expr_function_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 5, .end_column = 1};

    Token* params = arena_alloc(arena, sizeof(Token) * 2);
    params[0] = make_test_token(TOKEN_IDENTIFIER, "a", 1, 5);
    params[1] = make_test_token(TOKEN_IDENTIFIER, "b", 1, 8);

    Stmt** body_stmts = arena_alloc(arena, sizeof(Stmt*));
    body_stmts[0] = stmt_return(arena, expr_literal_number(arena, 42, span), span);
    Stmt* body = stmt_block(arena, body_stmts, 1, span);

    Expr* func = expr_function(arena, params, 2, body, span);

    ASSERT_EQ(func->type, EXPR_FUNCTION);
    ExprFunction* f = (ExprFunction*)func;
    ASSERT_EQ(f->param_count, 2);
    ASSERT_EQ(f->body, body);

    arena_free(arena);
}

TEST(expr_postfix_constructor) {
    Arena* arena = arena_new(0);
    Token id = make_test_token(TOKEN_IDENTIFIER, "x", 1, 1);
    Token op = make_test_token(TOKEN_PLUS_PLUS, "++", 1, 2);

    Expr* operand = expr_identifier(arena, id);
    Expr* postfix = expr_postfix(arena, operand, op);

    ASSERT_EQ(postfix->type, EXPR_POSTFIX);
    ExprPostfix* p = (ExprPostfix*)postfix;
    ASSERT_EQ(p->operand, operand);
    ASSERT_EQ(p->op.type, TOKEN_PLUS_PLUS);

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
    Expr* else_expr = expr_literal_number(arena, 2, span);
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

TEST(stmt_assignment_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 10};

    Token name = make_test_token(TOKEN_IDENTIFIER, "x", 1, 1);
    Expr* target = expr_identifier(arena, name);
    Expr* value = expr_literal_number(arena, 42, span);
    Stmt* assign = stmt_assignment(arena, target, value);

    ASSERT_EQ(assign->type, STMT_ASSIGNMENT);
    StmtAssignment* a = (StmtAssignment*)assign;
    ASSERT_EQ(a->target, target);
    ASSERT_EQ(a->value, value);

    arena_free(arena);
}

TEST(stmt_for_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 3, .end_column = 1};

    Token var = make_test_token(TOKEN_IDENTIFIER, "i", 1, 5);
    Token arr_name = make_test_token(TOKEN_IDENTIFIER, "items", 1, 10);
    Expr* iterable = expr_identifier(arena, arr_name);
    Stmt* body = stmt_expression(arena, expr_literal_number(arena, 1, span));

    Stmt* for_stmt = stmt_for(arena, var, iterable, body, span);

    ASSERT_EQ(for_stmt->type, STMT_FOR);
    StmtFor* f = (StmtFor*)for_stmt;
    ASSERT_EQ(f->iterable, iterable);
    ASSERT_EQ(f->body, body);
    ASSERT_EQ(f->name.length, 1);

    arena_free(arena);
}

TEST(stmt_struct_constructor) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 5, .end_column = 1};

    Token name = make_test_token(TOKEN_IDENTIFIER, "Point", 1, 8);
    Token* fields = arena_alloc(arena, sizeof(Token) * 2);
    fields[0] = make_test_token(TOKEN_IDENTIFIER, "x", 2, 5);
    fields[1] = make_test_token(TOKEN_IDENTIFIER, "y", 2, 8);

    // No methods for this test
    Stmt* struct_stmt = stmt_struct(arena, name, fields, 2, NULL, 0, span);

    ASSERT_EQ(struct_stmt->type, STMT_STRUCT);
    StmtStruct* s = (StmtStruct*)struct_stmt;
    ASSERT_EQ(s->field_count, 2);
    ASSERT_EQ(s->method_count, 0);
    ASSERT_EQ(s->name.length, 5);

    arena_free(arena);
}

// ============================================================================
// AST Print Tests
// ============================================================================

TEST(ast_print_expr_null_safe) {
    // Should not crash with NULL expr
    ast_print_expr(NULL, 0);
}

TEST(ast_print_stmt_null_safe) {
    // Should not crash with NULL stmt
    ast_print_stmt(NULL, 0);
}

TEST(ast_print_expr_all_types) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 10};

    // Test printing various expression types (should not crash)
    Expr* null_lit = expr_literal_null(arena, span);
    ast_print_expr(null_lit, 0);

    Expr* bool_lit = expr_literal_bool(arena, true, span);
    ast_print_expr(bool_lit, 0);

    Expr* num_lit = expr_literal_number(arena, 42.5, span);
    ast_print_expr(num_lit, 0);

    Expr* str_lit = expr_literal_string(arena, "test", 4, span);
    ast_print_expr(str_lit, 0);

    Token id_tok = make_test_token(TOKEN_IDENTIFIER, "myVar", 1, 1);
    Expr* id = expr_identifier(arena, id_tok);
    ast_print_expr(id, 0);

    Expr* unary = expr_unary(arena, TOKEN_MINUS, num_lit, span);
    ast_print_expr(unary, 0);

    Expr* left = expr_literal_number(arena, 1, span);
    Expr* right = expr_literal_number(arena, 2, span);
    Expr* binary = expr_binary(arena, left, TOKEN_PLUS, right);
    ast_print_expr(binary, 0);

    // Vec2
    Expr* x = expr_literal_number(arena, 10, span);
    Expr* y = expr_literal_number(arena, 20, span);
    Expr* vec = expr_vec2(arena, x, y, span);
    ast_print_expr(vec, 0);

    // List
    Expr** elements = arena_alloc(arena, sizeof(Expr*));
    elements[0] = num_lit;
    Expr* list = expr_list(arena, elements, 1, span);
    ast_print_expr(list, 0);

    // Call
    Expr* callee = expr_identifier(arena, id_tok);
    Expr** args = arena_alloc(arena, sizeof(Expr*));
    args[0] = num_lit;
    Expr* call = expr_call(arena, callee, args, 1, span);
    ast_print_expr(call, 0);

    arena_free(arena);
}

TEST(ast_print_stmt_all_types) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 5, .end_column = 1};

    // Expression statement
    Expr* num = expr_literal_number(arena, 42, span);
    Stmt* expr_stmt = stmt_expression(arena, num);
    ast_print_stmt(expr_stmt, 0);

    // Block
    Stmt** stmts = arena_alloc(arena, sizeof(Stmt*));
    stmts[0] = expr_stmt;
    Stmt* block = stmt_block(arena, stmts, 1, span);
    ast_print_stmt(block, 0);

    // If
    Expr* cond = expr_literal_bool(arena, true, span);
    Stmt* if_stmt = stmt_if(arena, cond, expr_stmt, NULL, span);
    ast_print_stmt(if_stmt, 0);

    // While
    Stmt* while_stmt = stmt_while(arena, cond, expr_stmt, span);
    ast_print_stmt(while_stmt, 0);

    // Return
    Stmt* ret = stmt_return(arena, num, span);
    ast_print_stmt(ret, 0);

    // Break/Continue
    Stmt* brk = stmt_break(arena, span);
    Stmt* cont = stmt_continue(arena, span);
    ast_print_stmt(brk, 0);
    ast_print_stmt(cont, 0);

    // Function
    Token fn_name = make_test_token(TOKEN_IDENTIFIER, "test", 1, 1);
    Stmt* func = stmt_function(arena, fn_name, NULL, 0, block, span);
    ast_print_stmt(func, 0);

    // Assignment
    Token var = make_test_token(TOKEN_IDENTIFIER, "x", 1, 1);
    Expr* target = expr_identifier(arena, var);
    Stmt* assign = stmt_assignment(arena, target, num);
    ast_print_stmt(assign, 0);

    // For
    Token iter_var = make_test_token(TOKEN_IDENTIFIER, "i", 1, 1);
    Token arr = make_test_token(TOKEN_IDENTIFIER, "items", 1, 5);
    Stmt* for_stmt = stmt_for(arena, iter_var, expr_identifier(arena, arr), expr_stmt, span);
    ast_print_stmt(for_stmt, 0);

    // Struct
    Token struct_name = make_test_token(TOKEN_IDENTIFIER, "Point", 1, 1);
    Stmt* struct_stmt = stmt_struct(arena, struct_name, NULL, 0, NULL, 0, span);
    ast_print_stmt(struct_stmt, 0);

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

static void count_binary(Expr* expr, void* ctx) {
    (void)expr;
    ((ExprCounterCtx*)ctx)->binary_count++;
}

static void count_identifier(Expr* expr, void* ctx) {
    (void)expr;
    ((ExprCounterCtx*)ctx)->identifier_count++;
}

TEST(visitor_expr_accept) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 5};

    ExprCounterCtx ctx = {0};
    ExprVisitor visitor = {
        .visit_literal_null = count_literal_null,
        .visit_literal_bool = count_literal_bool,
        .visit_literal_number = count_literal_number,
        .visit_literal_string = count_literal_string,
        .visit_binary = count_binary,
        .visit_identifier = count_identifier,
        .context = &ctx,
    };

    Expr* num = expr_literal_number(arena, 42, span);
    expr_accept(num, &visitor);
    ASSERT_EQ(ctx.literal_count, 1);

    Token tok = make_test_token(TOKEN_IDENTIFIER, "x", 1, 1);
    Expr* id = expr_identifier(arena, tok);
    expr_accept(id, &visitor);
    ASSERT_EQ(ctx.identifier_count, 1);

    Expr* left = expr_literal_number(arena, 1, span);
    Expr* right = expr_literal_number(arena, 2, span);
    Expr* binary = expr_binary(arena, left, TOKEN_PLUS, right);
    expr_accept(binary, &visitor);
    ASSERT_EQ(ctx.binary_count, 1);

    arena_free(arena);
}

TEST(visitor_null_safety) {
    ExprVisitor visitor = {0};

    // Should not crash with NULL expr
    expr_accept(NULL, &visitor);

    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 5};
    Expr* expr = expr_literal_number(arena, 42, span);

    // Should not crash with NULL visitor
    expr_accept(expr, NULL);

    arena_free(arena);
}

typedef struct {
    int stmt_count;
} StmtCounterCtx;

static void count_expression_stmt(Stmt* stmt, void* ctx) {
    (void)stmt;
    ((StmtCounterCtx*)ctx)->stmt_count++;
}

static void count_block_stmt(Stmt* stmt, void* ctx) {
    (void)stmt;
    ((StmtCounterCtx*)ctx)->stmt_count++;
}

TEST(visitor_stmt_accept) {
    Arena* arena = arena_new(0);
    Span span = {.start_line = 1, .start_column = 1, .end_line = 1, .end_column = 5};

    StmtCounterCtx ctx = {0};
    StmtVisitor visitor = {
        .visit_expression = count_expression_stmt,
        .visit_block = count_block_stmt,
        .context = &ctx,
    };

    Expr* expr = expr_literal_number(arena, 42, span);
    Stmt* stmt = stmt_expression(arena, expr);
    stmt_accept(stmt, &visitor);
    ASSERT_EQ(ctx.stmt_count, 1);

    Stmt** stmts = arena_alloc(arena, sizeof(Stmt*));
    stmts[0] = stmt;
    Stmt* block = stmt_block(arena, stmts, 1, span);
    stmt_accept(block, &visitor);
    ASSERT_EQ(ctx.stmt_count, 2);

    arena_free(arena);
}

// ============================================================================
// Type Name Tests
// ============================================================================

TEST(expr_type_name_valid) {
    ASSERT_STR_EQ(expr_type_name(EXPR_LITERAL_NULL), "LiteralNull");
    ASSERT_STR_EQ(expr_type_name(EXPR_LITERAL_BOOL), "LiteralBool");
    ASSERT_STR_EQ(expr_type_name(EXPR_LITERAL_NUMBER), "LiteralNumber");
    ASSERT_STR_EQ(expr_type_name(EXPR_LITERAL_STRING), "LiteralString");
    ASSERT_STR_EQ(expr_type_name(EXPR_IDENTIFIER), "Identifier");
    ASSERT_STR_EQ(expr_type_name(EXPR_UNARY), "Unary");
    ASSERT_STR_EQ(expr_type_name(EXPR_BINARY), "Binary");
    ASSERT_STR_EQ(expr_type_name(EXPR_CALL), "Call");
    ASSERT_STR_EQ(expr_type_name(EXPR_VEC2), "Vec2");
}

TEST(expr_type_name_invalid) {
    ASSERT_STR_EQ(expr_type_name(EXPR_COUNT), "Unknown");
    ASSERT_STR_EQ(expr_type_name((ExprType)-1), "Unknown");
    ASSERT_STR_EQ(expr_type_name((ExprType)999), "Unknown");
}

TEST(stmt_type_name_valid) {
    ASSERT_STR_EQ(stmt_type_name(STMT_EXPRESSION), "Expression");
    ASSERT_STR_EQ(stmt_type_name(STMT_ASSIGNMENT), "Assignment");
    ASSERT_STR_EQ(stmt_type_name(STMT_BLOCK), "Block");
    ASSERT_STR_EQ(stmt_type_name(STMT_IF), "If");
    ASSERT_STR_EQ(stmt_type_name(STMT_WHILE), "While");
    ASSERT_STR_EQ(stmt_type_name(STMT_FOR), "For");
    ASSERT_STR_EQ(stmt_type_name(STMT_RETURN), "Return");
    ASSERT_STR_EQ(stmt_type_name(STMT_BREAK), "Break");
    ASSERT_STR_EQ(stmt_type_name(STMT_CONTINUE), "Continue");
    ASSERT_STR_EQ(stmt_type_name(STMT_FUNCTION), "Function");
    ASSERT_STR_EQ(stmt_type_name(STMT_STRUCT), "Struct");
}

TEST(stmt_type_name_invalid) {
    ASSERT_STR_EQ(stmt_type_name(STMT_COUNT), "Unknown");
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("AST - Span Utilities");
    RUN_TEST(span_from_token_basic);
    RUN_TEST(span_merge_basic);

    TEST_SUITE("AST - Expression Constructors");
    RUN_TEST(expr_literal_null_constructor);
    RUN_TEST(expr_literal_bool_constructor);
    RUN_TEST(expr_literal_number_constructor);
    RUN_TEST(expr_literal_string_constructor);
    RUN_TEST(expr_identifier_constructor);
    RUN_TEST(expr_unary_constructor);
    RUN_TEST(expr_binary_constructor);
    RUN_TEST(expr_call_constructor);
    RUN_TEST(expr_list_constructor);
    RUN_TEST(expr_vec2_constructor);
    RUN_TEST(expr_get_constructor);
    RUN_TEST(expr_set_constructor);
    RUN_TEST(expr_index_constructor);
    RUN_TEST(expr_index_set_constructor);
    RUN_TEST(expr_function_constructor);
    RUN_TEST(expr_postfix_constructor);

    TEST_SUITE("AST - Statement Constructors");
    RUN_TEST(stmt_expression_constructor);
    RUN_TEST(stmt_block_constructor);
    RUN_TEST(stmt_if_constructor);
    RUN_TEST(stmt_if_no_else);
    RUN_TEST(stmt_while_constructor);
    RUN_TEST(stmt_return_with_value);
    RUN_TEST(stmt_return_bare);
    RUN_TEST(stmt_break_continue);
    RUN_TEST(stmt_function_constructor);
    RUN_TEST(stmt_assignment_constructor);
    RUN_TEST(stmt_for_constructor);
    RUN_TEST(stmt_struct_constructor);

    TEST_SUITE("AST - Print Functions");
    RUN_TEST(ast_print_expr_null_safe);
    RUN_TEST(ast_print_stmt_null_safe);
    RUN_TEST(ast_print_expr_all_types);
    RUN_TEST(ast_print_stmt_all_types);

    TEST_SUITE("AST - Visitor Pattern");
    RUN_TEST(visitor_expr_accept);
    RUN_TEST(visitor_null_safety);
    RUN_TEST(visitor_stmt_accept);

    TEST_SUITE("AST - Type Names");
    RUN_TEST(expr_type_name_valid);
    RUN_TEST(expr_type_name_invalid);
    RUN_TEST(stmt_type_name_valid);
    RUN_TEST(stmt_type_name_invalid);

    TEST_SUMMARY();
}
