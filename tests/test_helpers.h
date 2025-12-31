#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// Seed random for determinism in tests
static inline void test_seed_random(uint32_t seed) {
    srand(seed);
}

// Fixed time for deterministic log tests
static time_t test_fixed_time = 0;

static inline time_t test_time_func(time_t* t) {
    if (t) *t = test_fixed_time;
    return test_fixed_time;
}

// Helper to create temporary test files
static inline const char* test_create_temp_file(const char* content) {
    static char path[256];
    snprintf(path, sizeof(path), "/tmp/pixel_test_%d.pixel", (int)getpid());
    FILE* f = fopen(path, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
    return path;
}

static inline void test_remove_temp_file(const char* path) {
    remove(path);
}

#endif // TEST_HELPERS_H
