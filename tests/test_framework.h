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

// Floating point comparison with epsilon
#include <math.h>

#define ASSERT_FLOAT_EQ(a, b) do { \
    double _a = (double)(a); \
    double _b = (double)(b); \
    double _epsilon = 1e-9; \
    if (fabs(_a - _b) > _epsilon) { \
        if (!current_test_failed) printf("FAILED\n"); \
        printf("    Assertion failed: %s == %s\n", #a, #b); \
        printf("    Expected: %.15g, Actual: %.15g\n", _b, _a); \
        printf("    at %s:%d\n", __FILE__, __LINE__); \
        current_test_failed = 1; \
        return; \
    } \
} while (0)

#define ASSERT_FLOAT_EQ_EPS(a, b, eps) do { \
    double _a = (double)(a); \
    double _b = (double)(b); \
    if (fabs(_a - _b) > (eps)) { \
        if (!current_test_failed) printf("FAILED\n"); \
        printf("    Assertion failed: %s == %s (eps=%.15g)\n", #a, #b, (double)(eps)); \
        printf("    Expected: %.15g, Actual: %.15g\n", _b, _a); \
        printf("    at %s:%d\n", __FILE__, __LINE__); \
        current_test_failed = 1; \
        return; \
    } \
} while (0)

// Memory comparison
#define ASSERT_MEM_EQ(a, b, size) do { \
    if (memcmp((a), (b), (size)) != 0) { \
        if (!current_test_failed) printf("FAILED\n"); \
        printf("    Assertion failed: memory %s == %s (%zu bytes)\n", #a, #b, (size_t)(size)); \
        printf("    at %s:%d\n", __FILE__, __LINE__); \
        current_test_failed = 1; \
        return; \
    } \
} while (0)

// Enhanced integer assertion with value printing
#define ASSERT_EQ_INT(a, b) do { \
    long long _a = (long long)(a); \
    long long _b = (long long)(b); \
    if (_a != _b) { \
        if (!current_test_failed) printf("FAILED\n"); \
        printf("    Assertion failed: %s == %s\n", #a, #b); \
        printf("    Expected: %lld, Actual: %lld\n", _b, _a); \
        printf("    at %s:%d\n", __FILE__, __LINE__); \
        current_test_failed = 1; \
        return; \
    } \
} while (0)

#define ASSERT_TRUE(a) ASSERT(a)
#define ASSERT_FALSE(a) ASSERT(!(a))

#define ASSERT_STR_CONTAINS(haystack, needle) do { \
    if (strstr((haystack), (needle)) == NULL) { \
        if (!current_test_failed) printf("FAILED\n"); \
        printf("    Assertion failed: \"%s\" contains \"%s\"\n", (haystack), (needle)); \
        printf("    at %s:%d\n", __FILE__, __LINE__); \
        current_test_failed = 1; \
        return; \
    } \
} while (0)

// Greater than assertion
#define ASSERT_GT(a, b) do { \
    if (!((a) > (b))) { \
        if (!current_test_failed) printf("FAILED\n"); \
        printf("    Assertion failed: %s > %s\n", #a, #b); \
        printf("    at %s:%d\n", __FILE__, __LINE__); \
        current_test_failed = 1; \
        return; \
    } \
} while (0)

// Less than assertion
#define ASSERT_LT(a, b) do { \
    if (!((a) < (b))) { \
        if (!current_test_failed) printf("FAILED\n"); \
        printf("    Assertion failed: %s < %s\n", #a, #b); \
        printf("    at %s:%d\n", __FILE__, __LINE__); \
        current_test_failed = 1; \
        return; \
    } \
} while (0)

#endif // PH_TEST_FRAMEWORK_H
