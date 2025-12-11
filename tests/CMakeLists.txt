_link_libraries(test_vm pixel_compiler)
add_test(NAME test_vm COMMAND test_vm)

add_executable(test_gc unit/test_gc.c)
target_link_libraries(test_gc pixel_vm)
add_test(NAME test_gc COMMAND test_gc)

add_executable(test_stdlib unit/test_stdlib.c)
target_link_libraries(test_stdlib pixel_compiler pixel_runtim# Test executables
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
target