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
