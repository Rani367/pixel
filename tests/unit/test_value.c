#include "../test_framework.h"
#include "vm/value.h"
#include "vm/object.h"
#include "vm/gc.h"
#include <math.h>

// Setup/teardown for string interning
static void setup(void) {
    gc_init();
    strings_init();
}

static void teardown(void) {
    // Free all objects created during test
    gc_free_all();
    strings_free();
}

// ============================================================================
// Value Tests
// ============================================================================

TEST(value_none) {
   