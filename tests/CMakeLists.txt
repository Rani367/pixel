# Test executables
add_executable(test_arena unit/test_arena.c)
target_link_libraries(test_arena pixel_core)
add_test(NAME test_arena COMMAND test_arena)

add_executable(test_array unit/test_array.c)
target_link_libraries(test_array pixel_core)
add_test(NAME test_array COMMAND test_array)

add_executable(test_strings unit/test_strings.c)
target_link_libraries(test_strings pixel_core)
add_test(NAME test_strings COMMAND test_strings)

add_executable(test_table unit/test_table.c)
target_link_libraries(test_table pixel_core)
add_test(NAME test_table COMMAND test_table)

add_executable(test_lexer unit/test_lexer.c)
target_link_libraries(test_lexer pixel_compiler)
add_test(NAME test_lexer COMMAND test_lexer)

add_executable(test_ast unit/test_ast.c)
target_link_libraries(test_ast pixel_compiler)
add_test(NAME test_ast COMMAND test_ast)

add_executable(test_parser unit/test_parser.c)
target_link_libraries(test_parser pixel_compiler)
add_test(NAME test_parser COMMAND test_parser)

add_executable(test_analyzer unit/test_analyzer.c)
target_link_libraries(test_analyzer pixel_compiler)
add_test(NAME test_analyzer COMMAND test_analyzer)

add_executable(test_value unit/test_value.c)
target_link_libraries(test_value pixel_vm)
add_test(NAME test_value COMMAND test_value)

add_executable(test_chunk unit/test_chunk.c)
target_link_libraries(test_chunk pixel_vm)
add_test(NAME test_chunk COMMAND test_chunk)

add_executable(test_codegen unit/test_codegen.c)
target_link_libraries(test_codegen pixel_compiler)
add_test(NAME test_codegen COMMAND test_codegen)

add_executable(test_vm unit/test_vm.c)
target_link_libraries(test_vm pixel_compiler pixel_runtime)
add_test(NAME test_vm COMMAND test_vm)

add_executable(test_gc unit/test_gc.c)
target_link_libraries(test_gc pixel_vm)
add_test(NAME test_gc COMMAND test_gc)

add_executable(test_stdlib unit/test_stdlib.c)
target_link_libraries(test_stdlib pixel_compiler pixel_runtime)
add_test(NAME test_stdlib COMMAND test_stdlib)

add_executable(test_pal unit/test_pal.c)
target_link_libraries(test_pal pixel_pal)
add_test(NAME test_pal COMMAND test_pal)

add_executable(test_engine unit/test_engine.c)
target_link_libraries(test_engine pixel_engine pixel_compiler)
add_test(NAME test_engine COMMAND test_engine)

add_executable(test_engine_game_loop unit/test_engine_game_loop.c)
target_link_libraries(test_engine_game_loop pixel_core pixel_compiler pixel_vm pixel_runtime pixel_engine pixel_pal)
add_test(NAME test_engine_game_loop COMMAND test_engine_game_loop)

add_executable(test_physics unit/test_physics.c)
target_link_libraries(test_physics pixel_engine pixel_compiler)
add_test(NAME test_physics COMMAND test_physics)

# New unit tests for 100% coverage
add_executable(test_error unit/test_error.c)
target_link_libraries(test_error pixel_core)
add_test(NAME test_error COMMAND test_error)

add_executable(test_log unit/test_log.c)
target_link_libraries(test_log pixel_core)
add_test(NAME test_log COMMAND test_log)

add_executable(test_token unit/test_token.c)
target_link_libraries(test_token pixel_compiler)
add_test(NAME test_token COMMAND test_token)

add_executable(test_symbols unit/test_symbols.c)
target_link_libraries(test_symbols pixel_compiler)
add_test(NAME test_symbols COMMAND test_symbols)

add_executable(test_object unit/test_object.c)
target_link_libraries(test_object pixel_vm)
add_test(NAME test_object COMMAND test_object)

add_executable(test_opcodes unit/test_opcodes.c)
target_link_libraries(test_opcodes pixel_vm)
add_test(NAME test_opcodes COMMAND test_opcodes)

add_executable(test_debug unit/test_debug.c)
target_link_libraries(test_debug pixel_vm)
add_test(NAME test_debug COMMAND test_debug)

add_executable(test_main unit/test_main.c)
target_link_libraries(test_main pixel_core pixel_compiler pixel_vm pixel_runtime pixel_engine)
add_test(NAME test_main COMMAND test_main)

add_executable(test_engine_natives unit/test_engine_natives.c)
target_link_libraries(test_engine_natives pixel_core pixel_compiler pixel_vm pixel_runtime pixel_engine pixel_pal)
add_test(NAME test_engine_natives COMMAND test_engine_natives)

# Integration tests
add_executable(test_e2e integration/test_e2e.c)
target_link_libraries(test_e2e pixel_core pixel_compiler pixel_vm pixel_runtime pixel_engine)
add_test(NAME test_e2e COMMAND test_e2e)

add_executable(test_examples integration/test_examples.c)
target_link_libraries(test_examples pixel_core pixel_compiler pixel_vm pixel_runtime pixel_engine)
add_test(NAME test_examples COMMAND test_examples)

# SDL2 headless tests (only if SDL2 is available)
if(SDL2_FOUND AND SDL2_image_FOUND AND SDL2_mixer_FOUND AND SDL2_ttf_FOUND)
    add_executable(test_pal_sdl integration/test_pal_sdl.c)
    target_compile_definitions(test_pal_sdl PRIVATE PAL_USE_SDL2)
    target_link_libraries(test_pal_sdl pixel_pal SDL2::SDL2 SDL2_image::SDL2_image SDL2_mixer::SDL2_mixer SDL2_ttf::SDL2_ttf)
    target_include_directories(test_pal_sdl PRIVATE ${SDL2_INCLUDE_DIRS})
    add_test(NAME test_pal_sdl COMMAND test_pal_sdl)
    set_tests_properties(test_pal_sdl PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy"
    )
endif()
