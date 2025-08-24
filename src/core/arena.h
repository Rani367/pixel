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
