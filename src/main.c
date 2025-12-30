#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/common.h"
#include "core/arena.h"
#include "compiler/parser.h"
#include "compiler/analyzer.h"
#include "compiler/codegen.h"
#include "vm/vm.h"
#include "runtime/stdlib.h"
#include "engine/engine.h"
#include "engine/engine_natives.h"
#include "pal/pal.h"

#define VERSION "1.0.0"

// ============================================================================
// File Reading
// ============================================================================

static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", path);
        return NULL;
    }

    // Get file size
    fseek(file, 0L, SEEK_END);
    size_t file_size = (size_t)ftell(file);
    rewind(file);

    // Allocate buffer
    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Not enough memory to read '%s'\n", path);
        fclose(file);
        return NULL;
    }

    // Read file
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Error: Could not read file '%s'\n", path);
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

// ============================================================================
// Builtin Declarations for Analyzer
// ============================================================================

static void declare_builtins(Analyzer* analyzer) {
    // Standard library functions
    analyzer_declare_global(analyzer, "print");
    analyzer_declare_global(analyzer, "println");
    analyzer_declare_global(analyzer, "type");
    analyzer_declare_global(analyzer, "to_string");
    analyzer_declare_global(analyzer, "to_number");
    analyzer_declare_global(analyzer, "abs");
    analyzer_declare_global(analyzer, "floor");
    analyzer_declare_global(analyzer, "ceil");
    analyzer_declare_global(analyzer, "round");
    analyzer_declare_global(analyzer, "min");
    analyzer_declare_global(analyzer, "max");
    analyzer_declare_global(analyzer, "clamp");
    analyzer_declare_global(analyzer, "sqrt");
    analyzer_declare_global(analyzer, "pow");
    analyzer_declare_global(analyzer, "sin");
    analyzer_declare_global(analyzer, "cos");
    analyzer_declare_global(analyzer, "tan");
    analyzer_declare_global(analyzer, "atan2");
    analyzer_declare_global(analyzer, "random");
    analyzer_declare_global(analyzer, "random_range");
    analyzer_declare_global(analyzer, "random_int");
    analyzer_declare_global(analyzer, "len");
    analyzer_declare_global(analyzer, "push");
    analyzer_declare_global(analyzer, "pop");
    analyzer_declare_global(analyzer, "insert");
    analyzer_declare_global(analyzer, "remove");
    analyzer_declare_global(analyzer, "contains");
    analyzer_declare_global(analyzer, "index_of");
    analyzer_declare_global(analyzer, "substring");
    analyzer_declare_global(analyzer, "split");
    analyzer_declare_global(analyzer, "join");
    analyzer_declare_global(analyzer, "upper");
    analyzer_declare_global(analyzer, "lower");
    analyzer_declare_global(analyzer, "range");
    analyzer_declare_global(analyzer, "time");
    analyzer_declare_global(analyzer, "clock");
    analyzer_declare_global(analyzer, "vec2");

    // Engine functions
    analyzer_declare_global(analyzer, "rgb");
    analyzer_declare_global(analyzer, "rgba");
    analyzer_declare_global(analyzer, "create_window");
    analyzer_declare_global(analyzer, "set_title");
    analyzer_declare_global(analyzer, "window_width");
    analyzer_declare_global(analyzer, "window_height");
    analyzer_declare_global(analyzer, "clear");
    analyzer_declare_global(analyzer, "draw_rect");
    analyzer_declare_global(analyzer, "draw_circle");
    analyzer_declare_global(analyzer, "draw_line");
    analyzer_declare_global(analyzer, "key_down");
    analyzer_declare_global(analyzer, "key_pressed");
    analyzer_declare_global(analyzer, "mouse_x");
    analyzer_declare_global(analyzer, "mouse_y");
    analyzer_declare_global(analyzer, "mouse_down");
    analyzer_declare_global(analyzer, "delta_time");
    analyzer_declare_global(analyzer, "game_time");

    // Image and sprite functions
    analyzer_declare_global(analyzer, "load_image");
    analyzer_declare_global(analyzer, "image_width");
    analyzer_declare_global(analyzer, "image_height");
    analyzer_declare_global(analyzer, "draw_image");
    analyzer_declare_global(analyzer, "draw_image_ex");
    analyzer_declare_global(analyzer, "create_sprite");
    analyzer_declare_global(analyzer, "draw_sprite");
    analyzer_declare_global(analyzer, "set_sprite_frame");

    // Font and text functions
    analyzer_declare_global(analyzer, "load_font");
    analyzer_declare_global(analyzer, "default_font");
    analyzer_declare_global(analyzer, "draw_text");
    analyzer_declare_global(analyzer, "text_width");
    analyzer_declare_global(analyzer, "text_height");

    // Color constants
    analyzer_declare_global(analyzer, "RED");
    analyzer_declare_global(analyzer, "GREEN");
    analyzer_declare_global(analyzer, "BLUE");
    analyzer_declare_global(analyzer, "WHITE");
    analyzer_declare_global(analyzer, "BLACK");
    analyzer_declare_global(analyzer, "YELLOW");
    analyzer_declare_global(analyzer, "CYAN");
    analyzer_declare_global(analyzer, "MAGENTA");
    analyzer_declare_global(analyzer, "ORANGE");
    analyzer_declare_global(analyzer, "PURPLE");
    analyzer_declare_global(analyzer, "GRAY");
    analyzer_declare_global(analyzer, "GREY");

    // Key constants
    analyzer_declare_global(analyzer, "KEY_UP");
    analyzer_declare_global(analyzer, "KEY_DOWN");
    analyzer_declare_global(analyzer, "KEY_LEFT");
    analyzer_declare_global(analyzer, "KEY_RIGHT");
    analyzer_declare_global(analyzer, "KEY_SPACE");
    analyzer_declare_global(analyzer, "KEY_RETURN");
    analyzer_declare_global(analyzer, "KEY_ESCAPE");
    analyzer_declare_global(analyzer, "KEY_TAB");
    analyzer_declare_global(analyzer, "KEY_A");
    analyzer_declare_global(analyzer, "KEY_B");
    analyzer_declare_global(analyzer, "KEY_C");
    analyzer_declare_global(analyzer, "KEY_D");
    analyzer_declare_global(analyzer, "KEY_E");
    analyzer_declare_global(analyzer, "KEY_F");
    analyzer_declare_global(analyzer, "KEY_G");
    analyzer_declare_global(analyzer, "KEY_H");
    analyzer_declare_global(analyzer, "KEY_I");
    analyzer_declare_global(analyzer, "KEY_J");
    analyzer_declare_global(analyzer, "KEY_K");
    analyzer_declare_global(analyzer, "KEY_L");
    analyzer_declare_global(analyzer, "KEY_M");
    analyzer_declare_global(analyzer, "KEY_N");
    analyzer_declare_global(analyzer, "KEY_O");
    analyzer_declare_global(analyzer, "KEY_P");
    analyzer_declare_global(analyzer, "KEY_Q");
    analyzer_declare_global(analyzer, "KEY_R");
    analyzer_declare_global(analyzer, "KEY_S");
    analyzer_declare_global(analyzer, "KEY_T");
    analyzer_declare_global(analyzer, "KEY_U");
    analyzer_declare_global(analyzer, "KEY_V");
    analyzer_declare_global(analyzer, "KEY_W");
    analyzer_declare_global(analyzer, "KEY_X");
    analyzer_declare_global(analyzer, "KEY_Y");
    analyzer_declare_global(analyzer, "KEY_Z");
    analyzer_declare_global(analyzer, "KEY_0");
    analyzer_declare_global(analyzer, "KEY_1");
    analyzer_declare_global(analyzer, "KEY_2");
    analyzer_declare_global(analyzer, "KEY_3");
    analyzer_declare_global(analyzer, "KEY_4");
    analyzer_declare_global(analyzer, "KEY_5");
    analyzer_declare_global(analyzer, "KEY_6");
    analyzer_declare_global(analyzer, "KEY_7");
    analyzer_declare_global(analyzer, "KEY_8");
    analyzer_declare_global(analyzer, "KEY_9");

    // Mouse button constants
    analyzer_declare_global(analyzer, "MOUSE_LEFT");
    analyzer_declare_global(analyzer, "MOUSE_MIDDLE");
    analyzer_declare_global(analyzer, "MOUSE_RIGHT");
}

