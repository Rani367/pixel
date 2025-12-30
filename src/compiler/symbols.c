#include "compiler/symbols.h"
#include <string.h>

void scope_init(Scope* scope, int depth, Scope* enclosing) {
    scope->symbols = NULL;
    scope->count = 0;
    scope->capacity = 0;
    scope->depth = depth;
    scope->enclosing = enclosing;
}

void scope_free(Scope* scope) {
    if (scope->symbols != NULL) {
        PH_FREE(scope->symbols);
        scope->symbols = NULL;
    }
    scope->count = 0;
    scope->capacity = 0;
}

Symbol* scope_add_symbol(Scope* scope, const char* name, int length,
                         SymbolKind kind, int slot) {
    // Grow array if needed
    if (scope->count >= scope->capacity) {
        int new_capacity = PH_GROW_CAPACITY(scope->capacity);
        scope->symbols = PH_REALLOC(scope->symbols,
                                     sizeof(Symbol) * new_capacity);
        scope->capacity = new_capacity;
    }

    Symbol* symbol = &scope->symbols[scope->count++];
    symbol->name = name;
    symbol->length = length;
    symbol->kind = kind;
    symbol->depth = scope->depth;
    symbol->slot = slot;
    symbol->is_captured = false;
    symbol->is_initialized = false;

    return symbol;
}

Symbol* scope_lookup_local(Scope* scope, const char* name, int length) {
    // Search backwards to find most recent declaration
    for (int i = scope->count - 1; i >= 0; i--) {
        Symbol* symbol = &scope->symbols[i];
        if (names_equal(symbol->name, symbol->length, name, length)) {
            return symbol;
        }
    }
    return NULL;
}

Symbol* scope_lookup(Scope* scope, const char* name, int length) {
    Scope* current = scope;
    while (current != NULL) {
        Symbol* symbol = scope_lookup_local(current, name, length);
        if (symbol != NULL) {
            return symbol;
        }
        current = current->enclosing;
    }
    return NULL;
}

bool names_equal(const char* a, int a_len, const char* b, int b_len) {
    if (a_len != b_len) return false;
    return memcmp(a, b, a_len) == 0;
}
