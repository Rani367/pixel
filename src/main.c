;
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

    // 