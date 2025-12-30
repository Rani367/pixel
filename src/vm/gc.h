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
// #define DEBUG_STRESS_GC

// ============================================================================
// GC State Management
// ============================================================================

// Set the current VM for object allocation
// Call this before any object allocation that should be tracked by the VM
void gc_set_vm(struct VM* vm);

// Get the current VM (may be NULL during compilation)
struct VM* gc_get_vm(void);

// ============================================================================
// Object Allocation
// ============================================================================

// Allocate an object and track it for GC
// If a VM is set, adds to the VM's object list and may trigger GC
// Otherwise, adds to a global list (for compilation)
struct Object* gc_allocate_object(size_t size, int type);

// Track bytes allocated (internal use)
void gc_track_allocation(size_t size);

// ============================================================================
// Garbage Collection
// ============================================================================

// Run a full garbage collection cycle
void gc_collect(struct VM* vm);

// ============================================================================
// Marking API (for external roots)
// ============================================================================

// Mark a single value as reachable
void gc_mark_value(struct VM* vm, Value value);

// Mark an object as reachable
void gc_mark_object(struct VM* vm, struct Object* object);

// ============================================================================
// Lifecycle
// ============================================================================

// Initialize the global GC state (call once at startup)
void gc_init(void);

// Transfer compiled objects to the VM
// Call after compilation and before interpretation
void gc_transfer_objects(struct VM* vm);

// Free all remaining global objects (call at shutdown if no VM ran)
void gc_free_all(void);

#endif // PH_GC_H
