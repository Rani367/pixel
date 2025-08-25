
// Get an error by index
Error* codegen_get_error(Codegen* codegen, int index);

// Print all errors
void codegen_print_errors(Codegen* codegen, FILE* out);

#endif // PH_CODEGEN_H
messages
    const char* source_file;
    const char* source;             // Source code for pretty error printing
} Codegen;

// ============================================================================
// Codegen API
// ============================================================================

// Initialize the codegen context
void codegen_init(Codegen* codegen, const char* source_file, const char* source);

// Free codegen resources
void codegen_free(Codegen* codegen);

// Compile a list of top-level statements
// Returns the compiled function (script), or NULL on error
ObjFunction* codegen_compile(Codegen* codegen, Stmt** statements, int count);

// ============================================================================
// Error Reporting
// ============================================================================

// Get the number of errors
int codegen_error_count(Codegen* codegen);
