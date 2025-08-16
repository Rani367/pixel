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
        total_size = padding + size;