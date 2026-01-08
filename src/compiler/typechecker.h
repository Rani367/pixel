#ifndef PX_TYPECHECKER_H
#define PX_TYPECHECKER_H

#include "compiler/ast.h"
#include "compiler/types.h"
#include "core/arena.h"
#include "core/table.h"
#include <stdbool.h>

// ============================================================================
// Type Checker for Pixel Static (AOT Compilation)
// ============================================================================

// Type info associated with symbols
typedef struct {
    Type* type;
    bool is_mutable;
} TypeInfo;

// Type checker context
typedef struct {
    Arena* arena;           // Memory allocation
    Table type_table;       // Symbol name -> TypeInfo
    Table struct_types;     // Struct name -> Type*
    const char* filename;   // Current file being checked
    const char* source;     // Source code for error messages
    bool had_error;
} TypeChecker;

// ============================================================================
// Type Checker API
// ============================================================================

// Initialize type checker
void typechecker_init(TypeChecker* tc, Arena* arena, const char* filename, const char* source);

// Free type checker resources
void typechecker_free(TypeChecker* tc);

// Main entry point: type check a list of statements
// Returns true if no type errors were found
bool typechecker_check(TypeChecker* tc, Stmt** stmts, int count);

// Get the inferred type for an expression
// Returns NULL if expression has no type (e.g., error)
Type* typechecker_get_expr_type(TypeChecker* tc, Expr* expr);

// Look up a symbol's type
// Returns NULL if not found
Type* typechecker_lookup(TypeChecker* tc, const char* name, int length);

// Declare a built-in function type
void typechecker_declare_builtin(TypeChecker* tc, const char* name, Type* type);

// Register all standard library built-in functions
void typechecker_register_builtins(TypeChecker* tc);

// ============================================================================
// Type Conversion from AST TypeExpr
// ============================================================================

// Convert a TypeExpr AST node to a Type*
// Returns type_error() on invalid type expressions
Type* typechecker_resolve_type_expr(TypeChecker* tc, TypeExpr* type_expr);

#endif // PX_TYPECHECKER_H
