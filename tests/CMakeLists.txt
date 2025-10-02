_link_libraries(test_vm pixel_compiler)
add_test(NAME test_vm COMMAND test_vm)

add_executable(test_gc unit/test_gc.c)
target_link_libraries(test_gc pixel_vm)
add_test(NAME test_gc COMMAND test_gc)

add_executable(test_stdlib unit/test_stdlib.c)
target_link_libraries(test_stdlib pixel_compiler pixel_runtim