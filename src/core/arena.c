intptr_t current = (uintptr_t)(block->memory + block->used);
    uintptr_t aligned = (current + align - 1) & ~(align - 1);
    size_t padding = aligned - current;
    size_t total_size = padding + size;

    // Check if we need a new block
    if (block->used + total_size > block->capacity) {
        // Allocate a new block (at least as big as requested)
        size_t new_capacity = block->capacity * 2;
        if (new_capacity < size + align) {
            new_capacity = size + align;
        }

        ArenaBlock* new_block = arena_block_new(new_capacity);
        if (new_block == NULL) {
            return NULL;
        }

        block->next = new_block;
        arena->current = new_block;
        block = new_block;

        // Recalculate alignment for new block
        current = (uintptr_t)block->memory;
        aligned = (current + align - 1) & ~(align - 1);
        padding = aligned - current;
        total_size = padding + size;tal_used(Arena* arena) {
    PH_ASSERT(arena != NULL);

    size_t total = 0;
    ArenaBlock* block = arena->first;
    while (block != NULL) {
        total += block->used;
        block = block->next;
    }
    return total;
}
#include "arena.h"
#include <string.h>

static ArenaBlock* arena_block_new(size_t capacity) {
    ArenaBlock* block = PH_ALLOC(sizeof(ArenaBlock));
    if (block == NULL) {
        return NULL;
    }

    block->memory = PH_ALLOC(capacity);
    if (block->memory == NULL) {
        PH_FREE(block);
        return NULL;
    }

    block->capacity = capacity;
    block->used = 0;
    block->next = NULL;
    return block;
}

static void arena_block_free(ArenaBlock* block) {
    if (block == NULL) {
        return;
    }
    PH_FREE(block->memory);
    PH_FREE(block);
}

Arena* arena_new(size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = PH_ARENA_DEFAULT_CAPACITY;
    }

    Arena* arena = PH_ALLOC(sizeof(Arena));
    if (
    }

    void* ptr = block->memory + block->used + padding;
    block->used += total_size;

    // Zero the memory
    memset(ptr, 0, size);

    return ptr;
}

void arena_reset(Arena* arena) {
    PH_ASSERT(arena != NULL);

    // Reset all blocks to unused
    ArenaBlock* block = arena->first;
    while (block != NULL) {
        block->used = 0;
        block = block->next;
    }

    arena->current = arena->first;
}

void arena_free(Arena* arena) {
    if (arena == NULL) {
        return;
    }

    // Free all blocks
    ArenaBlock* block = arena->first;
    while (block != NULL) {
        ArenaBlock* next = block->next;
        arena_block_free(block);
        block = next;
    }

    PH_FREE(arena);
}

size_t arena_total_allocated(Arena* arena) {
    PH_ASSERT(arena != NULL);

    size_t total = 0;
    ArenaBlock* block = arena->first;
    while (block != NULL) {
        total += block->capacity;
        block = block->next;
    }
    return total;
}

size_t arena_to