pan);
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

    // Collect parameters
    int capacity = 8;
    int param_count = 0;
    Token* params = arena_alloc(parser->arena, sizeof(Token) * capacity);

    if (!check(parser, TOKEN_RIGHT_PAREN)) {
        do {
            if (param_count >= 255) {
                error_at_current(parser, "Cannot have more than 255 parameters.");
            }
            if (param_count >= capacity) {
                int new_capacity = capacity * 2;
                Token* new_params = arena_alloc(parser->arena, sizeof(Token) * new_capacity);
                memcpy(new_params, params, sizeof(Token) * param_count);
                params = new_params;
                capacity = new_capacity;
            }
            params[param_count++] = consume(parser, TOKEN_IDENTIFIER, "Expected parameter name.");
        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after parameters.");
    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' before function body.");

    Stmt* body = block(parser);
    Span span = span_merge(start_span, body->span);

    return stmt_function(parser->arena, name, params, param_count, body, span);
}

static Stmt* struct_declaration(Parser* parser) {
    Span start_span = span_from_token(parser->previous);

    Token name = consume(parser, TOKEN_IDENTIFIER, "Expected struct name.");
    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' after struct name.");

    // Collect fields
    int field_capacity = 8;
    int field_count = 0;
    Token* fields = arena_alloc(parser->arena, sizeof(Token) * field_capacity);

    // Collect methods
    int method_capacity = 8;
    int method_count = 0;
    Stmt** methods = arena_alloc(parser->arena, sizeof(Stmt*) * method_capacity);

    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)) {
        if (match(parser, TOKEN_FUNCTION)) {
            // Parse method
            if (method_count >= method_capacity) {
                int new_capacity = method_capacity * 2;
                Stmt** new_methods = arena_alloc(parser->arena, sizeof(Stmt*) * new_capacity);
                memcpy(new_methods, methods, sizeof(Stmt*) * method_count);
                methods = new_methods;
                method_capacity = new_capacity;
            }
            methods[method_count++] = function_declaration(parser);
        } else {
            // Parse field
            if (field_count >= field_capacity) {
                int new_capacity = field_capacity * 2;
                Token* new_fields = arena_alloc(parser->arena, sizeof(Token) * new_capacity);
                memcpy(new_fields, fields, sizeof(Token) * field_count);
                fields = new_fields;
                field_capacity = new_capacity;
            }
            fields[field_count++] = consume(parser, TOKEN_IDENTIFIER, "Expected field name.");

            // Optional comma between fields
            match(parser, TOKEN_COMMA);
        }
    }

    Token end = consume(parser, TOKEN_RIGHT_BRACE, "Expected '}' after struct body.");
    Span span = span_merge(start_span, span_from_token(end));

    return stmt_struct(parser->arena, name, fields, field_count, methods, method_count, span);
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
        return block(parser);
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

            if (!check(parser, TOKEN_RIGHT_PAREN)) {
                do {
                    if (param_count >= capacity) {
                        int new_capacity = capacity * 2;
                        Token* new_params = arena_alloc(parser->arena, sizeof(Token) * new_capacity);
                        memcpy(new_params, params, sizeof(Token) * param_count);
                        params = new_params;
                        capacity = new_capacity;
                    }
                    params[param_count++] = consume(parser, TOKEN_IDENTIFIER, "Expected parameter name.");
                } while (match(parser, TOKEN_COMMA));
            }

            consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after parameters.");
            consume(parser, TOKEN_LEFT_BRACE, "Expected '{' before function body.");

            Stmt* body = block(parser);
            Span span = span_merge(start_span, body->span);

            Expr* fn_expr = expr_function(parser->arena, params, param_count, body, span);
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
