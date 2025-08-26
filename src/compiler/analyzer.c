#include "compiler/analyzer.h"
#include "core/strings.h"
#include <string.h>
#include <stdarg.h>

// Forward declarations for recursive analysis
static void analyze_stmt(Analyzer* analyzer, Stmt* stmt);
static void analyze_expr(Analyzer* analyzer, Expr* expr);

// ============================================================================
// Error Reporting
// ============================================================================

static SourceLocation location_from_span(Analyzer* analyzer, Span span) {
    return source_location_new(analyzer->source_file,
                               span.start_line,
                               span.start_column,
                               span.end_column - span.start_column);
}

static SourceLocation location_from_token(Analyzer* analyzer, Token token) {
    return source_location_new(analyzer->source_file,
                               token.line,
                               token.column,
                               token.length);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 4, 5)))
#endif
static void report_error(Analyzer* analyzer, SourceLocation loc,
                         ErrorCode code, const char* fmt, ...) {
    if (analyzer->error_count >= ANALYZER_MAX_ERRORS) {
        return;
    }

    va_list args;
    va_start(args, fmt);

    // Format the message
    char buffer[512];
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    vsnprintf(buffer, sizeof(buffer), fmt, args);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    va_end(args);

    analyzer->errors[analyzer->error_count++] = error_new(code, loc, "%s", buffer);
}

// Built-in function names for "did you mean?" suggestions
static const char* builtin_functions[] = {
    // I/O
    "print", "println",
    // Type
    "type", "to_string", "to_number",
    // Math
    "abs", "floor", "ceil", "round", "min", "max", "clamp",
    "sqrt", "pow", "sin", "cos", "tan", "atan2",
    "random", "random_range", "random_int",
    // List
    "len", "push", "pop", "insert", "remove", "contains", "index_of",
    // String
    "substring", "split", "join", "upper", "lower",
    // Utility
    "range", "time", "clock",
    // Colors
    "rgb", "rgba",
    // Window
    "create_window", "set_title", "window_width", "window_height",
    // Drawing
    "clear", "draw_rect", "draw_circle", "draw_line",
    "draw_image", "draw_image_ex", "draw_sprite", "draw_text",
    // Input
    "key_down", "key_pressed", "key_released",
    "mouse_x", "mouse_y", "mouse_down", "mouse_pressed", "mouse_released",
    // Timing
    "delta_time", "game_time",
    // Images/Sprites
    "load_image", "image_width", "image_height",
    "create_sprite", "set_sprite_frame",
    // Fonts
    "load_font", "default_font", "text_width", "text_height",
    // Audio
    "load_sound", "play_sound", "play_sound_volume",
    "load_music", "play_music", "play_music_loop",
    "pause_music", "resume_music", "stop_music",
    "set_music_volume", "set_master_volume", "music_playing",
    // Physics
    "set_gravity", "get_gravity",
    "collides", "collides_rect", "collides_point", "collides_circle",
    "distance", "apply_force", "move_toward", "look_at",
    "lerp", "lerp_angle",
    // Camera
    "camera", "camera_x", "camera_y", "camera_zoom",
    "camera_set_position", "camera_set_zoom", "camera_follow", "camera_shake",
    "screen_to_world_x", "screen_to_world_y",
    "world_to_screen_x", "world_to_screen_y",
    // Animation
    "create_animation", "animation_play", "animation_stop", "animation_reset",
    "animation_set_looping", "animation_frame", "animation_playing",
    "sprite_set_animation", "sprite_play", "sprite_stop",
    // Scene
    "load_scene", "get_scene",
    // Particles
    "create_emitter", "emitter_emit", "emitter_set_color", "emitter_set_speed",
    "emitter_set_angle", "emitter_set_lifetime", "emitter_set_size",
    "emitter_set_gravity", "emitter_set_rate", "emitter_set_position",
    "emitter_set_active", "emitter_count", "draw_particles",
    NULL  // Sentinel
};

// Calculate simple edit distance (number of different characters)
static int simple_distance(const char* a, int len_a, const char* b, int len_b) {
    int max_len = PH_MAX(len_a, len_b);
    int min_len = PH_MIN(len_a, len_b);
    int matches = 0;

    for (int i = 0; i < min_len; i++) {
        if (a[i] == b[i]) matches++;
    }

    return max_len - matches;
}

// Check if a name is similar enough to suggest
static bool is_similar(const char* name, int length, const char* candidate, int cand_len) {
    // Must start with same letter and be similar length
    if (length <= 0 || cand_len <= 0) return false;
    if (name[0] != candidate[0]) return false;
    if (abs(cand_len - length) > 2) return false;
    return true;
}

