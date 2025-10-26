#ifndef PH_AST_H
#define PH_AST_H

#include "core/arena.h"
#include "compiler/token.h"

// Forward declarations
typedef struct Expr Expr;
typedef struct Stmt Stmt;

// Source span for error messages
typedef struct {
    int start_line;
    int start_column;
    int end_line;
    int end_column;
} Span;

// Expression types
typedef enum {
    EXPR_LITERAL_NULL,
    EXPR_LITERAL_BOOL,
    EXPR_LITERAL_NUMBER,
    EXPR_LITERAL_STRING,
    EXPR_IDENTIFIER,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_CALL,
    EXPR_GET,           // obj.field
    EXPR_SET,           // obj.field = value
    EXPR_INDEX,         // arr[i]
    EXPR_INDEX_SET,     // arr[i] = value
    EXPR_LIST,          // [a, b, c]
    EXPR_FUNCTION,      // function(x) { ... }
    EXPR_VEC2,          // vec2(x, y)
    EXPR_POSTFIX,       // x++ or x--

    EXPR_COUNT
} ExprType;

// Statement types
typedef enum {
    STMT_EXPRESSION,
    STMT_ASSIGNMENT,
    STMT_BLOCK,
    STMT_IF,
    STMT_WHILE,
    STMT_FOR,
    STMT_RETURN,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_FUNCTION,
    STMT_STRUCT,

    STMT_COUNT
} StmtType;

// Base expression (all expressions embed this)
struct Expr {
    ExprType type;
    Span span;
};

// Base statement (all statements embed this)
struct Stmt {
    StmtType type;
    Span span;
};

// ============================================================================
// Expression Types
// ============================================================================

typedef struct {
    Expr base;
} ExprLiteralNull;

typedef struct {
    Expr base;
    bool value;
} ExprLiteralBool;

typedef struct {
    Expr base;
    double value;
} ExprLiteralNumber;

typedef struct {
    Expr base;
    char* value;        // Arena-allocated copy
    int length;
} ExprLiteralString;

typedef struct {
    Expr base;
    Token name;
} ExprIdentifier;

typedef struct {
    Expr base;
    TokenType operator;
    Expr* operand;
} ExprUnary;

typedef struct {
    Expr base;
    Expr* left;
    TokenType operator;
    Expr* right;
} ExprBinary;

typedef struct {
    Expr base;
    Expr* callee;
    Expr** arguments;   // Arena-allocated array
    int arg_count;
} ExprCall;

typedef struct {
    Expr base;
    Expr* object;
    Token name;
} ExprGet;

typedef struct {
    Expr base;
    Expr* object;
    Token name;
    Expr* value;
} ExprSet;

typedef struct {
    Expr base;
    Expr*