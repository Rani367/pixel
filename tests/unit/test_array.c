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
    TestIntArray arr;
    ph_array_init(&arr);

    for (int i = 0; i < 100; i++) {
        ph_array_push(&arr, i * 2);
    }

    ASSERT_EQ(arr.count, 100);
    for (int i = 0; i < 100; i++) {
        ASSERT_EQ(arr.data[i], i * 2);
    }

    ph_array_free(&arr);
}

TEST(array_pop) {
    TestIntArray arr;
    ph_array_init(&arr);

    ph_array_push(&arr, 10);
    ph_array_push(&arr, 20);
    ph_array_push(&arr, 30);

    ASSERT_EQ(ph_array_pop(&arr), 30);
    ASSERT_EQ(arr.count, 2);

    ASSERT_EQ(ph_array_pop(&arr), 20);
    ASSERT_EQ(arr.count, 1);

    ASSERT_EQ(ph_array_pop(&arr), 10);
    ASSERT_EQ(arr.count, 0);

    ph_array_free(&arr);
}

TEST(array_last) {
    TestIntArray arr;
    ph_array_init(&arr);

    ph_array_push(&arr, 1);
    ASSERT_EQ(ph_array_last(&arr), 1);

    ph_array_push(&arr, 2);
    ASSERT_EQ(ph_array_last(&arr), 2);

    ph_array_push(&arr, 3);
    ASSERT_EQ(ph_array_last(&arr), 3);

    ph_array_free(&arr);
}

TEST(array_is_empty) {
    TestIntArray arr;
    ph_array_init(&arr);

    ASSERT(ph_array_is_empty(&arr));

    ph_array_push(&arr, 1);
    ASSERT(!ph_array_is_empty(&arr));

    (void)ph_array_pop(&arr);
    ASSERT(ph_array_is_empty(&arr));

    ph_array_free(&arr);
}

TEST(array_clear) {
    TestIntArray arr;
    ph_array_init(&arr);

    ph_array_push(&arr, 1);
    ph_array_push(&arr, 2);
    ph_array_push(&arr, 3);

    int old_capacity = arr.capacity;
    ph_array_clear(&arr);

    ASSERT_EQ(arr.count, 0);
    ASSERT_EQ(arr.capacity, old_capacity);  // Capacity preserved

    ph_array_free(&arr);
}

TEST(array_reserve) {
    TestIntArray arr;
    ph_array_init(&arr);

    ph_array_reserve(&arr, 100);
    ASSERT(arr.capacity >= 100);
    ASSERT_EQ(arr.count, 0);

    ph_array_free(&arr);
}

TEST(array_free_resets) {
    TestIntArray arr;
    ph_array_init(&arr);

    ph_array_push(&arr, 1);
    ph_array_push(&arr, 2);

    ph_array_free(&arr);

    ASSERT_NULL(arr.data);
    ASSERT_EQ(arr.count, 0);
    ASSERT_EQ(arr.capacity, 0);
}

int main(void) {
    TEST_SUITE("Array");

    RUN_TEST(array_init_empty);
    RUN_TEST(array_push_single);
    RUN_TEST(array_push_multiple);
    RUN_TEST(array_pop);
    RUN_TEST(array_last);
    RUN_TEST(array_is_empty);
    RUN_TEST(array_clear);
    RUN_TEST(array_reserve);
    RUN_TEST(array_free_resets);

    TEST_SUMMARY();
}
