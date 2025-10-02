#include "../test_framework.h"
#include "core/array.h"

PH_ARRAY_DEFINE(TestIntArray, int);

TEST(array_init_empty) {
    TestIntArray arr;
    ph_array_init(&arr);

    ASSERT_NULL(arr.data);
    ASSERT_EQ(arr.count, 0);
    ASSERT_EQ(arr.capacity, 0);
}

TEST(array_push_single) {
    TestIntArray arr;
    ph_array_init(&arr);

    ph_array_push(&arr, 42);

    ASSERT_NOT_NULL(arr.data);
    ASSERT_EQ(arr.count, 1);
    ASSERT_EQ(arr.data[0], 42);

    ph_array_free(&arr);
}

TEST(array_push_multiple) {
    TestIn