#include "../test_framework.h"
#include "core/arena.h"

TEST(arena_new_returns_valid_arena) {
    Arena* arena = arena_new(1024);
    ASSERT_NOT_NULL(arena);
    ASSERT_EQ(arena_total_allocated(arena), 1024);
    ASSERT_EQ(arena_total_used(arena), 0);
    arena_free(arena);
}

TEST(arena_new_default_capacity) {
    Arena* arena = arena_new(0);
    ASSERT_NOT_NULL(arena);
    ASSERT_EQ(arena_total_allocated(arena), PH_ARENA_DEFAULT_CAPACITY);
    arena_free(arena);
}

TEST(arena_alloc_returns_valid_pointer) {
    Arena* arena = arena_new(1024);
    void* ptr = arena_alloc(arena, 100);
    ASSERT_NOT_NULL(ptr);
    ASSERT(arena_total_used(arena) >= 100);
    arena_free(arena);
}

TEST(arena_alloc_multiple) {
    Arena* arena = arena_new(1024);
    void* ptr1 = arena_alloc(arena, 100);
    void* ptr2 = arena_alloc(arena, 200);
    void* ptr3 = arena_alloc(arena, 50);

    ASSERT_NOT_NULL(ptr1);
    ASSERT_NOT_NULL(ptr2);
    ASSERT_NOT_NULL(ptr3);

    // Pointers should be different
    ASSERT(ptr1 != ptr2);
    ASSERT(ptr2 != ptr3);
    ASSERT(ptr1 != ptr3);

    arena_free(arena);
}

TEST(arena_alloc_aligned) {
    Arena* arena = arena_new(1024);

    void* ptr1 = arena_alloc_aligned(arena, 1, 16);
    ASSERT_NOT_NULL(ptr1);
    ASSERT_EQ((uintptr_t)ptr1 % 16, 0);

    void* ptr2 = arena_alloc_aligned(arena, 1, 32);
    ASSERT_NOT_NULL(ptr2);
    ASSERT_EQ((uintptr_t)ptr2 % 32, 0);

    arena_free(arena);
}

TEST(arena_grows_when_needed) {
    Arena* arena = arena_new(64);
    size_t initial = arena_total_allocated(arena);

    // Allocate more than initial capacity
    void* ptr = arena_alloc(arena, 100);
    ASSERT_NOT_NULL(ptr);
    ASSERT(arena_total_allocated(arena) > initial);

    arena_free(arena);
}

TEST(arena_reset_reuses_memory) {
    Arena* arena = arena_new(1024);

    arena_alloc(arena, 100);
    arena_alloc(arena, 200);
    size_t used_before_reset = arena_total_used(arena);
    ASSERT(used_before_reset > 0);

    arena_reset(arena);
    ASSERT_EQ(arena_total_used(arena), 0);
    ASSERT_EQ(arena_total_allocated(arena), 1024);

    arena_free(arena);
}

TEST(arena_zero_initializes) {
    Arena* arena = arena_new(1024);
    uint8_t* ptr = arena_alloc(arena, 100);

    // Check that memory is zeroed
    for (int i = 0; i < 100; i++) {
        ASSERT_EQ(ptr[i], 0);
    }

    arena_free(arena);
}

int main(void) {
    TEST_SUITE("Arena");

    RUN_TEST(arena_new_returns_valid_arena);
    RUN_TEST(arena_new_default_capacity);
    RUN_TEST(arena_alloc_returns_valid_pointer);
    RUN_TEST(arena_alloc_multiple);
    RUN_TEST(arena_alloc_aligned);
    RUN_TEST(arena_grows_when_needed);
    RUN_TEST(arena_reset_reuses_memory);
    RUN_TEST(arena_zero_initializes);

    TEST_SUMMARY();
}
