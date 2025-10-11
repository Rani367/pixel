===================================================================

int main(void) {
    TEST_SUITE("Parser - Literals");
    RUN_TEST(parse_number);
    RUN_TEST(parse_float);
    RUN_TEST(parse_string);
    RUN_TEST(parse_true);
    RUN_TEST(parse_false);
    RUN_TEST(parse_null);
    RUN_TEST(parse_identifier);

    TEST_SUITE("Parser - Binary Expressions");
    RUN_TEST(parse_binary_add);
    RUN_TEST(parse_binary_precedence);
    RUN_TEST(parse_binary_left_associative);
    RUN_TEST(parse_comparison);
    RUN_TEST(parse_equality);
    RUN_TEST(parse_logical_and);
    RUN_TEST(parse_logical_or);
    RUN_TEST(parse_and_or_precedence);

    TEST_SUITE("Parser - Unary Expressions");
    RUN_TEST(parse_unary_minus);
    RUN_TEST(parse_unary_not);
    RUN_TEST(parse_unary_precedence);

    TEST_SUITE("Parser - Grouping");
    RUN_TEST(parse_grouping);

    TEST_SUITE("Parser - Call Expressions");
    RUN_TEST(parse_call_no_args);
    RUN_TEST(parse_call_with_args);
    RUN_TEST(parse_chained_calls);

    TEST_SUITE("Parser - Property Access");
    RUN_TEST(parse_property_get);
    RUN_TEST(parse_chained_property);
    RUN_TEST(parse_method_call);

    TEST_SUITE("Parser - Index Expressions");
    RUN_TEST(parse_index);
    RUN_TEST(parse_chained_index);

    TEST_SUITE("Parser - List Expressions");
    RUN_TEST(parse_empty_list);
    RUN_TEST(parse_list);

    TEST_SUITE("Parser - Function Expressions");
    RUN_TEST(parse_function_expr);

    TEST_SUITE("Parser - Statements");
    RUN_TEST(parse_assignment);
    RUN_TEST(parse_compound_assignment);
    RUN_TEST(parse_if_statement);
    RUN_TEST(parse_if_else);
    RUN_TEST(parse_while_statement);
    RUN_TEST(parse_for_statement);
    RUN_TEST(parse_return_with_value);
    RUN_TEST(parse_return_bare);
    RUN_TEST(parse_break_continue);

    TEST_SUITE("Parser - Declarations");
    RUN_TEST(parse_function_decl);
    RUN_TEST(parse_function_no_params);
    RUN_TEST(parse_struct_decl);

    TEST_SUITE("Parser - Error Handling");
    RUN_TEST(parse_error_missing_paren);
    RUN_TEST(parse_error_recovery);
    RUN_TEST(parse_empty_input);

    TEST_SUITE("Parser - Complex Cases");
    RUN_TEST(parse_complex_expression);
    RUN_TEST(parse_multiple_statements);
    RUN_TEST(parse_nested_function);

    TEST_SUMMARY();
}
na = arena_new(0);
    Parser parser;
    parser_init(&parser, "(1 + 2", arena);
    int count;
    Stmt** stmts = parser_parse(&parser, &count);
    (void)stmts;  // May be NULL on error

    ASSERT(parser_had_error(&parser));

    arena_free(arena);
}

TEST(parse_error_recovery) {
    Arena* arena = arena_new(0);
    Parser parser;
    // Missing right side of assignment, then valid statement
    parser_init(&parser, "x = \nfunction foo() { return 1 }", arena);
    int count;
    Stmt** stmts = parser_parse(&parser, &count);
    (void)stmts;

    ASSERT(parser_had_error(&parser));
    // Should still parse the function after recovery
    ASSERT(count >= 1);

    arena_free(arena);
}

TEST(parse_empty_input) {
    Arena* arena = arena_new(0);
    Parser parser;
    parser_init(&parser, "", arena);
    int count;
    Stmt** stmts = parser_parse(&parser, &count);

    ASSERT(!parser_had_error(&parser));
    ASSERT_EQ(count, 0);
    ASSERT_NOT_NULL(stmts);

    arena_free(arena);
}

