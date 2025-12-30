#ifndef PH_STDLIB_H
#define PH_STDLIB_H

#include "vm/vm.h"
#include "vm/object.h"

// Initialize the standard library by registering all native functions
void stdlib_init(VM* vm);

// Helper to define a native function in the VM's globals
void define_native(VM* vm, const char* name, NativeFn function, int arity);

#endif // PH_STDLIB_H
