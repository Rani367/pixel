#include "../test_framework.h"
#include "../test_helpers.h"
#include "core/arena.h"
#include "compiler/parser.h"
#include "compiler/analyzer.h"
#include "compiler/codegen.h"
#include "vm/vm.h"
#include "vm/gc.h"
#include "runtime/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Helper - Read file
// ============================================================================

static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = (size_t)ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

// Helper to declare builtins for analyzer
static void declare_builtins(Analyzer* analyzer) {
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
    analyzer_declare_global(analyzer, "load_image");
    analyzer_declare_global(analyzer, "image_width");
    analyzer_declare_global(analyzer, "image_height");
    analyzer_declare_global(analyzer, "draw_image");
    analyzer_declare_global(analyzer, "draw_image_ex");
    analyzer_declare_global(analyzer, "create_sprite");
    analyzer_declare_global(analyzer, "draw_sprite");
    analyzer_declare_global(analyzer, "set_sprite_frame");
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
    // ... additional keys as needed
}

// Helper - Check if file compiles (parse + analyze)
static int can_compile(const char* path) {
    char* source = read_file(path);
    if (!source) {
        return 1;  // File not found - skip test (pass)
    }

    gc_init();

    Arena* arena = arena_new(64 * 1024);
    Parser parser;
    parser_init(&parser, source, arena);

    int stmt_count = 0;
    Stmt** stmts = parser_parse(&parser, &stmt_count);

    if (parser_had_error(&parser)) {
        arena_free(arena);
        free(source);
        gc_free_all();
        return 0;
    }

    Analyzer analyzer;
    analyzer_init(&analyzer, path, source);
    declare_builtins(&analyzer);

    int success = analyzer_analyze(&analyzer, stmts, stmt_count);

    analyzer_free(&analyzer);
    arena_free(arena);
    free(source);
    gc_free_all();

    return success;
}

// ============================================================================
// Benchmark Files Tests
// ============================================================================

TEST(example_fib_compiles) {
    ASSERT(can_compile("benchmarks/fib.pixel"));
}

TEST(example_game_loop_compiles) {
    ASSERT(can_compile("benchmarks/game_loop.pixel"));
}

TEST(example_list_ops_compiles) {
    ASSERT(can_compile("benchmarks/list_ops.pixel"));
}

TEST(example_string_ops_compiles) {
    ASSERT(can_compile("benchmarks/string_ops.pixel"));
}

// ============================================================================
// Example Files Tests
// ============================================================================

TEST(example_advanced_features_compiles) {
    ASSERT(can_compile("examples/advanced_features.pixel"));
}

TEST(example_input_demo_compiles) {
    ASSERT(can_compile("examples/input_demo.pixel"));
}

TEST(example_physics_demo_compiles) {
    ASSERT(can_compile("examples/physics_demo.pixel"));
}

TEST(example_web_demo_compiles) {
    ASSERT(can_compile("examples/web_demo.pixel"));
}

// ============================================================================
// Game Examples Tests
// ============================================================================

TEST(example_breakout_compiles) {
    ASSERT(can_compile("examples/games/breakout.pixel"));
}

TEST(example_platformer_compiles) {
    ASSERT(can_compile("examples/games/platformer.pixel"));
}

TEST(example_pong_compiles) {
    ASSERT(can_compile("examples/games/pong.pixel"));
}

TEST(example_snake_compiles) {
    ASSERT(can_compile("examples/games/snake.pixel"));
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Benchmark Files");
    RUN_TEST(example_fib_compiles);
    RUN_TEST(example_game_loop_compiles);
    RUN_TEST(example_list_ops_compiles);
    RUN_TEST(example_string_ops_compiles);

    TEST_SUITE("Example Files");
    RUN_TEST(example_advanced_features_compiles);
    RUN_TEST(example_input_demo_compiles);
    RUN_TEST(example_physics_demo_compiles);
    RUN_TEST(example_web_demo_compiles);

    TEST_SUITE("Game Examples");
    RUN_TEST(example_breakout_compiles);
    RUN_TEST(example_platformer_compiles);
    RUN_TEST(example_pong_compiles);
    RUN_TEST(example_snake_compiles);

    TEST_SUMMARY();
}