// ============================================================================
// Complex Tests
// ============================================================================

TEST(parse_complex_expression) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("player.x + speed * dt", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_BINARY);
    ExprBinary* add = (ExprBinary*)expr;
    ASSERT_EQ(add->operator, TOKEN_PLUS);
    ASSERT_EQ(add->left->type, EXPR_GET);
    ASSERT_EQ(add->right->type, EXPR_BINARY);

    arena_free(arena);
}

TEST(parse_multiple_statements) {
    Arena* arena = arena_new(0);
    Parser parser;
    parser_init(&parser, "x = 1\ny = 2\nz = x + y", arena);
    int count;
    Stmt** stmts = parser_parse(&parser, &count);
    (void)stmts;

    ASSERT(!parser_had_error(&parser));
    ASSERT_EQ(count, 3);

    arena_free(arena);
}

TEST(parse_nested_function) {
    Arena* arena = arena_new(0);
    Parser parser;
    parser_init(&parser, "function outer() { function inner() { return 1 } return inner }", arena);
    int count;
    Stmt** stmts = parser_parse(&parser, &count);

    ASSERT(!parser_had_error(&parser));
    ASSERT_EQ(count, 1);
    ASSERT_EQ(stmts[0]->type, STMT_FUNCTION);

    arena_free(arena);
}

// ============================================================================
// Main
// =========#include "../test_framework.h"
#include "compiler/parser.h"
#include "core/arena.h"

// Helper to parse a single expression statement
static Expr* parse_expr(const char* source, Arena* arena) {
    Parser parser;
    parser_init(&parser, source, arena);
    int count;
    Stmt** stmts = parser_parse(&parser, &count);
    if (parser_had_error(&parser) || count == 0) return NULL;
    if (stmts[0]->type != STMT_EXPRESSION) return NULL;
    return ((StmtExpression*)stmts[0])->expression;
}

// ============================================================================
// Literal Tests
// ============================================================================

TEST(parse_number) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("42", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_LITERAL_NUMBER);
    ASSERT_EQ(((ExprLiteralNumber*)expr)->value, 42.0);

    arena_free(arena);
}

TEST(parse_float) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("3.14", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_LITERAL_NUMBER);
    // Approximate float comparison
    double val = ((ExprLiteralNumber*)expr)->value;
    ASSERT(val > 3.13 && val < 3.15);

    arena_free(arena);
}

TEST(parse_string) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("\"hello\"", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_LITERAL_STRING);
    ExprLiteralString* str = (ExprLiteralString*)expr;
    ASSERT_EQ(str->length, 5);
    ASSERT_STR_EQ(str->value, "hello");

    arena_free(arena);
}

TEST(parse_true) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("true", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_LITERAL_BOOL);
    ASSERT_EQ(((ExprLiteralBool*)expr)->value, true);

    arena_free(arena);
}

TEST(parse_false) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("false", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_LITERAL_BOOL);
    ASSERT_EQ(((ExprLiteralBool*)expr)->value, false);

    arena_free(arena);
}

TEST(parse_null) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("null", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_LITERAL_NULL);

    arena_free(arena);
}

TEST(parse_identifier) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("myVar", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_IDENTIFIER);
    ExprIdentifier* id = (ExprIdentifier*)expr;
    ASSERT_EQ(id->name.length, 5);

    arena_free(arena);
}

// ============================================================================
// Binary Expression Tests
// ============================================================================

TEST(parse_binary_add) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("1 + 2", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_BINARY);
    ExprBinary* bin = (ExprBinary*)expr;
    ASSERT_EQ(bin->operator, TOKEN_PLUS);
    ASSERT_EQ(bin->left->type, EXPR_LITERAL_NUMBER);
    ASSERT_EQ(bin->right->type, EXPR_LITERAL_NUMBER);

    arena_free(arena);
}

