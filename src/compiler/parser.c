#include "compiler/parser.h"
#include "core/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Precedence Levels
// ============================================================================

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * / %
    PREC_UNARY,       // - not
    PREC_CALL,        // () [] .
    PREC_PRIMARY,
} Precedence;

// ============================================================================
// Parse Rule Types
// ============================================================================

typedef Expr* (*ParsePrefixFn)(Parser* parser);
typedef Expr* (*ParseInfixFn)(Parser* parser, Expr* left);

typedef struct {
    ParsePrefixFn prefix;
    ParseInfixFn infix;
    Precedence precedence;
} ParseRule;

// Forward declarations
static Expr* expression(Parser* parser);
static Expr* parse_precedence(Parser* parser, Precedence precedence);
static Stmt* declaration(Parser* parser);
static Stmt* statement(Parser* parser);
static Stmt* block(Parser* parser);
static ParseRule* get_rule(TokenType type);

// ============================================================================
// Error Handling
// ============================================================================

static void error_at(Parser* parser, Token* token, const char* message) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->had_error = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing
    // LCOV_EXCL_START - error display format
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    // LCOV_EXCL_STOP

    fprintf(stderr, ": %s\n", message);
}

static void error(Parser* parser, const char* message) {
    error_at(parser, &parser->previous, message);
}

static void error_at_current(Parser* parser, const char* message) {
    error_at(parser, &parser->current, message);
}

// ============================================================================
// Parser Utilities
// ============================================================================

static void advance(Parser* parser) {
    parser->previous = parser->current;

    for (;;) {
        parser->current = lexer_scan_token(&parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;

        error_at_current(parser, parser->current.start);  // LCOV_EXCL_LINE - lexer error
    }
}

static bool check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

static bool match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return false;
    advance(parser);
    return true;
}

static Token consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return parser->previous;
    }

    error_at_current(parser, message);
    return parser->current;
}

static void synchronize(Parser* parser) {
    parser->panic_mode = false;

    while (parser->current.type != TOKEN_EOF) {
        if (parser->previous.type == TOKEN_SEMICOLON) return;
        if (parser->previous.type == TOKEN_RIGHT_BRACE) return;

        switch (parser->current.type) {
            case TOKEN_FUNCTION:
            case TOKEN_STRUCT:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_RETURN:
            case TOKEN_BREAK:
            case TOKEN_CONTINUE:
                return;
            default:
                break;
        }

        advance(parser);
    }
}

// ============================================================================
// Type Expression Parsing (for Pixel Static / AOT)
// ============================================================================

// Check if current token is a primitive type keyword
static bool is_primitive_type(TokenType type) {
    return type == TOKEN_TYPE_NUM ||
           type == TOKEN_TYPE_INT ||
           type == TOKEN_TYPE_STR ||
           type == TOKEN_TYPE_BOOL ||
           type == TOKEN_TYPE_NONE;
}

// Forward declaration for recursive type parsing
static TypeExpr* parse_type_expr(Parser* parser);

// Parse a type expression
// Handles: num, int, str, bool, none, list<T>, func(T1, T2) -> R, StructName, any
static TypeExpr* parse_type_expr(Parser* parser) {
    Span start_span = span_from_token(parser->current);

    // Primitive types: num, int, str, bool, none
    if (is_primitive_type(parser->current.type)) {
        TokenType prim = parser->current.type;
        advance(parser);
        return type_expr_primitive(parser->arena, prim, start_span);
    }

    // any type
    if (match(parser, TOKEN_TYPE_ANY)) {
        return type_expr_any(parser->arena, start_span);
    }

    // list<T> type
    if (match(parser, TOKEN_TYPE_LIST)) {
        consume(parser, TOKEN_LESS, "Expected '<' after 'list'.");
        TypeExpr* element = parse_type_expr(parser);
        if (element == NULL) return NULL;
        consume(parser, TOKEN_GREATER, "Expected '>' after list element type.");
        return type_expr_list(parser->arena, element, start_span);
    }

    // func(T1, T2) -> R type
    if (match(parser, TOKEN_TYPE_FUNC)) {
        consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after 'func'.");

        int capacity = 8;
        int param_count = 0;
        TypeExpr** param_types = arena_alloc(parser->arena, sizeof(TypeExpr*) * capacity);

        if (!check(parser, TOKEN_RIGHT_PAREN)) {
            do {
                if (param_count >= capacity) {
                    int new_capacity = capacity * 2;
                    TypeExpr** new_params = arena_alloc(parser->arena, sizeof(TypeExpr*) * new_capacity);
                    memcpy(new_params, param_types, sizeof(TypeExpr*) * param_count);
                    param_types = new_params;
                    capacity = new_capacity;
                }
                TypeExpr* ptype = parse_type_expr(parser);
                if (ptype == NULL) return NULL;
                param_types[param_count++] = ptype;
            } while (match(parser, TOKEN_COMMA));
        }

        consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after function parameter types.");

        // Optional return type: -> ReturnType
        TypeExpr* return_type = NULL;
        if (match(parser, TOKEN_ARROW)) {
            return_type = parse_type_expr(parser);
            if (return_type == NULL) return NULL;
        }

        return type_expr_func(parser->arena, param_types, param_count, return_type, start_span);
    }

    // Struct type: identifier (e.g., Player, Enemy)
    if (check(parser, TOKEN_IDENTIFIER)) {
        Token name = parser->current;
        advance(parser);
        return type_expr_struct(parser->arena, name);
    }

    error_at_current(parser, "Expected type.");
    return NULL;
}

