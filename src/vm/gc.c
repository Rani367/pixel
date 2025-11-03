#include "vm/gc.h"
#include "vm/vm.h"
#include "vm/object.h"
#include "vm/chunk.h"
#include "core/table.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

// ============================================================================
// Global State
// ============================================================================

// Current VM for object allocation (NULL during compilation)
static VM* gc_vm = NULL;

// Global object list for objects created during compilation
static Object* global_objects = NULL;

// ============================================================================
// GC State Management
// ============================================================================

void gc_set_vm(VM* vm) {
    gc_vm = vm;
}

VM* gc_get_vm(void) {
    return gc_vm;
}

void gc_init(void) {
    gc_vm = NULL;
    global_objects = NULL;
}

void gc_transfer_objects(VM* vm) {
    // Transfer objects from global list to VM
    if (global_objects == NULL) return;

    // Find the end of the global list
    Object* last = global_objects;
#ifdef DEBUG_LOG_GC
    size_t count = 1;
#endif
    while (last->next != NULL) {
        last = last->next;
#ifdef DEBUG_LOG_GC
        count++;
#endif
    }

    // Link the global list to the front of the VM's list
    last->next = vm->objects;
    vm->objects = global_objects;
    global_objects = NULL;

#ifdef DEBUG_LOG_GC
    printf("[gc] Transferred %zu objects to VM\n", count);
#endif
}

void gc_free_all(void) {
    Object* object = global_objects;
    while (object != NULL) {
        Object* next = object->next;
        object_free(object);
        object = next;
    }
    global_objects = NULL;
}

// ============================================================================
// Object Allocation
// ============================================================================

void gc_track_allocation(size_t size) {
    if (gc_vm != NULL) {
        gc_vm->bytes_allocated += size;

#ifdef DEBUG_STRESS_GC
        gc_collect(gc_vm);
#endif

        if (gc_vm->bytes_allocated > gc_vm->next_gc) {
            gc_collect(gc_vm);
        }
    }
}

Object* gc_allocate_object(size_t size, int type) {
    gc_track_allocation(size);

    Object* object = (Object*)PH_ALLOC(size);
    if (object == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    object->type = (ObjectType)type;
    object->marked = false;

    // Add to appropriate list
    if (gc_vm != NULL) {
        object->next = gc_vm->objects;
        gc_vm->objects = object;
    } else {
        object->next = global_objects;
        global_objects = object;
    }

#ifdef DEBUG_LOG_GC
    printf("[gc] %p allocate %zu bytes for type %d\n", (void*)object, size, type);
#endif

    return object;
}

// ============================================================================
// Mark Phase
// ============================================================================

static void add_to_gray_stack(VM* vm, Object* object) {
    // Grow gray stack if needed
    if (vm->gray_count >= vm->gray_capacity) {
        vm->gray_capacity = PH_GROW_CAPACITY(vm->gray_capacity);
        vm->gray_stack = realloc(vm->gray_stack,
                                  sizeof(Object*) * vm->gray_capacity);
        if (vm->gray_stack == NULL) {
            fprintf(stderr, "Out of memory for gray stack\n");
            exit(1);
        }
    }

    vm->gray_stack[vm->gray_count++] = object;
}

void gc_mark_object(VM* vm, Object* object) {
    if (object == NULL) return;
    if (object->marked) return;

#ifdef DEBUG_LOG_GC
    printf("[gc] %p mark ", (void*)object);
    value_print(OBJECT_VAL(object));
    printf("\n");
#endif

    object->marked = true;
    add_to_gray_stack(vm, object);
}

void gc_mark_value(VM* vm, Value value) {
    if (IS_OBJECT(value)) {
        gc_mark_object(vm, AS_OBJECT(value));
    }
}

// Tombstone marker (must match table.c)
#define TOMBSTONE_KEY ((const char*)1)

static void mark_table(VM* vm, Table* table) {
    for (int i = 0; i < table->capacity; i++) {
        TableEntry* entry = &table->entries[i];
        if (entry->key != NULL && entry->key != TOMBSTONE_KEY && entry->value != NULL) {
            // The key is the chars field of an ObjString - mark that string too
            // This keeps the key string alive so the table entry remains valid
            ObjString* key_string = (ObjString*)(entry->key - offsetof(ObjString, chars));
            gc_mark_object(vm, (Object*)key_string);

            // Mark the value if it's a Value*
            gc_mark_value(vm, *(Value*)entry->value);
        }
    }
}

static void mark_roots(VM* vm) {
#ifdef DEBUG_LOG_GC
    printf("[gc] -- mark roots --\n");
#endif

    // Mark the stack
    for (Value* slot = vm->stack; slot < vm->stack_top; slot++) {
        gc_mark_value(vm, *slot);
    }

    // Mark the call frames (closures)
    for (int i = 0; i < vm->frame_count; i++) {
        gc_mark_object(vm, (Object*)vm->frames[i].closure);
    }

    // Mark open upvalues
    for (ObjUpvalue* upvalue = vm->open_upvalues;
         upvalue != NULL;
         upvalue = upvalue->next) {
        gc_mark_object(vm, (Object*)upvalue);
    }

    // Mark globals (the values stored in the table)
    mark_table(vm, &vm->globals);
}

// ============================================================================
// Trace Phase
// ============================================================================

static void blacken_object(VM* vm, Object* object) {
#ifdef DEBUG_LOG_GC
    printf("[gc] %p blacken ", (void*)object);
    value_print(OBJECT_VAL(object));
    printf("\n");
#endif

    switch (object->type) {
        case OBJ_STRING:
            // Strings have no references
            break;

        case OBJ_UPVALUE:
            // Mark the closed-over value
            gc_mark_value(vm, ((ObjUpvalue*)object)->closed);
            break;

        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            gc_mark_object(vm, (Object*)function->name);
            // Mark constants in the function's chunk
            if (function->chunk != NULL) {
                for (int i = 0; i < function->chunk->constants.count; i++) {
                    gc_mark_value(vm, function->chunk->constants.values[i]);
                }
            }
            break;
        }

        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)object;
            gc_mark_object(vm, (Object*)closure->function);
            for (int i = 0; i < closure->upvalue_count; i++) {
                gc_mark_object(vm, (Object*)closure->upvalues[i]);
            }
            break;
        }

        case OBJ_STRUCT_DEF: {
            ObjStructDef* def = (ObjStructDef*)object;
            gc_mark_object(vm, (Object*)def->name);
            for (int i = 0; i < def->field_count; i++) {
                gc_mark_object(vm, (Object*)def->fields[i]);
            }
            // Mark method closures in the methods table
            for (int i = 0; i < def->methods.capacity; i++) {
                TableEntry* entry = &def->methods.entries[i];
                // Skip empty slots and tombstones (tombstone key == 1)
                if (entry->key != NULL && entry->key != (const char*)1) {
                    gc_mark_object(vm, (Object*)entry->value);
                }
            }
            break;
        }

        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*)object;
            gc_mark_object(vm, (Object*)instance->struct_def);
            for (int i = 0; i < instance->struct_def->field_count; i++) {
                gc_mark_value(vm, instance->fields[i]);
            }
            break;
        }

        case OBJ_LIST: {
            ObjList* list = (ObjList*)object;
            for (int i = 0; i < list->count; i++) {
                gc_mark_value(vm, list->items[i]);
            }
            break;
        }

        case OBJ_NATIVE: {
            ObjNative* native = (ObjNative*)object;
            gc_mark_object(vm, (Object*)native->name);
            break;
        }

        case OBJ_VEC2:
            // Vec2 has no references
            break;

        case OBJ_IMAGE: {
            ObjImage* image = (ObjImage*)object;
            gc_mark_object(vm, (Object*)image->path);
            break;
        }

        case OBJ_SPRITE: {
            ObjSprite* sprite = (ObjSprite*)object;
            gc_mark_object(vm, (Object*)sprite->image);
            gc_mark_object(vm, (Object*)sprite->animation);
            break;
        }

        case OBJ_FONT:
            // Font has no object references
            break;

        case OBJ_SOUND: {
            ObjSound* sound = (ObjSound*)object;
            gc_mark_object(vm, (Object*)sound->path);
            break;
        }

        case OBJ_MUSIC: {
            ObjMusic* music = (ObjMusic*)object;
            gc_mark_object(vm, (Object*)music->path);
            break;
        }

        case OBJ_CAMERA: {
            ObjCamera* camera = (ObjCamera*)object;
            gc_mark_object(vm, (Object*)camera->target);
            break;
        }

        case OBJ_ANIMATION: {
            ObjAnimation* anim = (ObjAnimation*)object;
            gc_mark_object(vm, (Object*)anim->image);
            gc_mark_object(vm, (Object*)anim->on_complete);
            break;
        }

        case OBJ_PARTICLE_EMITTER:
            // Particle emitter has no object references
            break;
    }
}