TEST(parse_binary_precedence) {
    Arena* arena = arena_new(0);
    // 1 + 2 * 3 should parse as 1 + (2 * 3)
    Expr* expr = parse_expr("1 + 2 * 3", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_BINARY);
    ExprBinary* add = (ExprBinary*)expr;
    ASSERT_EQ(add->operator, TOKEN_PLUS);
    ASSERT_EQ(add->left->type, EXPR_LITERAL_NUMBER);
    ASSERT_EQ(add->right->type, EXPR_BINARY);

    ExprBinary* mul = (ExprBinary*)add->right;
    ASSERT_EQ(mul->operator, TOKEN_STAR);

    arena_free(arena);
}

TEST(parse_binary_left_associative) {
    Arena* arena = arena_new(0);
    // 1 - 2 - 3 should parse as (1 - 2) - 3
    Expr* expr = parse_expr("1 - 2 - 3", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_BINARY);
    ExprBinary* outer = (ExprBinary*)expr;
    ASSERT_EQ(outer->operator, TOKEN_MINUS);
    ASSERT_EQ(outer->left->type, EXPR_BINARY);
    ASSERT_EQ(outer->right->type, EXPR_LITERAL_NUMBER);

    ExprBinary* inner = (ExprBinary*)outer->left;
    ASSERT_EQ(inner->operator, TOKEN_MINUS);

    arena_free(arena);
}

TEST(parse_comparison) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("a < b", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_BINARY);
    ExprBinary* bin = (ExprBinary*)expr;
    ASSERT_EQ(bin->operator, TOKEN_LESS);

    arena_free(arena);
}

TEST(parse_equality) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("x == y", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_BINARY);
    ExprBinary* bin = (ExprBinary*)expr;
    ASSERT_EQ(bin->operator, TOKEN_EQUAL_EQUAL);

    arena_free(arena);
}

TEST(parse_logical_and) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("a and b", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_BINARY);
    ExprBinary* bin = (ExprBinary*)expr;
    ASSERT_EQ(bin->operator, TOKEN_AND);

    arena_free(arena);
}

TEST(parse_logical_or) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("a or b", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_BINARY);
    ExprBinary* bin = (ExprBinary*)expr;
    ASSERT_EQ(bin->operator, TOKEN_OR);

    arena_free(arena);
}

TEST(parse_and_or_precedence) {
    Arena* arena = arena_new(0);
    // a or b and c should parse as a or (b and c)
    Expr* expr = parse_expr("a or b and c", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_BINARY);
    ExprBinary* or_expr = (ExprBinary*)expr;
    ASSERT_EQ(or_expr->operator, TOKEN_OR);
    ASSERT_EQ(or_expr->right->type, EXPR_BINARY);

    ExprBinary* and_expr = (ExprBinary*)or_expr->right;
    ASSERT_EQ(and_expr->operator, TOKEN_AND);

    arena_free(arena);
}

// ============================================================================
// Unary Expression Tests
// ============================================================================

TEST(parse_unary_minus) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("-42", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_UNARY);
    ExprUnary* unary = (ExprUnary*)expr;
    ASSERT_EQ(unary->operator, TOKEN_MINUS);
    ASSERT_EQ(unary->operand->type, EXPR_LITERAL_NUMBER);

    arena_free(arena);
}

TEST(parse_unary_not) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("not true", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_UNARY);
    ExprUnary* unary = (ExprUnary*)expr;
    ASSERT_EQ(unary->operator, TOKEN_NOT);
    ASSERT_EQ(unary->operand->type, EXPR_LITERAL_BOOL);

    arena_free(arena);
}

TEST(parse_unary_precedence) {
    Arena* arena = arena_new(0);
    // -a + b should parse as (-a) + b
    Expr* expr = parse_expr("-a + b", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_BINARY);
    ExprBinary* bin = (ExprBinary*)expr;
    ASSERT_EQ(bin->left->type, EXPR_UNARY);

    arena_free(arena);
}

// ============================================================================
// Grouping Tests
// ============================================================================