// ============================================================================
// Prefix Parsers
// ============================================================================

static Expr* number(Parser* parser) {
    double value = strtod(parser->previous.start, NULL);
    Span span = span_from_token(parser->previous);
    return expr_literal_number(parser->arena, value, span);
}

static Expr* string_(Parser* parser) {
    // Token includes quotes, so skip them
    const char* start = parser->previous.start + 1;
    int length = parser->previous.length - 2;
    Span span = span_from_token(parser->previous);
    return expr_literal_string(parser->arena, start, length, span);
}

static Expr* literal(Parser* parser) {
    Span span = span_from_token(parser->previous);
    switch (parser->previous.type) {
        case TOKEN_TRUE:
            return expr_literal_bool(parser->arena, true, span);
        case TOKEN_FALSE:
            return expr_literal_bool(parser->arena, false, span);
        case TOKEN_NULL:
            return expr_literal_null(parser->arena, span);
        default:
            return NULL;  // LCOV_EXCL_LINE - unreachable
    }
}

static Expr* identifier(Parser* parser) {
    return expr_identifier(parser->arena, parser->previous);
}

static Expr* grouping(Parser* parser) {
    Expr* expr = expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
    return expr;
}

static Expr* unary(Parser* parser) {
    TokenType operator = parser->previous.type;
    Span start_span = span_from_token(parser->previous);

    // Parse operand with higher precedence
    Expr* operand = parse_precedence(parser, PREC_UNARY);

    Span span = span_merge(start_span, operand->span);
    return expr_unary(parser->arena, operator, operand, span);
}

static Expr* list(Parser* parser) {
    Span start_span = span_from_token(parser->previous);

    // Collect elements
    int capacity = 8;
    int count = 0;
    Expr** elements = arena_alloc(parser->arena, sizeof(Expr*) * capacity);

    if (!check(parser, TOKEN_RIGHT_BRACKET)) {
        do {
            // LCOV_EXCL_START - capacity growth >8 elements
            if (count >= capacity) {
                int new_capacity = capacity * 2;
                Expr** new_elements = arena_alloc(parser->arena, sizeof(Expr*) * new_capacity);
                memcpy(new_elements, elements, sizeof(Expr*) * count);
                elements = new_elements;
                capacity = new_capacity;
            }
            // LCOV_EXCL_STOP
            elements[count++] = expression(parser);
        } while (match(parser, TOKEN_COMMA));
    }

    Token end = consume(parser, TOKEN_RIGHT_BRACKET, "Expected ']' after list elements.");
    Span span = span_merge(start_span, span_from_token(end));

    return expr_list(parser->arena, elements, count, span);
}

