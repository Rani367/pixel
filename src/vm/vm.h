==================

// Initialize the VM
void vm_init(VM* vm);

// Free VM resources
void vm_free(VM* vm);

// ============================================================================
// Execution
// ============================================================================

// Interpret a compiled function (main entry point)
InterpretResult vm_interpret(VM* vm, ObjFunction* function);

// Call a closure with arguments (for engine callbacks)
// Returns true on success, false on runtime error
bool vm_call_closure(VM* vm, ObjClosure* closure, int argc, Value* argv);

// ============================================================================
// Stack Operations (for native functions)
// ============================================================================

// Push a value onto the stack
void vm_push(VM* vm, Value value);

// Pop a value from the stack
Value vm_pop(VM* vm);

// Peek at a value on the stack (0 = top)
Value vm_peek(VM* vm, int distance);

// ============================================================================
// Global Variables
// ============================================================================

// Define a global variable
void vm_define_global(VM* vm, ObjString* name, Value value);

// ============================================================================
// Error Reporting
// ============================================================================

// Report a runtime error with printf-style formatting
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
void vm_runtime_error(VM* vm, const char* format, ...);

#endif // PH_VM_H
#ifndef PH_VM_H
#define PH_VM_H

#include "core/common.h"
#include "core/table.h"
#include "vm/value.h"
#include "vm/chunk.h"
#include "vm/object.h"

// Stack and frame limits
#define STACK_MAX 256
#define FRAMES_MAX 64

// Interpretation result
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

// Call frame for function execution
typedef struct {
    ObjClosure* closure;
    uint8_t* ip;            // Instruction pointer within the function's chunk
    Value* slots;           // First slot in the stack for this call frame
} CallFrame;

// Virtual machine state
typedef struct VM {
    // Call stack
    CallFrame frames[FRAMES_MAX];
    int frame_count;

    // Value stack
    Value stack[STACK_MAX];
    Value* stack_top;

    // Global variables
    Table globals;

    // String interning (shared with object.c)
    // Note: strings_init/strings_free manage this

    // Open upvalues (linked list, sorted by stack slot)
    ObjUpvalue* open_upvalues;

    // Object tracking for GC
    Object* objects;

    // GC state
    size_t bytes_allocated;
    size_t next_gc;

    // Gray stack for tri-color marking
    Object** gray_stack;
    int gray_count;
    int gray_capacity;
} VM;

// ============================================================================
// VM Lifecycle
// ==========================================================