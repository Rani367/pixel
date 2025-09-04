#ifndef PH_PARSER_H
#define PH_PARSER_H

#include "compiler/lexer.h"
#include "compiler/ast.h"
#include "core/arena.h"

typedef struct {
    Lexer lexer;
    Token current;
    Token previous;
    Arena* arena;
    bool had_error;
    bool panic_mode;
} Parser;

// Initialize parser with source code and arena for AST allocation
void parser_init(Parser* parser, const char* source, Arena* arena);

// Parse source into array of statements
// Returns NULL on fatal error, sets *out_count to number of statements
Stmt** parser_parse(Parser* parser, int* out_count);

// Check if parser encountered any errors
bool parser_had_error(Parser* parser);

#endif // PH_PARSER_H
