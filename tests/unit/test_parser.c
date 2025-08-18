===================================================================

int main(void) {
    TEST_SUITE("Parser - Literals");
    RUN_TEST(parse_number);
    RUN_TEST(parse_float);
    RUN_TEST(parse_string);
    RUN_TEST(parse_true);
    RUN_TEST(parse_false);
    RUN_TEST(parse_null);
    RUN_TEST(parse_identifier);

    TEST_SUITE("Parser - Binary Expressions");
    RUN_TEST(parse_binary_add);
    RUN_TEST(parse_binary_precedence);
    RUN_TEST(parse_binary_left_associative);
    RUN_TEST(parse_comparison);
    RUN_TEST(parse_equality);
    RUN_TEST(parse_logical_and);
    RUN_TEST(parse_logical_or);
    RUN_TEST(parse_and_or_precedence);

    TEST_SUITE("Parser - Unary Expressions");
    RUN_TEST(parse_unary_minus);
    RUN_TEST(parse_unary_not);
    RUN_TEST(parse_unary_precedence);

    TEST_SUITE("Parser - Grouping");
    RUN_TEST(parse_grouping);

    TEST_SUITE("Parser - Call Expressions");
    RUN_TEST(parse_call_no_args);
    RUN_TEST(parse_call_with_args);
    RUN_TEST(parse_chained_calls);

    TEST_SUITE("Parser - Property Access");
    RUN_TEST(parse_property_get);
    RUN_TEST(parse_chained_property);
    RUN_TEST(parse_method_call);

    TEST_SUITE("Parser - Index Expressions");
    RUN_TEST(parse_index);
    RUN_TEST(parse_chained_index);

    TEST_SUITE("Parser - List Expressions");
    RUN_TEST(parse_empty_list);
    RUN_TEST(parse_list);

    TEST_SUITE("Parser - Function Expressions");
    RUN_TEST(parse_function_expr);

    TEST_SUITE("Parser - Statements");
    RUN_TEST(parse_assignment);
    RUN_TEST(parse_compound_assignment);
    RUN_TEST(parse_if_statement);
    RUN_TEST(parse_if_else);
    RUN_TEST(parse_while_statement);
    RUN_TEST(parse_for_statement);
    RUN_TEST(parse_return_with_value);
    RUN_TEST(parse_return_bare);
    RUN_TEST(parse_break_continue);

    TEST_SUITE("Parser - Declarations");
    RUN_TEST(parse_function_decl);
    RUN_TEST(parse_function_no_params);
    RUN_TEST(parse_struct_decl);

    TEST_SUITE("Parser - Error Handling");
    RUN_TEST(parse_error_missing_paren);
    RUN_TEST(parse_error_recovery);
    RUN_TEST(parse_empty_input);

    TEST_SUITE("Parser - Complex Cases");
    RUN_TEST(parse_complex_expression);
    RUN_TEST(parse_multiple_statements);
    RUN_TEST(parse_nested_function);

    TEST_SUMMARY();
}