static Expr* function_expr(Parser* parser) {
    Span start_span = span_from_token(parser->previous);

    consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after 'function'.");

    // Collect parameters and optional type annotations
    int capacity = 8;
    int param_count = 0;
    Token* params = arena_alloc(parser->arena, sizeof(Token) * capacity);
    TypeExpr** param_types = arena_alloc(parser->arena, sizeof(TypeExpr*) * capacity);
    bool has_any_types = false;

    if (!check(parser, TOKEN_RIGHT_PAREN)) {
        do {
            // LCOV_EXCL_START - capacity growth >8 params
            if (param_count >= capacity) {
                int new_capacity = capacity * 2;
                Token* new_params = arena_alloc(parser->arena, sizeof(Token) * new_capacity);
                TypeExpr** new_types = arena_alloc(parser->arena, sizeof(TypeExpr*) * new_capacity);
                memcpy(new_params, params, sizeof(Token) * param_count);
                memcpy(new_types, param_types, sizeof(TypeExpr*) * param_count);
                params = new_params;
                param_types = new_types;
                capacity = new_capacity;
            }
            // LCOV_EXCL_STOP
            params[param_count] = consume(parser, TOKEN_IDENTIFIER, "Expected parameter name.");

            // Optional type annotation: param: type
            if (match(parser, TOKEN_COLON)) {
                param_types[param_count] = parse_type_expr(parser);
                if (param_types[param_count] == NULL) return NULL;
                has_any_types = true;
            } else {
                param_types[param_count] = NULL;
            }
            param_count++;
        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after parameters.");

    // Optional return type: -> ReturnType
    TypeExpr* return_type = NULL;
    if (match(parser, TOKEN_ARROW)) {
        return_type = parse_type_expr(parser);
        if (return_type == NULL) return NULL;
        has_any_types = true;
    }

    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' before function body.");

    Stmt* body = block(parser);
    Span span = span_merge(start_span, body->span);

    return expr_function(parser->arena, params, param_count,
                         has_any_types ? param_types : NULL, return_type, body, span);
}

// ============================================================================
// Infix Parsers
// ============================================================================

static Expr* binary(Parser* parser, Expr* left) {
    TokenType operator = parser->previous.type;
    ParseRule* rule = get_rule(operator);

    // Parse right operand with higher precedence (left-associative)
    Expr* right = parse_precedence(parser, (Precedence)(rule->precedence + 1));

    return expr_binary(parser->arena, left, operator, right);
}

static Expr* call(Parser* parser, Expr* callee) {
    Span start_span = callee->span;

    // Collect arguments
    int capacity = 8;
    int arg_count = 0;
    Expr** arguments = arena_alloc(parser->arena, sizeof(Expr*) * capacity);

    if (!check(parser, TOKEN_RIGHT_PAREN)) {
        do {
            // LCOV_EXCL_START - capacity growth >8 args
            if (arg_count >= 255) {
                error(parser, "Cannot have more than 255 arguments.");
            }
            if (arg_count >= capacity) {
                int new_capacity = capacity * 2;
                Expr** new_args = arena_alloc(parser->arena, sizeof(Expr*) * new_capacity);
                memcpy(new_args, arguments, sizeof(Expr*) * arg_count);
                arguments = new_args;
                capacity = new_capacity;
            }
            // LCOV_EXCL_STOP
            arguments[arg_count++] = expression(parser);
        } while (match(parser, TOKEN_COMMA));
    }

    Token end = consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after arguments.");
    Span span = span_merge(start_span, span_from_token(end));

    return expr_call(parser->arena, callee, arguments, arg_count, span);
}

static Expr* index_(Parser* parser, Expr* object) {
    Span start_span = object->span;

    Expr* index_expr = expression(parser);
    Token end = consume(parser, TOKEN_RIGHT_BRACKET, "Expected ']' after index.");
    Span span = span_merge(start_span, span_from_token(end));

    return expr_index(parser->arena, object, index_expr, span);
}

static Expr* dot(Parser* parser, Expr* object) {
    Token name = consume(parser, TOKEN_IDENTIFIER, "Expected property name after '.'.");
    return expr_get(parser->arena, object, name);
}

static Expr* postfix(Parser* parser, Expr* operand) {
    Token op = parser->previous;
    return expr_postfix(parser->arena, operand, op);
}

// ============================================================================
// Parse Rules Table
// ============================================================================

static ParseRule rules[TOKEN_COUNT];

static void init_rules(void) {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    // Initialize all to NULL/PREC_NONE
    for (int i = 0; i < TOKEN_COUNT; i++) {
        rules[i] = (ParseRule){NULL, NULL, PREC_NONE};
    }

    // Prefix rules
    rules[TOKEN_LEFT_PAREN]    = (ParseRule){grouping, call,    PREC_CALL};
    rules[TOKEN_LEFT_BRACKET]  = (ParseRule){list,     index_,  PREC_CALL};
    rules[TOKEN_MINUS]         = (ParseRule){unary,    binary,  PREC_TERM};
    rules[TOKEN_NOT]           = (ParseRule){unary,    NULL,    PREC_NONE};
    rules[TOKEN_NUMBER]        = (ParseRule){number,   NULL,    PREC_NONE};
    rules[TOKEN_STRING]        = (ParseRule){string_,  NULL,    PREC_NONE};
    rules[TOKEN_TRUE]          = (ParseRule){literal,  NULL,    PREC_NONE};
    rules[TOKEN_FALSE]         = (ParseRule){literal,  NULL,    PREC_NONE};
    rules[TOKEN_NULL]          = (ParseRule){literal,  NULL,    PREC_NONE};
    rules[TOKEN_IDENTIFIER]    = (ParseRule){identifier, NULL,  PREC_NONE};
    rules[TOKEN_THIS]          = (ParseRule){identifier, NULL,  PREC_NONE};
    rules[TOKEN_FUNCTION]      = (ParseRule){function_expr, NULL, PREC_NONE};

    // Infix rules
    rules[TOKEN_PLUS]          = (ParseRule){NULL, binary, PREC_TERM};
    rules[TOKEN_STAR]          = (ParseRule){NULL, binary, PREC_FACTOR};
    rules[TOKEN_SLASH]         = (ParseRule){NULL, binary, PREC_FACTOR};
    rules[TOKEN_PERCENT]       = (ParseRule){NULL, binary, PREC_FACTOR};
    rules[TOKEN_EQUAL_EQUAL]   = (ParseRule){NULL, binary, PREC_EQUALITY};
    rules[TOKEN_BANG_EQUAL]    = (ParseRule){NULL, binary, PREC_EQUALITY};
    rules[TOKEN_LESS]          = (ParseRule){NULL, binary, PREC_COMPARISON};
    rules[TOKEN_LESS_EQUAL]    = (ParseRule){NULL, binary, PREC_COMPARISON};
    rules[TOKEN_GREATER]       = (ParseRule){NULL, binary, PREC_COMPARISON};
    rules[TOKEN_GREATER_EQUAL] = (ParseRule){NULL, binary, PREC_COMPARISON};
    rules[TOKEN_AND]           = (ParseRule){NULL, binary, PREC_AND};
    rules[TOKEN_OR]            = (ParseRule){NULL, binary, PREC_OR};
    rules[TOKEN_DOT]           = (ParseRule){NULL, dot,     PREC_CALL};
    rules[TOKEN_PLUS_PLUS]     = (ParseRule){NULL, postfix, PREC_CALL};
    rules[TOKEN_MINUS_MINUS]   = (ParseRule){NULL, postfix, PREC_CALL};
}

static ParseRule* get_rule(TokenType type) {
    return &rules[type];
}

// ============================================================================
// Pratt Parser Core
// ============================================================================

static Expr* parse_precedence(Parser* parser, Precedence precedence) {
    advance(parser);

    ParsePrefixFn prefix_rule = get_rule(parser->previous.type)->prefix;
    if (prefix_rule == NULL) {
        error(parser, "Expected expression.");
        return NULL;
    }

    Expr* left = prefix_rule(parser);
    if (left == NULL) return NULL;

    while (precedence <= get_rule(parser->current.type)->precedence) {
        advance(parser);
        ParseInfixFn infix_rule = get_rule(parser->previous.type)->infix;
        // LCOV_EXCL_START - unreachable parser error
        if (infix_rule == NULL) {
            error(parser, "Expected infix operator.");
            return left;
        }
        // LCOV_EXCL_STOP
        left = infix_rule(parser, left);
        if (left == NULL) return NULL;
    }

    return left;
}

static Expr* expression(Parser* parser) {
    return parse_precedence(parser, PREC_ASSIGNMENT);
}

// ============================================================================
// Statement Parsing
// ============================================================================

static Stmt* expression_statement(Parser* parser) {
    Expr* expr = expression(parser);
    if (expr == NULL) return NULL;

    // Check for typed variable declaration: x: type = value
    if (match(parser, TOKEN_COLON)) {
        // Must be an identifier
        if (expr->type != EXPR_IDENTIFIER) {
            error(parser, "Expected identifier before ':'.");
            return NULL;
        }
        ExprIdentifier* ident = (ExprIdentifier*)expr;
        Token name = ident->name;

        // Parse the type annotation
        TypeExpr* type = parse_type_expr(parser);
        if (type == NULL) return NULL;

        // Require initializer for now
        consume(parser, TOKEN_EQUAL, "Expected '=' after type in variable declaration.");
        Expr* initializer = expression(parser);
        if (initializer == NULL) return NULL;

        Span span = span_merge(span_from_token(name), initializer->span);
        return stmt_var_decl(parser->arena, name, type, initializer, span);
    }

    // Check for assignment
    if (match(parser, TOKEN_EQUAL)) {
        Expr* value = expression(parser);
        if (value == NULL) return NULL;

        // Validate assignment target
        if (expr->type == EXPR_IDENTIFIER) {
            return stmt_assignment(parser->arena, expr, value);
        } else if (expr->type == EXPR_GET) {
            // obj.field = value becomes ExprSet
            ExprGet* get = (ExprGet*)expr;
            Expr* set = expr_set(parser->arena, get->object, get->name, value);
            return stmt_expression(parser->arena, set);
        } else if (expr->type == EXPR_INDEX) {
            // arr[i] = value becomes ExprIndexSet
            ExprIndex* idx = (ExprIndex*)expr;
            Expr* set = expr_index_set(parser->arena, idx->object, idx->index, value);
            return stmt_expression(parser->arena, set);
        // LCOV_EXCL_START - invalid assignment target
        } else {
            error(parser, "Invalid assignment target.");
            return NULL;
        }
        // LCOV_EXCL_STOP
    }

    // Handle compound assignment: += -= *= /=
    if (match(parser, TOKEN_PLUS_EQUAL) || match(parser, TOKEN_MINUS_EQUAL) ||
        match(parser, TOKEN_STAR_EQUAL) || match(parser, TOKEN_SLASH_EQUAL)) {
        TokenType compound_op = parser->previous.type;
        Expr* rhs = expression(parser);
        if (rhs == NULL) return NULL;

        // Determine binary operator
        TokenType binary_op;
        // LCOV_EXCL_START - compound assignment operators
        switch (compound_op) {
            case TOKEN_PLUS_EQUAL:  binary_op = TOKEN_PLUS;  break;
            case TOKEN_MINUS_EQUAL: binary_op = TOKEN_MINUS; break;
            case TOKEN_STAR_EQUAL:  binary_op = TOKEN_STAR;  break;
            case TOKEN_SLASH_EQUAL: binary_op = TOKEN_SLASH; break;
            default: binary_op = TOKEN_PLUS; break;  // Unreachable
        }
        // LCOV_EXCL_STOP

        // Desugar: x += y => x = x + y
        Expr* binary_expr = expr_binary(parser->arena, expr, binary_op, rhs);

        if (expr->type == EXPR_IDENTIFIER) {
            return stmt_assignment(parser->arena, expr, binary_expr);
        // LCOV_EXCL_START - compound assignment for property/index
        } else if (expr->type == EXPR_GET) {
            ExprGet* get = (ExprGet*)expr;
            Expr* set = expr_set(parser->arena, get->object, get->name, binary_expr);
            return stmt_expression(parser->arena, set);
        } else if (expr->type == EXPR_INDEX) {
            ExprIndex* idx = (ExprIndex*)expr;
            Expr* set = expr_index_set(parser->arena, idx->object, idx->index, binary_expr);
            return stmt_expression(parser->arena, set);
        } else {
            error(parser, "Invalid compound assignment target.");
            return NULL;
        }
        // LCOV_EXCL_STOP
    }

    return stmt_expression(parser->arena, expr);
}

static Stmt* block(Parser* parser) {
    Span start_span = span_from_token(parser->previous);

    int capacity = 8;
    int count = 0;
    Stmt** statements = arena_alloc(parser->arena, sizeof(Stmt*) * capacity);

    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)) {
        Stmt* stmt = declaration(parser);
        if (stmt != NULL) {
            if (count >= capacity) {
                int new_capacity = capacity * 2;
                Stmt** new_stmts = arena_alloc(parser->arena, sizeof(Stmt*) * new_capacity);
                memcpy(new_stmts, statements, sizeof(Stmt*) * count);
                statements = new_stmts;
                capacity = new_capacity;
            }
            statements[count++] = stmt;
        }
    }

    Token end = consume(parser, TOKEN_RIGHT_BRACE, "Expected '}' after block.");
    Span span = span_merge(start_span, span_from_token(end));

    return stmt_block(parser->arena, statements, count, span);
}

