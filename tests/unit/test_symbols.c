#include "../test_framework.h"
#include "compiler/symbols.h"
#include <stdlib.h>

// ============================================================================
// Scope Lifecycle Tests
// ============================================================================

TEST(scope_init_sets_defaults) {
    Scope scope;
    scope_init(&scope, 0, NULL);

    ASSERT_NULL(scope.symbols);
    ASSERT_EQ(scope.count, 0);
    ASSERT_EQ(scope.capacity, 0);
    ASSERT_EQ(scope.depth, 0);
    ASSERT_NULL(scope.enclosing);

    scope_free(&scope);
}

TEST(scope_init_with_enclosing) {
    Scope outer;
    scope_init(&outer, 0, NULL);

    Scope inner;
    scope_init(&inner, 1, &outer);

    ASSERT_EQ(inner.depth, 1);
    ASSERT_EQ(inner.enclosing, &outer);

    scope_free(&inner);
    scope_free(&outer);
}

TEST(scope_free_empty) {
    Scope scope;
    scope_init(&scope, 0, NULL);
    scope_free(&scope);  // Should not crash

    ASSERT_NULL(scope.symbols);
    ASSERT_EQ(scope.count, 0);
    ASSERT_EQ(scope.capacity, 0);
}

TEST(scope_free_with_symbols) {
    Scope scope;
    scope_init(&scope, 0, NULL);

    // Add some symbols
    scope_add_symbol(&scope, "x", 1, SYMBOL_LOCAL, 0);
    scope_add_symbol(&scope, "y", 1, SYMBOL_LOCAL, 1);
    scope_add_symbol(&scope, "z", 1, SYMBOL_LOCAL, 2);

    ASSERT_EQ(scope.count, 3);

    scope_free(&scope);
    ASSERT_NULL(scope.symbols);
    ASSERT_EQ(scope.count, 0);
}

// ============================================================================
// Symbol Management Tests
// ============================================================================

TEST(scope_add_symbol_first) {
    Scope scope;
    scope_init(&scope, 0, NULL);

    Symbol* sym = scope_add_symbol(&scope, "variable", 8, SYMBOL_LOCAL, 0);

    ASSERT_NOT_NULL(sym);
    ASSERT_EQ(scope.count, 1);
    ASSERT(names_equal(sym->name, sym->length, "variable", 8));
    ASSERT_EQ(sym->kind, SYMBOL_LOCAL);
    ASSERT_EQ(sym->slot, 0);
    ASSERT_EQ(sym->depth, 0);
    ASSERT_FALSE(sym->is_captured);
    ASSERT_FALSE(sym->is_initialized);

    scope_free(&scope);
}

TEST(scope_add_symbol_multiple_grows) {
    Scope scope;
    scope_init(&scope, 0, NULL);

    // Add many symbols to trigger growth
    for (int i = 0; i < 20; i++) {
        char name[16];
        snprintf(name, sizeof(name), "var%d", i);
        Symbol* sym = scope_add_symbol(&scope, name, (int)strlen(name), SYMBOL_LOCAL, i);
        ASSERT_NOT_NULL(sym);
    }

    ASSERT_EQ(scope.count, 20);
    ASSERT_GT(scope.capacity, 20);

    scope_free(&scope);
}

TEST(scope_add_symbol_all_kinds) {
    Scope scope;
    scope_init(&scope, 0, NULL);

    Symbol* local = scope_add_symbol(&scope, "local", 5, SYMBOL_LOCAL, 0);
    Symbol* global = scope_add_symbol(&scope, "global", 6, SYMBOL_GLOBAL, 0);
    Symbol* func = scope_add_symbol(&scope, "func", 4, SYMBOL_FUNCTION, 0);
    Symbol* strct = scope_add_symbol(&scope, "MyStruct", 8, SYMBOL_STRUCT, 0);
    Symbol* param = scope_add_symbol(&scope, "param", 5, SYMBOL_PARAMETER, 0);

    ASSERT_EQ(local->kind, SYMBOL_LOCAL);
    ASSERT_EQ(global->kind, SYMBOL_GLOBAL);
    ASSERT_EQ(func->kind, SYMBOL_FUNCTION);
    ASSERT_EQ(strct->kind, SYMBOL_STRUCT);
    ASSERT_EQ(param->kind, SYMBOL_PARAMETER);

    scope_free(&scope);
}

// ============================================================================
// Symbol Lookup Tests
// ============================================================================

TEST(scope_lookup_local_found) {
    Scope scope;
    scope_init(&scope, 0, NULL);

    scope_add_symbol(&scope, "x", 1, SYMBOL_LOCAL, 0);
    scope_add_symbol(&scope, "y", 1, SYMBOL_LOCAL, 1);
    scope_add_symbol(&scope, "z", 1, SYMBOL_LOCAL, 2);

    Symbol* found = scope_lookup_local(&scope, "y", 1);
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(found->slot, 1);

    scope_free(&scope);
}

TEST(scope_lookup_local_not_found) {
    Scope scope;
    scope_init(&scope, 0, NULL);

    scope_add_symbol(&scope, "x", 1, SYMBOL_LOCAL, 0);

    Symbol* found = scope_lookup_local(&scope, "w", 1);
    ASSERT_NULL(found);

    scope_free(&scope);
}