// Find a similar symbol name for "did you mean?" suggestions
// Returns a static Symbol (for builtins) or a scope Symbol
static Symbol builtin_symbol;  // Static storage for builtin suggestions

static Symbol* find_similar_symbol(Analyzer* analyzer, const char* name, int length) {
    Scope* scope = analyzer->current_scope;
    Symbol* best_match = NULL;
    int best_distance = 3;  // Only suggest if <= 2 edits away
    const char* best_builtin = NULL;

    // Search scope symbols
    while (scope != NULL) {
        for (int i = 0; i < scope->count; i++) {
            Symbol* sym = &scope->symbols[i];

            if (is_similar(name, length, sym->name, sym->length)) {
                int distance = simple_distance(name, length, sym->name, sym->length);
                if (distance < best_distance) {
                    best_distance = distance;
                    best_match = sym;
                    best_builtin = NULL;  // Prefer scope symbols
                }
            }
        }
        scope = scope->enclosing;
    }

    // Search built-in function names
    for (int i = 0; builtin_functions[i] != NULL; i++) {
        const char* builtin = builtin_functions[i];
        int builtin_len = (int)strlen(builtin);

        if (is_similar(name, length, builtin, builtin_len)) {
            int distance = simple_distance(name, length, builtin, builtin_len);
            if (distance < best_distance) {
                best_distance = distance;
                best_match = NULL;
                best_builtin = builtin;
            }
        }
    }

    // If we found a builtin match, create a static symbol for it
    if (best_builtin != NULL) {
        builtin_symbol.name = best_builtin;
        builtin_symbol.length = (int)strlen(best_builtin);
        builtin_symbol.kind = SYMBOL_FUNCTION;
        builtin_symbol.slot = -1;
        builtin_symbol.is_initialized = true;
        return &builtin_symbol;
    }

    return best_match;
}

// ============================================================================
// Scope Management
// ============================================================================

static void begin_scope(Analyzer* analyzer) {
    Scope* new_scope = PH_ALLOC(sizeof(Scope));
    scope_init(new_scope, analyzer->current_scope->depth + 1,
               analyzer->current_scope);
    analyzer->current_scope = new_scope;
}

static void end_scope(Analyzer* analyzer) {
    Scope* old_scope = analyzer->current_scope;
    analyzer->current_scope = old_scope->enclosing;
    scope_free(old_scope);
    PH_FREE(old_scope);
}

static void declare_variable(Analyzer* analyzer, Token name, SymbolKind kind) {
    Scope* scope = analyzer->current_scope;

    // Check for redeclaration in the same scope
    Symbol* existing = scope_lookup_local(scope, name.start, name.length);
    if (existing != NULL) {
        report_error(analyzer, location_from_token(analyzer, name),
                     ERR_REDEFINED_VARIABLE,
                     "Variable '%.*s' is already declared in this scope",
                     name.length, name.start);
        return;
    }

    // Determine the slot based on kind
    int slot = -1;
    if (kind == SYMBOL_LOCAL || kind == SYMBOL_PARAMETER) {
        slot = analyzer->local_count++;
    }

    scope_add_symbol(scope, name.start, name.length, kind, slot);
}

static void define_variable(Analyzer* analyzer, Token name) {
    Symbol* symbol = scope_lookup(analyzer->current_scope, name.start, name.length);
    if (symbol != NULL) {
        symbol->is_initialized = true;
    }
}

static Symbol* resolve_variable(Analyzer* analyzer, Token name) {
    Symbol* symbol = scope_lookup(analyzer->current_scope, name.start, name.length);

    if (symbol == NULL) {
        // Variable not found - report error with suggestion
        Symbol* similar = find_similar_symbol(analyzer, name.start, name.length);
        if (similar != NULL) {
            report_error(analyzer, location_from_token(analyzer, name),
                         ERR_UNDEFINED_VARIABLE,
                         "Undefined variable '%.*s'. Did you mean '%.*s'?",
                         name.length, name.start,
                         similar->length, similar->name);
        } else {
            report_error(analyzer, location_from_token(analyzer, name),
                         ERR_UNDEFINED_VARIABLE,
                         "Undefined variable '%.*s'",
                         name.length, name.start);
        }
        return NULL;
    }

    return symbol;
}

// ============================================================================
// Expression Analysis
// ============================================================================

