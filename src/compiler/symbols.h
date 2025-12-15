ope and all enclosing scopes
Symbol* scope_lookup(Scope* scope, const char* name, int length);

// Check if two names match
bool names_equal(const char* a, int a_len, const char* b, int b_len);

#endif // PH_SYMBOLS_H
 a scope
void scope_init(Scope* scope, int depth, Scope* enclosing);

// Free a scope's symbol array (does not free enclosing scopes)
void scope_free(Scope* scope);

// Add a symbol to a scope (returns pointer to the added symbol)
Symbol* scope_add_symbol(Scope* scope, const char* name, int length,
        #ifndef PH_SYMBOLS_H
#define PH_SYMBOLS_H

#include "core/common.h"
#include "compiler/token.h"

// Symbol kinds
typedef enum {
    SYMBOL_LOCAL,       // Local variable in a function
    SYMBOL_GLOBAL,      // Global variable
    SYMBOL_FUNCTION,    // Function definition
    SYMBOL_STRUCT,      // Struct definition
    SYMBOL_PARAMETER,   // Function parameter
} SymbolKind;

// Symbol entry in the symbol table
typedef struct Symbol {
    const char* name;       // Pointer into source (not owned)
    int length;             // Name length
    SymbolKind kind;
    int depth;              // Scope depth (0 = global)
    int slot;               // Stack slot for locals/parameters
    bool is_captured;       // Captured by a closure?
    bool is_initialized;    // Has been assigned a value?
} Symbol;

// Scope - a linked list of scopes for nested blocks
typedef struct Scope {
    Symbol* symbols;        // Dynamic array of symbols
    int count;
    int capacity;
    int depth;              // Nesting depth (0 = global)
    struct Scope* enclosing;
} Scope;

// Initialize                 SymbolKind kind, int slot);

// Look up a symbol in a scope (not enclosing scopes)
Symbol* scope_lookup_local(Scope* scope, const char* name, int length);

// Look up a symbol in this sc