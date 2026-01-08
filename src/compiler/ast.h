#ifndef PH_AST_H
#define PH_AST_H

#include "core/arena.h"
#include "compiler/token.h"

// Forward declarations
typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct TypeExpr TypeExpr;

// Source span for error messages
typedef struct {
    int start_line;
    int start_column;
    int end_line;
    int end_column;
} Span;

// ============================================================================
// Type Expression Types (for Pixel Static / AOT compilation)
// ============================================================================

typedef enum {
    TYPE_EXPR_PRIMITIVE,   // num, int, str, bool, none
    TYPE_EXPR_LIST,        // list<T>
    TYPE_EXPR_FUNC,        // func(T1, T2) -> R
    TYPE_EXPR_STRUCT,      // User-defined struct type (by name)
    TYPE_EXPR_ANY,         // Dynamic 'any' type (escape hatch)

    TYPE_EXPR_COUNT
} TypeExprKind;

// Base type expression (all type expressions embed this)
struct TypeExpr {
    TypeExprKind kind;
    Span span;
};

// Primitive type: num, int, str, bool, none
typedef struct {
    TypeExpr base;
    TokenType primitive_type;  // TOKEN_TYPE_NUM, TOKEN_TYPE_INT, etc.
} TypeExprPrimitive;

// List type: list<T>
typedef struct {
    TypeExpr base;
    TypeExpr* element_type;
} TypeExprList;

// Function type: func(T1, T2) -> R
typedef struct {
    TypeExpr base;
    TypeExpr** param_types;
    int param_count;
    TypeExpr* return_type;     // NULL means none/void
} TypeExprFunc;

// Struct type reference: Player, Enemy, etc.
typedef struct {
    TypeExpr base;
    Token name;
} TypeExprStruct;

// Any type (for dynamic fallback)
typedef struct {
    TypeExpr base;
} TypeExprAny;

// ============================================================================
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
    STMT_VAR_DECL,     // Typed variable declaration: x: num = 42

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
    Expr* object;
    Expr* index;
} ExprIndex;

typedef struct {
    Expr base;
    Expr* object;
    Expr* index;
    Expr* value;
} ExprIndexSet;

typedef struct {
    Expr base;
    Expr** elements;    // Arena-allocated array
    int count;
} ExprList;

typedef struct {
    Expr base;
    Token* params;          // Arena-allocated array
    int param_count;
    TypeExpr** param_types; // Optional type annotations (NULL if untyped)
    TypeExpr* return_type;  // Optional return type (NULL if untyped)
    Stmt* body;             // Block statement
} ExprFunction;

typedef struct {
    Expr base;
    Expr* x;
    Expr* y;
} ExprVec2;

typedef struct {
    Expr base;
    Expr* operand;      // Must be an lvalue (identifier, get, or index)
    Token op;           // TOKEN_PLUS_PLUS or TOKEN_MINUS_MINUS
} ExprPostfix;

// ============================================================================
// Statement Types
// ============================================================================

typedef struct {
    Stmt base;
    Expr* expression;
} StmtExpression;

typedef struct {
    Stmt base;
    Expr* target;       // identifier, get, or index expression
    Expr* value;
} StmtAssignment;

typedef struct {
    Stmt base;
    Stmt** statements;  // Arena-allocated array
    int count;
} StmtBlock;

typedef struct {
    Stmt base;
    Expr* condition;
    Stmt* then_branch;
    Stmt* else_branch;  // NULL if no else
} StmtIf;

typedef struct {
    Stmt base;
    Expr* condition;
    Stmt* body;
} StmtWhile;

typedef struct {
    Stmt base;
    Token name;         // Loop variable
    Expr* iterable;
    Stmt* body;
} StmtFor;

typedef struct {
    Stmt base;
    Expr* value;        // NULL for bare return
} StmtReturn;

typedef struct {
    Stmt base;
} StmtBreak;

typedef struct {
    Stmt base;
} StmtContinue;

typedef struct {
    Stmt base;
    Token name;
    Token* params;          // Arena-allocated array
    int param_count;
    TypeExpr** param_types; // Optional type annotations (NULL if untyped)
    TypeExpr* return_type;  // Optional return type (NULL if untyped)
    Stmt* body;             // Block statement
} StmtFunction;

typedef struct {
    Stmt base;
    Token name;
    Token* fields;          // Arena-allocated array of field names
    int field_count;
    TypeExpr** field_types; // Optional type annotations (NULL if untyped)
    Stmt** methods;         // Arena-allocated array of StmtFunction pointers
    int method_count;
} StmtStruct;

// Typed variable declaration: x: num = 42
typedef struct {
    Stmt base;
    Token name;
    TypeExpr* type;         // Required type annotation
    Expr* initializer;      // Optional initializer (NULL if none)
} StmtVarDecl;

// ============================================================================
// Span Utilities
// ============================================================================

// Create a span from a single token
Span span_from_token(Token token);

// Merge two spans (start of first, end of second)
Span span_merge(Span start, Span end);

// ============================================================================
// Type Expression Constructors
// ============================================================================