TEST(scope_lookup_local_most_recent) {
    Scope scope;
    scope_init(&scope, 0, NULL);

    // Add same name twice (shadowing)
    scope_add_symbol(&scope, "x", 1, SYMBOL_LOCAL, 0);
    Symbol* second = scope_add_symbol(&scope, "x", 1, SYMBOL_LOCAL, 1);

    Symbol* found = scope_lookup_local(&scope, "x", 1);
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(found, second);  // Should return the most recent one
    ASSERT_EQ(found->slot, 1);

    scope_free(&scope);
}

TEST(scope_lookup_traverses_enclosing) {
    Scope outer;
    scope_init(&outer, 0, NULL);
    scope_add_symbol(&outer, "outer_var", 9, SYMBOL_GLOBAL, 0);

    Scope inner;
    scope_init(&inner, 1, &outer);
    scope_add_symbol(&inner, "inner_var", 9, SYMBOL_LOCAL, 0);

    // Should find outer_var by traversing enclosing
    Symbol* found = scope_lookup(&inner, "outer_var", 9);
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(found->kind, SYMBOL_GLOBAL);

    scope_free(&inner);
    scope_free(&outer);
}

TEST(scope_lookup_deep_nesting) {
    Scope level0;
    scope_init(&level0, 0, NULL);
    scope_add_symbol(&level0, "a", 1, SYMBOL_GLOBAL, 0);

    Scope level1;
    scope_init(&level1, 1, &level0);
    scope_add_symbol(&level1, "b", 1, SYMBOL_LOCAL, 0);

    Scope level2;
    scope_init(&level2, 2, &level1);
    scope_add_symbol(&level2, "c", 1, SYMBOL_LOCAL, 0);

    Scope level3;
    scope_init(&level3, 3, &level2);

    // From deepest level, find global from level0
    Symbol* found = scope_lookup(&level3, "a", 1);
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(found->kind, SYMBOL_GLOBAL);

    scope_free(&level3);
    scope_free(&level2);
    scope_free(&level1);
    scope_free(&level0);
}

TEST(scope_lookup_shadowing) {
    Scope outer;
    scope_init(&outer, 0, NULL);
    scope_add_symbol(&outer, "x", 1, SYMBOL_GLOBAL, 0);

    Scope inner;
    scope_init(&inner, 1, &outer);
    Symbol* inner_x = scope_add_symbol(&inner, "x", 1, SYMBOL_LOCAL, 0);

    // Should find inner x, not outer
    Symbol* found = scope_lookup(&inner, "x", 1);
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(found, inner_x);
    ASSERT_EQ(found->kind, SYMBOL_LOCAL);

    scope_free(&inner);
    scope_free(&outer);
}

TEST(scope_lookup_not_found) {
    Scope outer;
    scope_init(&outer, 0, NULL);

    Scope inner;
    scope_init(&inner, 1, &outer);

    Symbol* found = scope_lookup(&inner, "nonexistent", 11);
    ASSERT_NULL(found);

    scope_free(&inner);
    scope_free(&outer);
}

// ============================================================================
// Name Comparison Tests
// ============================================================================

TEST(names_equal_same) {
    ASSERT(names_equal("hello", 5, "hello", 5));
    ASSERT(names_equal("x", 1, "x", 1));
    ASSERT(names_equal("", 0, "", 0));
}

TEST(names_equal_different_length) {
    ASSERT_FALSE(names_equal("hello", 5, "helloworld", 10));
    ASSERT_FALSE(names_equal("x", 1, "xy", 2));
}

TEST(names_equal_different_content) {
    ASSERT_FALSE(names_equal("hello", 5, "world", 5));
    ASSERT_FALSE(names_equal("abc", 3, "abd", 3));
}

TEST(names_equal_empty) {
    ASSERT(names_equal("", 0, "", 0));
    ASSERT_FALSE(names_equal("", 0, "x", 1));
    ASSERT_FALSE(names_equal("x", 1, "", 0));
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Scope Lifecycle");
    RUN_TEST(scope_init_sets_defaults);
    RUN_TEST(scope_init_with_enclosing);
    RUN_TEST(scope_free_empty);
    RUN_TEST(scope_free_with_symbols);

    TEST_SUITE("Symbol Management");
    RUN_TEST(scope_add_symbol_first);
    RUN_TEST(scope_add_symbol_multiple_grows);
    RUN_TEST(scope_add_symbol_all_kinds);

    TEST_SUITE("Symbol Lookup");
    RUN_TEST(scope_lookup_local_found);
    RUN_TEST(scope_lookup_local_not_found);
    RUN_TEST(scope_lookup_local_most_recent);
    RUN_TEST(scope_lookup_traverses_enclosing);
    RUN_TEST(scope_lookup_deep_nesting);
    RUN_TEST(scope_lookup_shadowing);
    RUN_TEST(scope_lookup_not_found);

    TEST_SUITE("Name Comparison");
    RUN_TEST(names_equal_same);
    RUN_TEST(names_equal_different_length);
    RUN_TEST(names_equal_different_content);
    RUN_TEST(names_equal_empty);

    TEST_SUMMARY();
}
