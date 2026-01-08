#ifndef PX_CCODEGEN_H
#define PX_CCODEGEN_H

#include "compiler/ast.h"
#include "compiler/types.h"
#include "compiler/typechecker.h"
#include "core/arena.h"
#include "core/strings.h"
#include <stdbool.h>
#include <stdio.h>

// ============================================================================
// C Code Generator for Pixel Static (AOT Compilation)
//
// Generates C source code from typed Pixel AST.
// The generated code can be compiled with a standard C compiler.
// ============================================================================

typedef struct CCodegen {
    Arena* arena;
    TypeChecker* typechecker;  // For type lookups
    StringBuilder output;       // C source output
    int indent_level;
    int temp_counter;           // For generating unique temp variable names
    int closure_counter;        // For generating unique closure struct names
    const char* filename;
    bool had_error;
} CCodegen;

// ============================================================================
// C Code Generator API
// ============================================================================

// Initialize C code generator
void ccodegen_init(CCodegen* gen, Arena* arena, TypeChecker* tc, const char* filename);

// Free C code generator resources
void ccodegen_free(CCodegen* gen);

// Generate C code from statements
// Returns the generated C source as a string (arena-allocated)
// Returns NULL on error
char* ccodegen_generate(CCodegen* gen, Stmt** stmts, int count);

// Generate C code and write to file
bool ccodegen_generate_to_file(CCodegen* gen, Stmt** stmts, int count,
                               const char* output_path);

// ============================================================================
// Code Generation Helpers
// ============================================================================

// Get the C type string for a Pixel type
const char* ccodegen_type_to_c(CCodegen* gen, Type* type);

// Generate a unique temporary variable name
const char* ccodegen_temp_var(CCodegen* gen);

#endif // PX_CCODEGEN_H
