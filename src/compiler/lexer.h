#ifndef PH_LEXER_H
#define PH_LEXER_H

#include "token.h"

typedef struct {
    const char* source;    // Full source string
    const char* start;     // Start of current token
    const char* current;   // Current character
    int line;
    int column;
    int start_column;      // Column where current token started
    const char* error;     // Pending error message (NULL if none)
} Lexer;

// Initialize a lexer with source code
void lexer_init(Lexer* lexer, const char* source);

// Scan and return the next token
Token lexer_scan_token(Lexer* lexer);

#endif // PH_LEXER_H
