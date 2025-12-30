#ifndef PH_ANALYZER_H
#define PH_ANALYZER_H

#include "core/common.h"
#include "core/error.h"
#include "compiler/ast.h"
#include "compiler/symbols.h"

// Maximum number of errors before stopping analysis
#define ANALYZER_MAX_ERRORS 32

// Analyzer context
typedef struct {
    Scope* current_scope;       // Current scope being analyzed
    Scope global_scope;         // The global scope
    int loop_depth;             // Nesting depth of loops (for break/continue)
    int function_depth;         // Nesting depth of functions (for return)
    bool in_struct;             // Inside a struct definition?

    // Error tracking
    Error* errors[ANALYZER_MAX_ERRORS];
    int error_count;

    // For variable slot assignment
    int local_count;            // Number of locals in current function

    // Source file name for error messages
    const char* source_file;

    // Source code for pretty error printing
    const char* source;
} Analyzer;

// Initialize the analyzer
void analyzer_init(Analyzer* analyzer, const char* source_file, const char* source);

// Free analyzer resources
void analyzer_free(Analyzer* analyzer);

// Analyze a list of statements (returns true if no errors)
bool analyzer_analyze(Analyzer* analyzer, Stmt** statements, int count);

// Get the number of errors
int analyzer_error_count(Analyzer* analyzer);

// Get an error by index
Error* analyzer_get_error(Analyzer* analyzer, int index);

// Print all errors
void analyzer_print_errors(Analyzer* analyzer, FILE* out);

// Declare a global variable (for native functions, etc.)
void analyzer_declare_global(Analyzer* analyzer, const char* name);

#endif // PH_ANALYZER_H