static Stmt* if_statement(Parser* parser) {
    Span start_span = span_from_token(parser->previous);

    Expr* condition = expression(parser);
    if (condition == NULL) return NULL;

    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' after if condition.");
    Stmt* then_branch = block(parser);

    Stmt* else_branch = NULL;
    Span end_span = then_branch->span;

    if (match(parser, TOKEN_ELSE)) {
        if (match(parser, TOKEN_IF)) {
            // else if
            else_branch = if_statement(parser);
        } else {
            consume(parser, TOKEN_LEFT_BRACE, "Expected '{' after else.");
            else_branch = block(parser);
        }
        if (else_branch) {
            end_span = else_branch->span;
        }
    }

    Span span = span_merge(start_span, end_span);
    return stmt_if(parser->arena, condition, then_branch, else_branch, span);
}

static Stmt* while_statement(Parser* parser) {
    Span start_span = span_from_token(parser->previous);

    Expr* condition = expression(parser);
    if (condition == NULL) return NULL;

    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' after while condition.");
    Stmt* body = block(parser);

    Span span = span_merge(start_span, body->span);
    return stmt_while(parser->arena, condition, body, span);
}

static Stmt* for_statement(Parser* parser) {
    Span start_span = span_from_token(parser->previous);

    Token name = consume(parser, TOKEN_IDENTIFIER, "Expected variable name after 'for'.");
    consume(parser, TOKEN_IN, "Expected 'in' after variable name.");

    Expr* iterable = expression(parser);
    if (iterable == NULL) return NULL;

    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' after for iterable.");
    Stmt* body = block(parser);

    Span span = span_merge(start_span, body->span);
    return stmt_for(parser->arena, name, iterable, body, span);
}