TEST(parse_grouping) {
    Arena* arena = arena_new(0);
    // (1 + 2) * 3 should parse multiplication at top
    Expr* expr = parse_expr("(1 + 2) * 3", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_BINARY);
    ExprBinary* mul = (ExprBinary*)expr;
    ASSERT_EQ(mul->operator, TOKEN_STAR);
    ASSERT_EQ(mul->left->type, EXPR_BINARY);  // The grouped addition

    ExprBinary* add = (ExprBinary*)mul->left;
    ASSERT_EQ(add->operator, TOKEN_PLUS);

    arena_free(arena);
}

// ============================================================================
// Call Expression Tests
// ============================================================================

TEST(parse_call_no_args) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("foo()", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_CALL);
    ExprCall* call = (ExprCall*)expr;
    ASSERT_EQ(call->callee->type, EXPR_IDENTIFIER);
    ASSERT_EQ(call->arg_count, 0);

    arena_free(arena);
}

TEST(parse_call_with_args) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("print(1, 2, 3)", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_CALL);
    ExprCall* call = (ExprCall*)expr;
    ASSERT_EQ(call->arg_count, 3);

    arena_free(arena);
}

TEST(parse_chained_calls) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("a()()", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_CALL);
    ExprCall* outer = (ExprCall*)expr;
    ASSERT_EQ(outer->callee->type, EXPR_CALL);

    arena_free(arena);
}

// ============================================================================
// Property Access Tests
// ============================================================================

TEST(parse_property_get) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("obj.field", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_GET);
    ExprGet* get = (ExprGet*)expr;
    ASSERT_EQ(get->object->type, EXPR_IDENTIFIER);
    ASSERT_EQ(get->name.length, 5);

    arena_free(arena);
}

TEST(parse_chained_property) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("a.b.c", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_GET);
    ExprGet* outer = (ExprGet*)expr;
    ASSERT_EQ(outer->object->type, EXPR_GET);

    arena_free(arena);
}

TEST(parse_method_call) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("obj.method(1, 2)", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_CALL);
    ExprCall* call = (ExprCall*)expr;
    ASSERT_EQ(call->callee->type, EXPR_GET);
    ASSERT_EQ(call->arg_count, 2);

    arena_free(arena);
}

// ============================================================================
// Index Expression Tests
// ============================================================================

TEST(parse_index) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("arr[0]", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_INDEX);
    ExprIndex* idx = (ExprIndex*)expr;
    ASSERT_EQ(idx->object->type, EXPR_IDENTIFIER);
    ASSERT_EQ(idx->index->type, EXPR_LITERAL_NUMBER);

    arena_free(arena);
}

TEST(parse_chained_index) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("a[0][1]", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_INDEX);
    ExprIndex* outer = (ExprIndex*)expr;
    ASSERT_EQ(outer->object->type, EXPR_INDEX);

    arena_free(arena);
}

// ============================================================================
// List Expression Tests
// ============================================================================

TEST(parse_empty_list) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("[]", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_LIST);
    ExprList* list = (ExprList*)expr;
    ASSERT_EQ(list->count, 0);

    arena_free(arena);
}

TEST(parse_list) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("[1, 2, 3]", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_LIST);
    ExprList* list = (ExprList*)expr;
    ASSERT_EQ(list->count, 3);

    arena_free(arena);
}

// ============================================================================
// Function Expression Tests
// ============================================================================

TEST(parse_function_expr) {
    Arena* arena = arena_new(0);
    Expr* expr = parse_expr("function(x, y) { return x + y }", arena);

    ASSERT_NOT_NULL(expr);
    ASSERT_EQ(expr->type, EXPR_FUNCTION);
    ExprFunction* fn = (ExprFunction*)expr;
    ASSERT_EQ(fn->param_count, 2);
    ASSERT_NOT_NULL(fn->body);
    ASSERT_EQ(fn->body->type, STMT_BLOCK);

    arena_free(arena);
}

// ============================================================================
// Statement Tests
// ============================================================================

