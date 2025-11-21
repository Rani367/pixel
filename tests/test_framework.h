#ifndef PH_TEST_FRAMEWORK_H
#define PH_TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;
static int current_test_failed = 0;

#define TEST(name) static void test_##name(void)

#define RUN_TEST(name) do { \
    tests_run++; \
    current_test_failed = 0; \
    printf("  %s... ", #name); \
    fflush(stdout); \
    test_##name(); \
    if (!current_test_failed) { \
        tests_passed++; \
        printf("OK\n"); \
    } \
} while (0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        if (!current_test_failed) { \
            printf("FAILED\n"); \
        } \
        printf("    Assertion failed: %s\n", #cond); \
        printf("    at %s:%d\n", __FILE__, __LINE__); \
        current_test_failed = 1; \
        return; \
    } \
} while (0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        if (!current_test_failed) { \
            printf("FAILED\n"); \
        } \
        printf("    Assertion failed: %s == %s\n", #a, #b); \
        printf("    at %s:%d\n", __FILE__, __LINE__); \
        current_test_failed = 1; \
        return; \
    } \
} while (0)

#define ASSERT_NE(a, b) do { \
    if ((a) == (b)) { \
        if (!current_test_failed) { \
            printf("FAILED\n"); \
        } \
        printf("    Assertion failed: %s != %s\n", #a, #b); \
        printf("    at %s:%d\n", __FILE__, __LINE__); \
        current_test_failed = 1; \
        return; \
    } \
} while (0)

#define ASSERT_STR_EQ(a, b) do { \
    if (strcmp((a), (b)) != 0) { \
        if (!current_test_failed) { \
            printf("FAILED\n"); \
        } \
        printf("    Assertion failed: \"%s\" == \"%s\"\n", (a), (b)); \
        printf("    at %s:%d\n", __FILE__, __LINE__); \
        current_test_failed = 1; \
        return; \
    } \
} while (0)

#define ASSERT_NULL(a) ASSERT((a) == NULL)
#define ASSERT_NOT_NULL(a) ASSERT((a) != NULL)

#define TEST_SUITE(name) \
    printf("\n%s:\n", name)

#define TEST_SUMMARY() do { \
    printf("\n%d/%d tests passed\n", tests_passed, tests_run); \
    return tests_passed == tests_run ? 0 : 1; \
} while (0)

#endif // PH_TEST_FRAMEWORK_H