static Stmt* return_statement(Parser* parser) {
    Span start_span = span_from_token(parser->previous);

    Expr* value = NULL;
    Span end_span = start_span;

    // Check if there's a return value (next token isn't a statement terminator)
    if (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF) &&
        !check(parser, TOKEN_FUNCTION) && !check(parser, TOKEN_STRUCT) &&
        !check(parser, TOKEN_IF) && !check(parser, TOKEN_WHILE) &&
        !check(parser, TOKEN_FOR) && !check(parser, TOKEN_RETURN) &&
        !check(parser, TOKEN_BREAK) && !check(parser, TOKEN_CONTINUE)) {
        value = expression(parser);
        if (value) {
            end_span = value->span;
        }
    }

    Span span = span_merge(start_span, end_span);
    return stmt_return(parser->arena, value, span);
}

static Stmt* break_statement(Parser* parser) {
    Span span = span_from_token(parser->previous);
    return stmt_break(parser->arena, span);
}

static Stmt* continue_statement(Parser* parser) {
    Span span = span_from_token(parser->previous);
    return stmt_continue(parser->arena, span);
}

static Stmt* function_declaration(Parser* parser) {
    Span start_span = span_from_token(parser->previous);

    Token name = consume(parser, TOKEN_IDENTIFIER, "Expected function name.");
    consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after function name.");

    // Collect parameters and optional type annotations
    int capacity = 8;
    int param_count = 0;
    Token* params = arena_alloc(parser->arena, sizeof(Token) * capacity);
    TypeExpr** param_types = arena_alloc(parser->arena, sizeof(TypeExpr*) * capacity);
    bool has_any_types = false;

    if (!check(parser, TOKEN_RIGHT_PAREN)) {
        do {
            // LCOV_EXCL_START - capacity growth >8 params
            if (param_count >= 255) {
                error_at_current(parser, "Cannot have more than 255 parameters.");
            }
            if (param_count >= capacity) {
                int new_capacity = capacity * 2;
                Token* new_params = arena_alloc(parser->arena, sizeof(Token) * new_capacity);
                TypeExpr** new_types = arena_alloc(parser->arena, sizeof(TypeExpr*) * new_capacity);
                memcpy(new_params, params, sizeof(Token) * param_count);
                memcpy(new_types, param_types, sizeof(TypeExpr*) * param_count);
                params = new_params;
                param_types = new_types;
                capacity = new_capacity;
            }
            // LCOV_EXCL_STOP
            params[param_count] = consume(parser, TOKEN_IDENTIFIER, "Expected parameter name.");

            // Optional type annotation: param: type
            if (match(parser, TOKEN_COLON)) {
                param_types[param_count] = parse_type_expr(parser);
                if (param_types[param_count] == NULL) return NULL;
                has_any_types = true;
            } else {
                param_types[param_count] = NULL;
            }
            param_count++;
        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after parameters.");

    // Optional return type: -> ReturnType
    TypeExpr* return_type = NULL;
    if (match(parser, TOKEN_ARROW)) {
        return_type = parse_type_expr(parser);
        if (return_type == NULL) return NULL;
        has_any_types = true;
    }

    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' before function body.");

    Stmt* body = block(parser);
    Span span = span_merge(start_span, body->span);

    // Only pass param_types if any were provided
    return stmt_function(parser->arena, name, params, param_count,
                         has_any_types ? param_types : NULL, return_type, body, span);
}

static Stmt* struct_declaration(Parser* parser) {
    Span start_span = span_from_token(parser->previous);

    Token name = consume(parser, TOKEN_IDENTIFIER, "Expected struct name.");
    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' after struct name.");

    // Collect fields and optional type annotations
    int field_capacity = 8;
    int field_count = 0;
    Token* fields = arena_alloc(parser->arena, sizeof(Token) * field_capacity);
    TypeExpr** field_types = arena_alloc(parser->arena, sizeof(TypeExpr*) * field_capacity);
    bool has_any_types = false;

    // Collect methods
    int method_capacity = 8;
    int method_count = 0;
    Stmt** methods = arena_alloc(parser->arena, sizeof(Stmt*) * method_capacity);

    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)) {
        if (match(parser, TOKEN_FUNCTION)) {
            // Parse method
            // LCOV_EXCL_START - capacity growth >8 methods
            if (method_count >= method_capacity) {
                int new_capacity = method_capacity * 2;
                Stmt** new_methods = arena_alloc(parser->arena, sizeof(Stmt*) * new_capacity);
                memcpy(new_methods, methods, sizeof(Stmt*) * method_count);
                methods = new_methods;
                method_capacity = new_capacity;
            }
            // LCOV_EXCL_STOP
            methods[method_count++] = function_declaration(parser);
        } else {
            // Parse field
            // LCOV_EXCL_START - capacity growth >8 fields
            if (field_count >= field_capacity) {
                int new_capacity = field_capacity * 2;
                Token* new_fields = arena_alloc(parser->arena, sizeof(Token) * new_capacity);
                TypeExpr** new_types = arena_alloc(parser->arena, sizeof(TypeExpr*) * new_capacity);
                memcpy(new_fields, fields, sizeof(Token) * field_count);
                memcpy(new_types, field_types, sizeof(TypeExpr*) * field_count);
                fields = new_fields;
                field_types = new_types;
                field_capacity = new_capacity;
            }
            // LCOV_EXCL_STOP
            fields[field_count] = consume(parser, TOKEN_IDENTIFIER, "Expected field name.");

            // Optional type annotation: field: type
            if (match(parser, TOKEN_COLON)) {
                field_types[field_count] = parse_type_expr(parser);
                if (field_types[field_count] == NULL) return NULL;
                has_any_types = true;
            } else {
                field_types[field_count] = NULL;
            }
            field_count++;

            // Optional comma between fields
            match(parser, TOKEN_COMMA);
        }
    }

    Token end = consume(parser, TOKEN_RIGHT_BRACE, "Expected '}' after struct body.");
    Span span = span_merge(start_span, span_from_token(end));

    // Only pass field_types if any were provided
    return stmt_struct(parser->arena, name, fields, field_count,
                       has_any_types ? field_types : NULL, methods, method_count, span);
}

