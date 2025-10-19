
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

    // Source info for error #ifndef PH_CODEGEN_H
#define PH_CODEGEN_H

#include "core/common.h"
#include "core/error.h"
#include "compiler/ast.h"
#include "vm/chunk.h"
#include "vm/object.h"

// Maximum number of local variables per function
#define UINT8_COUNT 256

// Maximum number of upvalues per closure
#define MAX_UPVALUES 256

// Maximum number of errors before stopping compilation
#define CODEGEN_MAX_ERRORS 32

// Local variable in the current scope
typedef struct {
    const char* name;       // Pointer into source (not owned)
    int length;             // Name length
    int depth;              // Scope depth where declared
    bool is_captured;       // Captured by a closure?
} Local;

// Upvalue tracking
typedef struct {