// ============================================================================
// Commands
// ============================================================================

static int cmd_run(const char* filename) {
    // 1. Read source file
    char* source = read_file(filename);
    if (!source) {
        return 1;
    }

    // 2. Parse source to AST
    Arena* arena = arena_new(64 * 1024);
    Parser parser;
    parser_init(&parser, source, arena);

    int stmt_count = 0;
    Stmt** stmts = parser_parse(&parser, &stmt_count);

    if (parser_had_error(&parser)) {
        arena_free(arena);
        free(source);
        return 1;
    }

    // 3. Semantic analysis
    Analyzer analyzer;
    analyzer_init(&analyzer, filename, source);
    declare_builtins(&analyzer);

    if (!analyzer_analyze(&analyzer, stmts, stmt_count)) {
        analyzer_print_errors(&analyzer, stderr);
        analyzer_free(&analyzer);
        arena_free(arena);
        free(source);
        return 1;
    }
    analyzer_free(&analyzer);

    // 4. Compile to bytecode
    Codegen codegen;
    codegen_init(&codegen, filename, source);

    ObjFunction* function = codegen_compile(&codegen, stmts, stmt_count);
    if (!function) {
        codegen_print_errors(&codegen, stderr);
        codegen_free(&codegen);
        arena_free(arena);
        free(source);
        return 1;
    }
    codegen_free(&codegen);

    // 5. Initialize VM
    VM vm;
    vm_init(&vm);
    stdlib_init(&vm);

    // 6. Initialize engine
    Engine* engine = engine_new(&vm);
    engine_set(engine);

    // Use SDL2 if available, fall back to mock
#ifdef PAL_USE_SDL2
    if (!engine_init(engine, PAL_BACKEND_SDL2)) {
        fprintf(stderr, "Warning: Failed to initialize SDL2, using mock backend\n");
        engine_init(engine, PAL_BACKEND_MOCK);
    }
#else
    engine_init(engine, PAL_BACKEND_MOCK);
#endif

    engine_natives_init(&vm);

    // 7. Run top-level code
    InterpretResult result = vm_interpret(&vm, function);

    // 8. If successful and has game callbacks, run game loop
    if (result == INTERPRET_OK) {
        engine_detect_callbacks(engine);
        if (engine_has_callbacks(engine)) {
            engine_run(engine);
        }
    }

    // 9. Cleanup
    engine_shutdown(engine);
    engine_free(engine);
    vm_free(&vm);
    arena_free(arena);
    free(source);

    return result == INTERPRET_OK ? 0 : 1;
}

// ============================================================================
// Main Entry Point
// ============================================================================

static void print_usage(const char* program) {
    fprintf(stderr, "Usage: %s <command> [options] [file]\n\n", program);
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  run <file>      Run a Pixel script\n");
    fprintf(stderr, "  compile <file>  Compile to bytecode\n");
    fprintf(stderr, "  disasm <file>   Disassemble bytecode\n");
    fprintf(stderr, "  version         Print version\n");
    fprintf(stderr, "  help            Show this message\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "version") == 0) {
        printf("Pixel %s\n", VERSION);
        return 0;
    }

    if (strcmp(argv[1], "help") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "run") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: 'run' requires a file argument\n");
            return 1;
        }
        return cmd_run(argv[2]);
    }

    if (strcmp(argv[1], "compile") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: 'compile' requires a file argument\n");
            return 1;
        }
        printf("Compiling: %s (not yet implemented)\n", argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "disasm") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: 'disasm' requires a file argument\n");
            return 1;
        }
        printf("Disassembling: %s (not yet implemented)\n", argv[2]);
        return 0;
    }

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    print_usage(argv[0]);
    return 1;
}