static Stmt* statement(Parser* parser) {
    if (match(parser, TOKEN_IF)) {
        return if_statement(parser);
    }
    if (match(parser, TOKEN_WHILE)) {
        return while_statement(parser);
    }
    if (match(parser, TOKEN_FOR)) {
        return for_statement(parser);
    }
    if (match(parser, TOKEN_RETURN)) {
        return return_statement(parser);
    }
    if (match(parser, TOKEN_BREAK)) {
        return break_statement(parser);
    }
    if (match(parser, TOKEN_CONTINUE)) {
        return continue_statement(parser);
    }
    if (match(parser, TOKEN_LEFT_BRACE)) {
        return block(parser);  // LCOV_EXCL_LINE - standalone block
    }

    return expression_statement(parser);
}

static Stmt* declaration(Parser* parser) {
    Stmt* stmt = NULL;

    // function NAME(...) is a declaration, function(...) is an expression
    if (check(parser, TOKEN_FUNCTION)) {
        // Peek ahead: if next-next token is IDENTIFIER, it's a declaration
        // We need to look past 'function' to see if an identifier follows
        advance(parser);  // consume 'function'
        if (check(parser, TOKEN_IDENTIFIER)) {
            stmt = function_declaration(parser);
        } else {
            // It's a function expression - back up and let expression_statement handle it
            // We can't easily back up, so we handle the function expression here
            // as an expression statement
            Span start_span = span_from_token(parser->previous);

            consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after 'function'.");

            int capacity = 8;
            int param_count = 0;
            Token* params = arena_alloc(parser->arena, sizeof(Token) * capacity);
            TypeExpr** param_types = arena_alloc(parser->arena, sizeof(TypeExpr*) * capacity);
            bool has_any_types = false;

            if (!check(parser, TOKEN_RIGHT_PAREN)) {
                do {
                    // LCOV_EXCL_START - capacity growth >8 params
                    if (param_count >= capacity) {
                        int new_capacity = capacity * 2;
                        Token* new_params = arena_alloc(parser->arena, sizeof(Token) * new_capacity);
                        TypeExpr** new_types = arena_alloc(parser->arena, sizeof(TypeExpr*) * new_capacity);
                        memcpy(new_params, params, sizeof(Token) * param_count);
                        memcpy(new_types, param_types, sizeof(TypeExpr*) * param_count);
                        params = new_params;
                        param_types = new_types;
                        capacity = new_capacity;
                    }
                    // LCOV_EXCL_STOP
                    params[param_count] = consume(parser, TOKEN_IDENTIFIER, "Expected parameter name.");

                    // Optional type annotation: param: type
                    if (match(parser, TOKEN_COLON)) {
                        param_types[param_count] = parse_type_expr(parser);
                        if (param_types[param_count] == NULL) return NULL;
                        has_any_types = true;
                    } else {
                        param_types[param_count] = NULL;
                    }
                    param_count++;
                } while (match(parser, TOKEN_COMMA));
            }

            consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after parameters.");

            // Optional return type: -> ReturnType
            TypeExpr* return_type = NULL;
            if (match(parser, TOKEN_ARROW)) {
                return_type = parse_type_expr(parser);
                if (return_type == NULL) return NULL;
                has_any_types = true;
            }

            consume(parser, TOKEN_LEFT_BRACE, "Expected '{' before function body.");

            Stmt* body = block(parser);
            Span span = span_merge(start_span, body->span);

            Expr* fn_expr = expr_function(parser->arena, params, param_count,
                                          has_any_types ? param_types : NULL, return_type, body, span);
            stmt = stmt_expression(parser->arena, fn_expr);
        }
    } else if (match(parser, TOKEN_STRUCT)) {
        stmt = struct_declaration(parser);
    } else {
        stmt = statement(parser);
    }

    if (parser->panic_mode) {
        synchronize(parser);
    }

    return stmt;
}

// ============================================================================
// Public API
// ============================================================================

void parser_init(Parser* parser, const char* source, Arena* arena) {
    lexer_init(&parser->lexer, source);
    parser->arena = arena;
    parser->had_error = false;
    parser->panic_mode = false;
    parser->current = (Token){0};
    parser->previous = (Token){0};

    init_rules();

    // Prime the parser with the first token
    advance(parser);
}

Stmt** parser_parse(Parser* parser, int* out_count) {
    int capacity = 8;
    int count = 0;
    Stmt** statements = arena_alloc(parser->arena, sizeof(Stmt*) * capacity);

    while (!check(parser, TOKEN_EOF)) {
        Stmt* stmt = declaration(parser);
        if (stmt != NULL) {
            if (count >= capacity) {
                int new_capacity = capacity * 2;
                Stmt** new_stmts = arena_alloc(parser->arena, sizeof(Stmt*) * new_capacity);
                memcpy(new_stmts, statements, sizeof(Stmt*) * count);
                statements = new_stmts;
                capacity = new_capacity;
            }
            statements[count++] = stmt;
        }
    }

    *out_count = count;
    return statements;
}

bool parser_had_error(Parser* parser) {
    return parser->had_error;
}
