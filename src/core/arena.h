ize_t align);

// Reset the arena (reuse memory without freeing)
void arena_reset(Arena* arena);

// Free the arena and all its memory
void arena_free(Arena* arena);

// Get total bytes allocated across all blocks
size_t arena_total_allocated(Arena* arena);

// Get total bytes used across all blocks
size_t arena_total_used(Arena* arena);

#endif // PH_ARENA_H
na allocator
typedef struct {
    ArenaBlock* first;
    ArenaBlock* current;
} Arena;

// Create a new arena with the given initial capacity
Arena* arena_new(size_t initial_capacity);

// Allocate memory from the arena (unaligned)
void* arena_alloc(Arena* arena, size_t size);

// Allocate memory from the arena with alignment
void* arena_alloc_aligned(Arena* arena, size_t size, s