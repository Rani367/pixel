
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

    uint8_t index;          // Index in enclosing function's locals or upvalues
    bool is_local;          // true = local in enclosing, false = upvalue in enclosing
} Upvalue;

// Function type for tracking context
typedef enum {
    TYPE_SCRIPT,            // Top-level script
    TYPE_FUNCTION,          // Named function
    TYPE_METHOD,            // Struct method (future use)
    TYPE_INITIALIZER,       // Struct initializer (future use)
} FunctionType;

// Compiler context for a single function
typedef struct Compiler {
    struct Compiler* enclosing;     // Enclosing compiler (for nested functions)
    ObjFunction* function;          // Function being compiled
    FunctionType type;

    Local locals[UINT8_COUNT];
    int local_count;
    int scope_depth;

    Upvalue upvalues[MAX_UPVALUES];

    // Loop tracking for break/continue
    int loop_start;                 // Start of current loop (-1 if not in loop)
    int loop_depth;                 // Scope depth at loop start
    int* break_jumps;               // Array of break jump locations to patch
    int break_count;
    int break_capacity;
} Compiler;

// Main codegen context
typedef struct {
    Compiler* current;              // Current compiler (function being compiled)

    // Error tracking
    Error* errors[CODEGEN_MAX_ERRORS];
    int error_count;
    bool had_error;
    bool panic_mode;

    // Source info for error 