static void trace_references(VM* vm) {
#ifdef DEBUG_LOG_GC
    printf("[gc] -- trace r   Object* object = vm->objects;

    while (object != NULL) {
        if (object->marked) {
            // Object is reachable, unmark it for next cycle
            object->marked = false;
            previous = object;
            object = object->next;
        } else {
            // Object is unreachable, free it
            Object* unreached = object;
            object = object->next;

            if (previous != NULL) {
                previous->next = object;
            } else {
                vm->objects = object;
            }

#ifdef DEBUG_LOG_GC
            printf("[gc] %p free type %d\n", (void*)unreached, unreached->type);
#endif

            // Track the freed memory (approximate - we don't track exact size per object)
            // This is a simplification; production GC would track exact sizes
            vm->bytes_allocated -= sizeof(Object);  // Minimum

            object_free(unreached);
        }
    }
}

// ============================================================================
// String Table Weak References
// ============================================================================

// Remove unmarked strings from the intern table
// This is called before sweep so we can identify strings that will be freed
extern void strings_remove_white(void);

// ============================================================================
// Main GC Entry Point
// ============================================================================

void gc_collect(VM* vm) {
#ifdef DEBUG_LOG_GC
    printf("[gc] == gc begin ==\n");
    size_t before = vm->bytes_allocated;
#endif

    // Mark phase
    mark_roots(vm);

    // Trace phase
    trace_references(vm);

    // eferences --\n");
#endif

    while (vm->gray_count > 0) {
        Object* object = vm->gray_stack[--vm->gray_count];
        blacken_object(vm, object);
    }
}

// ============================================================================
// Sweep Phase
// ============================================================================

static void sweep(VM* vm) {
#ifdef DEBUG_LOG_GC
    printf("[gc] -- sweep --\n");
#endif

    Object* previous = NULL;
 Remove weak references to unmarked strings
    strings_remove_white();

    // Sweep phase
    sweep(vm);

    // Adjust next GC threshold
    vm->next_gc = vm->bytes_allocated * GC_HEAP_GROW_FACTOR;
    if (vm->next_gc < GC_INITIAL_THRESHOLD) {
        vm->next_gc = GC_INITIAL_THRESHOLD;
    }

#ifdef DEBUG_LOG_GC
    printf("[gc] == gc end ==\n");
    printf("[gc] collected %zu bytes (from %zu to %zu) next at %zu\n",
           before - vm->bytes_allocated, before, vm->bytes_allocated,
           vm->next_gc);
#endif
}
