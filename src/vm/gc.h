call at shutdown if no VM ran)
void gc_free_all(void);

#endif // PH_GC_H
#ifndef PH_GC_H
#define PH_GC_H

#include "core/common.h"
#include "vm/value.h"

// Forward declarations (use struct to avoid typedef conflicts)
struct VM;
struct Object;

// GC configuration
#define GC_HEAP_GROW_FACTOR 2
#define GC_INITIAL_THRESHOLD (1024 * 1024)  // 1MB

// Gray stack initial capacity
#define GC_GRAY_STACK_INITIAL 64

// Debug flags (uncomment to enable)
// #define DEBUG_LOG_GC
// #define DE