TypeExpr* type_expr_primitive(Arena* arena, TokenType primitive_type, Span span);
TypeExpr* type_expr_list(Arena* arena, TypeExpr* element_type, Span span);
TypeExpr* type_expr_func(Arena* arena, TypeExpr** param_types, int param_count,
                          TypeExpr* return_type, Span span);
TypeExpr* type_expr_struct(Arena* arena, Token name);
TypeExpr* type_expr_any(Arena* arena, Span span);

// Get the name of a type expression kind
const char* type_expr_kind_name(TypeExprKind kind);

// Print a type expression for debugging
void type_expr_print(TypeExpr* type);

// ============================================================================
// Expression Constructors
// ============================================================================

Expr* expr_literal_null(Arena* arena, Span span);
Expr* expr_literal_bool(Arena* arena, bool value, Span span);
Expr* expr_literal_number(Arena* arena, double value, Span span);
Expr* expr_literal_string(Arena* arena, const char* value, int length, Span span);
Expr* expr_identifier(Arena* arena, Token name);
Expr* expr_unary(Arena* arena, TokenType op, Expr* operand, Span span);
Expr* expr_binary(Arena* arena, Expr* left, TokenType op, Expr* right);
Expr* expr_call(Arena* arena, Expr* callee, Expr** args, int arg_count, Span span);
Expr* expr_get(Arena* arena, Expr* object, Token name);
Expr* expr_set(Arena* arena, Expr* object, Token name, Expr* value);
Expr* expr_index(Arena* arena, Expr* object, Expr* index, Span span);
Expr* expr_index_set(Arena* arena, Expr* object, Expr* index, Expr* value);
Expr* expr_list(Arena* arena, Expr** elements, int count, Span span);
Expr* expr_function(Arena* arena, Token* params, int param_count,
                     TypeExpr** param_types, TypeExpr* return_type,
                     Stmt* body, Span span);
Expr* expr_vec2(Arena* arena, Expr* x, Expr* y, Span span);
Expr* expr_postfix(Arena* arena, Expr* operand, Token op);

// ============================================================================
// Statement Constructors
// ============================================================================

Stmt* stmt_expression(Arena* arena, Expr* expression);
Stmt* stmt_assignment(Arena* arena, Expr* target, Expr* value);
Stmt* stmt_block(Arena* arena, Stmt** statements, int count, Span span);
Stmt* stmt_if(Arena* arena, Expr* condition, Stmt* then_branch, Stmt* else_branch, Span span);
Stmt* stmt_while(Arena* arena, Expr* condition, Stmt* body, Span span);
Stmt* stmt_for(Arena* arena, Token name, Expr* iterable, Stmt* body, Span span);
Stmt* stmt_return(Arena* arena, Expr* value, Span span);
Stmt* stmt_break(Arena* arena, Span span);
Stmt* stmt_continue(Arena* arena, Span span);
Stmt* stmt_function(Arena* arena, Token name, Token* params, int param_count,
                     TypeExpr** param_types, TypeExpr* return_type,
                     Stmt* body, Span span);
Stmt* stmt_struct(Arena* arena, Token name, Token* fields, int field_count,
                  TypeExpr** field_types, Stmt** methods, int method_count, Span span);
Stmt* stmt_var_decl(Arena* arena, Token name, TypeExpr* type, Expr* initializer, Span span);

// ============================================================================
// Visitor Pattern
// ============================================================================

typedef void (*ExprVisitFn)(Expr* expr, void* ctx);
typedef void (*StmtVisitFn)(Stmt* stmt, void* ctx);

typedef struct {
    ExprVisitFn visit_literal_null;
    ExprVisitFn visit_literal_bool;
    ExprVisitFn visit_literal_number;
    ExprVisitFn visit_literal_string;
    ExprVisitFn visit_identifier;
    ExprVisitFn visit_unary;
    ExprVisitFn visit_binary;
    ExprVisitFn visit_call;
    ExprVisitFn visit_get;
    ExprVisitFn visit_set;
    ExprVisitFn visit_index;
    ExprVisitFn visit_index_set;
    ExprVisitFn visit_list;
    ExprVisitFn visit_function;
    ExprVisitFn visit_vec2;
    ExprVisitFn visit_postfix;
    void* context;
} ExprVisitor;

typedef struct {
    StmtVisitFn visit_expression;
    StmtVisitFn visit_assignment;
    StmtVisitFn visit_block;
    StmtVisitFn visit_if;
    StmtVisitFn visit_while;
    StmtVisitFn visit_for;
    StmtVisitFn visit_return;
    StmtVisitFn visit_break;
    StmtVisitFn visit_continue;
    StmtVisitFn visit_function;
    StmtVisitFn visit_struct;
    StmtVisitFn visit_var_decl;
    void* context;
} StmtVisitor;

// Dispatch to appropriate visitor function
void expr_accept(Expr* expr, ExprVisitor* visitor);
void stmt_accept(Stmt* stmt, StmtVisitor* visitor);

// ============================================================================
// Debug Utilities
// ============================================================================

// Get the name of an expression type
const char* expr_type_name(ExprType type);

// Get the name of a statement type
const char* stmt_type_name(StmtType type);

// Print AST as indented tree
void ast_print_expr(Expr* expr, int indent);
void ast_print_stmt(Stmt* stmt, int indent);

#endif // PH_AST_H
