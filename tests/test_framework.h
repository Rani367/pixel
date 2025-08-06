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
        if (!current_test