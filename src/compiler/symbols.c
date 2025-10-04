kind;
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
