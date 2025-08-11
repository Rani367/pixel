ope and all enclosing scopes
Symbol* scope_lookup(Scope* scope, const char* name, int length);

// Check if two names match
bool names_equal(const char* a, int a_len, const char* b, int b_len);

#endif // PH_SYMBOLS_H