static void analyze_expr(Analyzer* analyzer, Expr* expr) {
    if (expr == NULL) return;

    switch (expr->type) {
        case EXPR_LITERAL_NULL:
        case EXPR_LITERAL_BOOL:
        case EXPR_LITERAL_NUMBER:
        case EXPR_LITERAL_STRING:
            // Literals need no analysis
            break;

        case EXPR_IDENTIFIER: {
            ExprIdentifier* id = (ExprIdentifier*)expr;
            resolve_variable(analyzer, id->name);
            break;
        }

        case EXPR_UNARY: {
            ExprUnary* unary = (ExprUnary*)expr;
            analyze_expr(analyzer, unary->operand);
            break;
        }

        case EXPR_BINARY: {
            ExprBinary* binary = (ExprBinary*)expr;
            analyze_expr(analyzer, binary->left);
            analyze_expr(analyzer, binary->right);
            break;
        }

        case EXPR_CALL: {
            ExprCall* call = (ExprCall*)expr;
            analyze_expr(analyzer, call->callee);
            for (int i = 0; i < call->arg_count; i++) {
                analyze_expr(analyzer, call->arguments[i]);
            }
            break;
        }

        case EXPR_GET: {
            ExprGet* get = (ExprGet*)expr;
            analyze_expr(analyzer, get->object);
            break;
        }

        case EXPR_SET: {
            ExprSet* set = (ExprSet*)expr;
            analyze_expr(analyzer, set->object);
            analyze_expr(analyzer, set->value);
            break;
        }

        case EXPR_INDEX: {
            ExprIndex* index = (ExprIndex*)expr;
            analyze_expr(analyzer, index->object);
            analyze_expr(analyzer, index->index);
            break;
        }

        case EXPR_INDEX_SET: {
            ExprIndexSet* index_set = (ExprIndexSet*)expr;
            analyze_expr(analyzer, index_set->object);
            analyze_expr(analyzer, index_set->index);
            analyze_expr(analyzer, index_set->value);
            break;
        }

        case EXPR_LIST: {
            ExprList* list = (ExprList*)expr;
            for (int i = 0; i < list->count; i++) {
                analyze_expr(analyzer, list->elements[i]);
            }
            break;
        }

        case EXPR_FUNCTION: {
            ExprFunction* fn = (ExprFunction*)expr;

            // Save state - functions reset loop depth (break/continue not allowed)
            int saved_local_count = analyzer->local_count;
            int saved_loop_depth = analyzer->loop_depth;
            analyzer->local_count = 0;
            analyzer->loop_depth = 0;
            analyzer->function_depth++;

            begin_scope(analyzer);

            // Declare parameters
            for (int i = 0; i < fn->param_count; i++) {
                declare_variable(analyzer, fn->params[i], SYMBOL_PARAMETER);
                define_variable(analyzer, fn->params[i]);
            }

            // Analyze body
            analyze_stmt(analyzer, fn->body);

            end_scope(analyzer);

            // Restore state
            analyzer->function_depth--;
            analyzer->loop_depth = saved_loop_depth;
            analyzer->local_count = saved_local_count;
            break;
        }

        case EXPR_VEC2: {
            ExprVec2* vec = (ExprVec2*)expr;
            analyze_expr(analyzer, vec->x);
            analyze_expr(analyzer, vec->y);
            break;
        }

        case EXPR_POSTFIX: {
            ExprPostfix* postfix = (ExprPostfix*)expr;
            analyze_expr(analyzer, postfix->operand);
            break;
        }

        case EXPR_COUNT:
            PH_UNREACHABLE();
    }
}

// ============================================================================
// Statement Analysis
// ============================================================================

static void analyze_stmt(Analyzer* analyzer, Stmt*scope != &analyzer->global_scope) {
        end_scope(analyzer);
    }

    // Free global scope
    scope_free(&analyzer->global_scope);

    // Free errors
    for (int i = 0; i < analyzer->error_count; i++) {
        if (analyzer->errors[i] != NULL) {
            error_free(analyzer->errors[i]);
            analyzer->errors[i] = NULL;
        }
    }
}

bool analyzer_analyze(Analyzer* analyzer, Stmt** statements, int count) {
    for (int i = 0; i < count; i++) {
        analyze_stmt(analyzer, statements[i]);
    }

    return analyzer->error_count == 0;
}

int analyzer_error_count(Analyzer* analyzer) {
    return analyzer->error_count;
}

Error* analyzer_get_error(Analyzer* analyzer, int index) {
    if (index < 0 || index >= analyzer->error_count) {
        return NULL;
    }
    return analyzer->errors[index];
}

void analyzer_print_errors(Analyzer* analyzer, FILE* out) {
    for (int i = 0; i < analyzer->error_count; i++) {
        error_print_pretty(analyzer->errors[i], analyzer->source, out);
    }
}

void analyzer_declare_global(Analyzer* analyzer, const char* name) {
    int length = (int)strlen(name);
    Symbol* existing = scope_lookup_local(&analyzer->global_scope, name, length);
    if (existing == NULL) {
        Symbol* sym = scope_add_symbol(&analyzer->global_scope, name, length,
                                       SYMBOL_FUNCTION, -1);
        sym->is_initialized = true;
    }
}