TEST(parse_assignment) {
    Arena* arena = arena_new(0);
    Parser parser;
    parser_init(&parser, "x = 42", arena);
    int count;
    Stmt** stmts = parser_parse(&parser, &count);

    ASSERT(!parser_had_error(&parser));
    ASSERT_EQ(count, 1);
    ASSERT_EQ(stmts[0]->type, STMT_ASSIGNMENT);
    StmtAssignment* assign = (StmtAssignment*)stmts[0];
    ASSERT_EQ(assign->target->type, EXPR_IDENTIFIER);
    ASSERT_EQ(assign->value->type, EXPR_LITERAL_NUMBER);

    arena_free(arena);
}

TEST(parse_compound_assignment) {
    Arena* arena = arena_new(0);
    Parser parser;
    parser_init(&parser, "x += 5", arena);
    int count;
    Stmt** stmts = parser_parse(&parser, &count);

    ASSERT(!parser_had_error(&parser));
    ASSERT_EQ(count, 1);
    ASSERT_EQ(stmts[0]->type, STMT_ASSIGNMENT);
    StmtAssignment* assign = (StmtAssignment*)stmts[0];
    // Value should be a binary expression: x + 5
    ASSERT_EQ(assign->value->type, EXPR_BINARY);

    arena_free(arena);
}

TEST(parse_if_statement) {
    Arena* arena = arena_new(0);
    Parser parser;
    parser_init(&parser, "if true { x = 1 }", arena);
    int count;
    Stmt** stmts = parser_parse(&parser, &count);

    ASSERT(!parser_had_error(&parser));
    ASSERT_EQ(count, 1);
    ASSERT_EQ(stmts[0]->type, STMT_IF);
    StmtIf* if_stmt = (StmtIf*)stmts[0];
    ASSERT_NOT_NULL(if_stmt->condition);
    ASSERT_NOT_NULL(if_stmt->then_branch);
    ASSERT_NULL(if_stmt->else_branch);

    arena_free(arena);
}

TEST(parse_if_else) {
    Arena* arena = arena_new(0);
    Parser parser;
    parser_init(&parser, "if x { a = 1 } else { b = 2 }", arena);
    int count;
    Stmt** stmts = parser_parse(&parser, &count);

    ASSERT(!parser_had_error(&parser));
    ASSERT_EQ(count, 1);
    ASSERT_EQ(stmts[0]->type, STMT_IF);
    StmtIf* if_stmt = (StmtIf*)stmts[0];
    ASSERT_NOT_NULL(if_stmt->else_branch);

    arena_free(arena);
}

TEST(parse_while_statement) {
    Arena* arena = arena_new(0);
    Parser parser;
    parser_init(&parser, "while running { update() }", arena);
    int count;
    Stmt** stmts = parser_parse(&parser, &count);

    ASSERT(!parser_had_error(&parser));
    ASSERT_EQ(count, 1);
    ASSERT_EQ(stmts[0]->type, STMT_WHILE);
    StmtWhile* while_stmt = (StmtWhile*)stmts[0];
    ASSERT_NOT_NULL(while_stmt->condition);
    ASSERT_NOT_NULL(while_stmt->body);

    arena_free(arena);
}

TEST(parse_for_statement) {
    Arena* arena = arena_new(0);
    Parser parser;
    parser_init(&parser, "for i in items { print(i) }", arena);
    int count;
    Stmt** stmts = parser_parse(&parser, &count);

    ASSERT(!parser_had_error(&parser));
    ASSERT_EQ(count, 1);
    ASSERT_EQ(stmts[0]->type, STMT_FOR);
    StmtFor* for_stmt = (StmtFor*)stmts[0];
    ASSERT_EQ(for_stmt->name.length, 1);
    ASSERT_NOT_NULL(for_stmt->iterable);
    ASSERT_NOT_NULL(for_stmt->body);

    arena_free(arena);
}

TEST(parse_return_with_value) {
    Arena* arena = arena_new(0);
    Parser parser;
    parser_in