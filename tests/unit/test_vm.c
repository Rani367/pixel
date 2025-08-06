       "result_y = p.y"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("result_x", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 10);

    ASSERT(get_global("result_y", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 20);

    teardown();
}

TEST(struct_positional_with_methods) {
    setup();
    InterpretResult result = run_source(
        "struct Player {\n"
        "    health, name,\n"
        "    function take_damage(amount) {\n"
        "        this.health = this.health - amount\n"
        "    }\n"
        "}\n"
        "p = Player(100, \"Hero\")\n"
        "p.take_damage(30)\n"
        "result = p.health"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("result", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 70);

    teardown();
}

// ============================================================================
// Increment/Decrement Tests
// ============================================================================

TEST(postfix_increment) {
    setup();
    InterpretResult result = run_source(
        "x = 5\n"
        "y = x++\n"
        "z = x"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("y", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 5);  // y gets old value

    ASSERT(get_global("z", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 6);  // x was incremented

    teardown();
}

TEST(postfix_decrement) {
    setup();
    InterpretResult result = run_source(
        "x = 10\n"
        "y = x--\n"
        "z = x"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("y", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 10);  // y gets old value

    ASSERT(get_global("z", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 9);   // x was decremented

    teardown();
}

TEST(postfix_in_loop) {
    setup();
    InterpretResult result = run_source(
        "i = 0\n"
        "sum = 0\n"
        "while i < 5 {\n"
        "    sum = sum + i++\n"
        "}\n"
        "result = sum"
    );
    ASSERT_EQ(result, INTERPRET_OK);

    Value* val;
    ASSERT(get_global("result", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 10);  // 0+1+2+3+4 = 10

    ASSERT(get_global("i", &val));
    ASSERT(IS_NUMBER(*val));
    ASSERT_EQ(AS_NUMBER(*val), 5);

    teardown();
}

// ============================================================================
// Runtime Error Tests
// ============================================================================

TEST(error_type_arithmetic) {
    setup();
    InterpretResult result = run_source("x = 1 + \"hello\"");
    ASSERT_EQ(result, INTERPRET_RUNTIME_ERROR);
    teardown();
}

TEST(error_undefined_variable) {
    setup();
    // Note: The analyzer catches undefined variables at compile time,
    // so this returns COMPILE_ERROR, not RUNTIME_ERROR
    InterpretResult result = run_source("x = undefined_var");
    ASSERT_EQ(result, INTERPRET_COMPILE_ERROR);
    teardown();
}

TEST(error_call_non_function) {
    setup();
    InterpretResult result = run_source("x = 42\nx()");
    ASSERT_EQ(result, INTERPRET_RUNTIME_ERROR);
    teardown();
}

TEST(error_wrong_arity) {
    setup();
    InterpretResult result = run_source(
        "function add(a, b) { return a + b }\n"
        "x = add(1)"
    );
    ASSERT_EQ(result, INTERPRET_RUNTIME_ERROR);
    teardown();
}

TEST(error_index_out_of_bounds) {
    setup();
    InterpretResult result = run_source(
        "arr = [1, 2, 3]\n"
        "x = arr[10]"
    );
    ASSERT_EQ(result, INTERPRET_RUNTIME_ERROR);
    teardown();
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("VM - Arithmetic");
    RUN_TEST(arithmetic_add);
    RUN_TEST(arithmetic_subtract);
    RUN_TEST(arithmetic_multiply);
    RUN_TEST(arithmetic_divide);
    RUN_TEST(arithmetic_modulo);
    RUN_TEST(arithmetic_negate);
    RUN_TEST(arithmetic_complex);

    TEST_SUITE("VM - Strings");
    RUN_TEST(string_concat);

    TEST_SUITE("VM - Comparisons");
    RUN_TEST(comparison_equal);
    RUN_TEST(comparison_not_equal);
    RUN_TEST(comparison_less);
    RUN_TEST(comparison_less_equal);
    RUN_TEST(comparison_greater);
    RUN_TEST(comparison_greater_equal);

    TEST_SUITE("VM - Logical");
    RUN_TEST(logical_not);
    RUN_TEST(logical_and);
    RUN_TEST(logical_or);

    TEST_SUITE("VM - Variables");
    RUN_TEST(global_variable);

    TEST_SUITE("VM - Control Flow");
    RUN_TEST(if_true);
    RUN_TEST(if_false);
    RUN_TEST(if_else_true);
    RUN_TEST(if_else_false);
    RUN_TEST(while_loop);
    RUN_TEST(nested_while);

    TEST_SUITE("VM - Functions");
    RUN_TEST(function_simple);
    RUN_TEST(function_recursion);
    RUN_TEST(function_fibonacci);

    TEST_SUITE("VM - Closures");
    RUN_TEST(closure_simple);
    RUN_TEST(closure_multiple);

    TEST_SUITE("VM - Lists");
    RUN_TEST(list_create);
    RUN_TEST(list_index_get);
    RUN_TEST(list_index_set);
    RUN_TEST(list_negative_index_get);
    RUN_TEST(list_negative_index_set);
    RUN_TEST(string_negative_index);

    TEST_SUITE("VM - Structs");
    RUN_TEST(struct_create);
    RUN_TEST(struct_property_access);
    RUN_TEST(struct_method_simple);
    RUN_TEST(struct_method_with_params);
    RUN_TEST(struct_method_return_value);
    RUN_TEST(struct_positional_constructor);
    RUN_TEST(struct_positional_with_methods);

    TEST_SUITE("VM - Increment/Decrement");
    RUN_TEST(postfix_increment);
    RUN_TEST(postfix_decrement);
    RUN_TEST(postfix_in_loop);

    TEST_SUITE("VM - Runtime Errors");
    RUN_TEST(error_type_arithmetic);
    RUN_TEST(error_undefined_variable);
    RUN_TEST(error_call_non_function);
    RUN_TEST(error_wrong_arity);
    RUN_TEST(error_index_out_of_bounds);

    TEST_SUMMARY();
}
