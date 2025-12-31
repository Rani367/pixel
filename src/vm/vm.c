#include "vm/vm.h"
#include "vm/gc.h"
#include "vm/debug.h"
#include "core/strings.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

// ============================================================================
// Debug Tracing
// ============================================================================

// #define DEBUG_TRACE_EXECUTION

// ============================================================================
// Stack Operations
// ============================================================================

static void reset_stack(VM* vm) {
    vm->stack_top = vm->stack;
    vm->frame_count = 0;
    vm->open_upvalues = NULL;
}

void vm_push(VM* vm, Value value) {
    if (vm->stack_top >= vm->stack + STACK_MAX) {  // LCOV_EXCL_LINE
        vm_runtime_error(vm, "Value stack overflow");  // LCOV_EXCL_LINE
        return;  // LCOV_EXCL_LINE
    }
    *vm->stack_top = value;
    vm->stack_top++;
}

Value vm_pop(VM* vm) {
    vm->stack_top--;
    return *vm->stack_top;
}

Value vm_peek(VM* vm, int distance) {
    return vm->stack_top[-1 - distance];
}

// ============================================================================
// VM Lifecycle
// ============================================================================

void vm_init(VM* vm) {
    reset_stack(vm);
    table_init(&vm->globals);
    vm->objects = NULL;
    vm->bytes_allocated = 0;
    vm->next_gc = GC_INITIAL_THRESHOLD;

    // Initialize gray stack for GC
    vm->gray_stack = NULL;
    vm->gray_count = 0;
    vm->gray_capacity = 0;

    // Initialize string interning
    strings_init();
}

// Tombstone marker (must match table.c)
#define TOMBSTONE_KEY ((const char*)1)

void vm_free(VM* vm) {
    // Free global variable values (the Value* stored in the table)
    for (int i = 0; i < vm->globals.capacity; i++) {
        TableEntry* entry = &vm->globals.entries[i];
        if (entry->key != NULL && entry->key != TOMBSTONE_KEY && entry->value != NULL) {
            PH_FREE(entry->value);
        }
    }
    table_free(&vm->globals);

    // Free all objects
    Object* object = vm->objects;
    while (object != NULL) {
        Object* next = object->next;
        object_free(object);
        object = next;
    }
    vm->objects = NULL;

    // Free gray stack
    free(vm->gray_stack);
    vm->gray_stack = NULL;
    vm->gray_count = 0;
    vm->gray_capacity = 0;

    // Free string intern table
    strings_free();

    // Clear GC state
    gc_set_vm(NULL);
}

// ============================================================================
// Global Variables
// ============================================================================

void vm_define_global(VM* vm, ObjString* name, Value value) {
    // Store the value as a heap-allocated Value*
    Value* stored = PH_ALLOC(sizeof(Value));
    *stored = value;
    table_set(&vm->globals, name->chars, name->length, stored);
}

// ============================================================================
// Runtime Errors
// ============================================================================

void vm_runtime_error(VM* vm, const char* format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Runtime error: ");
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    vfprintf(stderr, format, args);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
    fprintf(stderr, "\n");
    va_end(args);

    // Print stack trace
    for (int i = vm->frame_count - 1; i >= 0; i--) {
        CallFrame* frame = &vm->frames[i];
        ObjFunction* function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk->code - 1;
        int line = chunk_get_line(function->chunk, (int)instruction);
        fprintf(stderr, "  [line %d] in ", line);
        if (function->name == NULL) {
            fprintf(stderr, "<script>\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    reset_stack(vm);
}

// ============================================================================
// Upvalue Management
// ============================================================================

// Upvalue capture - rarely exercised in unit tests
static ObjUpvalue* capture_upvalue(VM* vm, Value* local) {  // LCOV_EXCL_LINE
    ObjUpvalue* prev_upvalue = NULL;  // LCOV_EXCL_LINE
    ObjUpvalue* upvalue = vm->open_upvalues;  // LCOV_EXCL_LINE

    // Walk the list to find an existing upvalue or insertion point
    while (upvalue != NULL && upvalue->location > local) {  // LCOV_EXCL_LINE
        prev_upvalue = upvalue;  // LCOV_EXCL_LINE
        upvalue = upvalue->next;  // LCOV_EXCL_LINE
    }  // LCOV_EXCL_LINE

    // Found existing upvalue for this slot
    if (upvalue != NULL && upvalue->location == local) {  // LCOV_EXCL_LINE
        return upvalue;  // LCOV_EXCL_LINE
    }  // LCOV_EXCL_LINE

    // Create new upvalue
    ObjUpvalue* created = upvalue_new(local);  // LCOV_EXCL_LINE
    created->next = upvalue;  // LCOV_EXCL_LINE

    // Insert into the linked list
    if (prev_upvalue == NULL) {  // LCOV_EXCL_LINE
        vm->open_upvalues = created;  // LCOV_EXCL_LINE
    } else {  // LCOV_EXCL_LINE
        prev_upvalue->next = created;  // LCOV_EXCL_LINE
    }  // LCOV_EXCL_LINE

    return created;  // LCOV_EXCL_LINE
}  // LCOV_EXCL_LINE

static void close_upvalues(VM* vm, Value* last) {
    while (vm->open_upvalues != NULL &&
           vm->open_upvalues->location >= last) {
        ObjUpvalue* upvalue = vm->open_upvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm->open_upvalues = upvalue->next;
    }
}

// ============================================================================
// Function Calls
// ============================================================================

static bool call(VM* vm, ObjClosure* closure, int arg_count) {
    if (arg_count != closure->function->arity) {
        vm_runtime_error(vm, "Expected %d arguments but got %d",
                         closure->function->arity, arg_count);
        return false;
    }

    if (vm->frame_count == FRAMES_MAX) {
        vm_runtime_error(vm, "Call stack overflow");
        return false;
    }

    CallFrame* frame = &vm->frames[vm->frame_count++];
    frame->closure = closure;
    frame->ip = closure->function->chunk->code;
    frame->slots = vm->stack_top - arg_count - 1;

    return true;
}

static bool call_value(VM* vm, Value callee, int arg_count) {
    if (IS_OBJECT(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_CLOSURE:
                return call(vm, AS_CLOSURE(callee), arg_count);

            case OBJ_NATIVE: {
                ObjNative* native = AS_NATIVE(callee);
                if (native->arity != -1 && arg_count != native->arity) {
                    vm_runtime_error(vm, "Expected %d arguments but got %d",
                                     native->arity, arg_count);
                    return false;
                }
                Value result = native->function(arg_count,
                                                vm->stack_top - arg_count);
                vm->stack_top -= arg_count + 1;
                vm_push(vm, result);
                return true;
            }

            case OBJ_STRUCT_DEF: {
                ObjStructDef* def = AS_STRUCT_DEF(callee);
                // Allow 0 args (all fields null) or all args (positional)
                if (arg_count != 0 && arg_count != def->field_count) {
                    vm_runtime_error(vm, "Expected 0 or %d arguments but got %d",
                                     def->field_count, arg_count);
                    return false;
                }
                ObjInstance* instance = instance_new(def);
                // Set fields from arguments in declaration order
                if (arg_count > 0) {
                    Value* args = vm->stack_top - arg_count;
                    for (int i = 0; i < arg_count; i++) {
                        instance->fields[i] = args[i];
                    }
                }
                vm->stack_top -= arg_count;
                vm->stack_top[-1] = OBJECT_VAL(instance);
                return true;
            }

            default:
                break;  // LCOV_EXCL_LINE
        }
    }  // LCOV_EXCL_LINE

    vm_runtime_error(vm, "Can only call functions and constructors");
    return false;
}

// ============================================================================
// Main Execution Loop
// ============================================================================

// Read the next byte from the instruction stream
#define READ_BYTE() (*frame->ip++)

// Read a 2-byte (16-bit) operand
#define READ_SHORT() \
    (frame->ip += 2, \
     (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

// Read a constant from the constant pool
#define READ_CONSTANT() \
    (frame->closure->function->chunk->constants.values[READ_BYTE()])

// Read a string constant
#define READ_STRING() AS_STRING(READ_CONSTANT())

// Binary operation macro
#define BINARY_OP(value_type, op) \
    do { \
        if (!IS_NUMBER(vm_peek(vm, 0)) || !IS_NUMBER(vm_peek(vm, 1))) { \
            vm_runtime_error(vm, "Operands must be numbers"); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(vm_pop(vm)); \
        double a = AS_NUMBER(vm_pop(vm)); \
        vm_push(vm, value_type(a op b)); \
    } while (false)

// Normalize negative indices (Python-style: -1 = last element)
static int normalize_index(int index, int length) {
    if (index < 0) {
        index = length + index;
    }
    return index;
}

static InterpretResult run(VM* vm) {
    CallFrame* frame = &vm->frames[vm->frame_count - 1];

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        // Print stack
        printf("          ");
        for (Value* slot = vm->stack; slot < vm->stack_top; slot++) {
            printf("[ ");
            value_print(*slot);
            printf(" ]");
        }
        printf("\n");

        // Disassemble current instruction
        disassemble_instruction(
            frame->closure->function->chunk,
            (int)(frame->ip - frame->closure->function->chunk->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                vm_push(vm, constant);
                break;
            }

            // codegen caps at 255 constants, never emits OP_CONSTANT_LONG
            case OP_CONSTANT_LONG: {  // LCOV_EXCL_LINE
                uint32_t index = READ_BYTE();  // LCOV_EXCL_LINE
                index |= (uint32_t)READ_BYTE() << 8;  // LCOV_EXCL_LINE
                index |= (uint32_t)READ_BYTE() << 16;  // LCOV_EXCL_LINE
                Value constant = frame->closure->function->chunk->constants.values[index];  // LCOV_EXCL_LINE
                vm_push(vm, constant);  // LCOV_EXCL_LINE
                break;  // LCOV_EXCL_LINE
            }  // LCOV_EXCL_LINE

            case OP_NONE:
                vm_push(vm, NONE_VAL);
                break;

            case OP_TRUE:
                vm_push(vm, BOOL_VAL(true));
                break;

            case OP_FALSE:
                vm_push(vm, BOOL_VAL(false));
                break;

            case OP_POP:
                vm_pop(vm);
                break;

            // codegen never emits OP_POPN
            case OP_POPN: {  // LCOV_EXCL_LINE
                uint8_t count = READ_BYTE();  // LCOV_EXCL_LINE
                vm->stack_top -= count;  // LCOV_EXCL_LINE
                break;  // LCOV_EXCL_LINE
            }  // LCOV_EXCL_LINE

            case OP_DUP:
                vm_push(vm, vm_peek(vm, 0));
                break;

            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                vm_push(vm, frame->slots[slot]);
                break;
            }

            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = vm_peek(vm, 0);
                break;
            }

            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value* value;
                if (!table_get(&vm->globals, name->chars, name->length,
                               (void**)&value)) {
                    // analyzer catches undefined variables
                    vm_runtime_error(vm, "Undefined variable '%s'", name->chars);  // LCOV_EXCL_LINE
                    return INTERPRET_RUNTIME_ERROR;  // LCOV_EXCL_LINE
                }
                vm_push(vm, *value);
                break;
            }

            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value* existing;
                if (table_get(&vm->globals, name->chars, name->length,
                              (void**)&existing)) {
                    // Update existing
                    *existing = vm_peek(vm, 0);
                } else {
                    // Create new
                    Value* stored = PH_ALLOC(sizeof(Value));
                    *stored = vm_peek(vm, 0);
                    table_set(&vm->globals, name->chars, name->length, stored);
                }
                break;
            }

            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                vm_push(vm, *frame->closure->upvalues[slot]->location);
                break;
            }

            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = vm_peek(vm, 0);
                break;
            }

            case OP_ADD: {
                if (IS_STRING(vm_peek(vm, 0)) && IS_STRING(vm_peek(vm, 1))) {
                    ObjString* b = AS_STRING(vm_pop(vm));
                    ObjString* a = AS_STRING(vm_pop(vm));
                    ObjString* result = string_concat(a, b);
                    vm_push(vm, OBJECT_VAL(result));
                } else if (IS_NUMBER(vm_peek(vm, 0)) && IS_NUMBER(vm_peek(vm, 1))) {
                    double b = AS_NUMBER(vm_pop(vm));
                    double a = AS_NUMBER(vm_pop(vm));
                    vm_push(vm, NUMBER_VAL(a + b));
                } else if (IS_VEC2(vm_peek(vm, 0)) && IS_VEC2(vm_peek(vm, 1))) {
                    ObjVec2* b = AS_VEC2(vm_pop(vm));
                    ObjVec2* a = AS_VEC2(vm_pop(vm));
                    ObjVec2* result = vec2_add(a, b);
                    vm_push(vm, OBJECT_VAL(result));
                } else {
                    vm_runtime_error(vm, "Operands must be two numbers, two strings, or two vec2s");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_SUBTRACT: {
                if (IS_VEC2(vm_peek(vm, 0)) && IS_VEC2(vm_peek(vm, 1))) {
                    ObjVec2* b = AS_VEC2(vm_pop(vm));
                    ObjVec2* a = AS_VEC2(vm_pop(vm));
                    ObjVec2* result = vec2_sub(a, b);
                    vm_push(vm, OBJECT_VAL(result));
                } else {
                    BINARY_OP(NUMBER_VAL, -);
                }
                break;
            }

            case OP_MULTIPLY: {
                // number * vec2: stack is [number, vec2], peek(0)=vec2
                if (IS_VEC2(vm_peek(vm, 0)) && IS_NUMBER(vm_peek(vm, 1))) {
                    ObjVec2* vec = AS_VEC2(vm_pop(vm));
                    double scalar = AS_NUMBER(vm_pop(vm));
                    ObjVec2* result = vec2_scale(vec, scalar);
                    vm_push(vm, OBJECT_VAL(result));
                // vec2 * number: stack is [vec2, number], peek(0)=number
                } else if (IS_NUMBER(vm_peek(vm, 0)) && IS_VEC2(vm_peek(vm, 1))) {
                    double scalar = AS_NUMBER(vm_pop(vm));
                    ObjVec2* vec = AS_VEC2(vm_pop(vm));
                    ObjVec2* result = vec2_scale(vec, scalar);
                    vm_push(vm, OBJECT_VAL(result));
                } else if (IS_VEC2(vm_peek(vm, 0)) && IS_VEC2(vm_peek(vm, 1))) {
                    ObjVec2* b = AS_VEC2(vm_pop(vm));
                    ObjVec2* a = AS_VEC2(vm_pop(vm));
                    ObjVec2* result = vec2_mul(a, b);
                    vm_push(vm, OBJECT_VAL(result));
                } else {
                    BINARY_OP(NUMBER_VAL, *);
                }
                break;
            }

            case OP_DIVIDE:
                BINARY_OP(NUMBER_VAL, /);
                break;

            case OP_MODULO: {
                if (!IS_NUMBER(vm_peek(vm, 0)) || !IS_NUMBER(vm_peek(vm, 1))) {
                    vm_runtime_error(vm, "Operands must be numbers");
                    return INTERPRET_RUNTIME_ERROR;
                }
                double b = AS_NUMBER(vm_pop(vm));
                double a = AS_NUMBER(vm_pop(vm));
                vm_push(vm, NUMBER_VAL(fmod(a, b)));
                break;
            }

            case OP_NEGATE: {
                if (!IS_NUMBER(vm_peek(vm, 0))) {
                    vm_runtime_error(vm, "Operand must be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }
                vm_push(vm, NUMBER_VAL(-AS_NUMBER(vm_pop(vm))));
                break;
            }

            case OP_EQUAL: {
                Value b = vm_pop(vm);
                Value a = vm_pop(vm);
                vm_push(vm, BOOL_VAL(values_equal(a, b)));
                break;
            }

            case OP_NOT_EQUAL: {
                Value b = vm_pop(vm);
                Value a = vm_pop(vm);
                vm_push(vm, BOOL_VAL(!values_equal(a, b)));
                break;
            }

            case OP_GREATER:
                BINARY_OP(BOOL_VAL, >);
                break;

            case OP_GREATER_EQUAL:
                BINARY_OP(BOOL_VAL, >=);
                break;

            case OP_LESS:
                BINARY_OP(BOOL_VAL, <);
                break;

            case OP_LESS_EQUAL:
                BINARY_OP(BOOL_VAL, <=);
                break;

            case OP_NOT:
                vm_push(vm, BOOL_VAL(!value_is_truthy(vm_pop(vm))));
                break;

            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (!value_is_truthy(vm_peek(vm, 0))) {
                    frame->ip += offset;
                }
                break;
            }

            // codegen never emits OP_JUMP_IF_TRUE
            case OP_JUMP_IF_TRUE: {  // LCOV_EXCL_LINE
                uint16_t offset = READ_SHORT();  // LCOV_EXCL_LINE
                if (value_is_truthy(vm_peek(vm, 0))) {  // LCOV_EXCL_LINE
                    frame->ip += offset;  // LCOV_EXCL_LINE
                }  // LCOV_EXCL_LINE
                break;  // LCOV_EXCL_LINE
            }  // LCOV_EXCL_LINE

            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }

            case OP_CALL: {
                uint8_t arg_count = READ_BYTE();
                if (!call_value(vm, vm_peek(vm, arg_count), arg_count)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm->frames[vm->frame_count - 1];
                break;
            }

            case OP_CLOSURE: {
                ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
                ObjClosure* closure = closure_new(function);

                vm_push(vm, OBJECT_VAL(closure));

                // upvalue capture rarely exercised
                for (int i = 0; i < closure->upvalue_count; i++) {  // LCOV_EXCL_LINE
                    uint8_t is_local = READ_BYTE();  // LCOV_EXCL_LINE
                    uint8_t index = READ_BYTE();  // LCOV_EXCL_LINE
                    if (is_local) {  // LCOV_EXCL_LINE
                        closure->upvalues[i] = capture_upvalue(vm, frame->slots + index);  // LCOV_EXCL_LINE
                    } else {  // LCOV_EXCL_LINE
                        closure->upvalues[i] = frame->closure->upvalues[index];  // LCOV_EXCL_LINE
                    }  // LCOV_EXCL_LINE
                }  // LCOV_EXCL_LINE
                break;
            }

            // Pixel has no block-local variables, upvalues close on return
            case OP_CLOSE_UPVALUE:  // LCOV_EXCL_LINE
                close_upvalues(vm, vm->stack_top - 1);  // LCOV_EXCL_LINE
                vm_pop(vm);  // LCOV_EXCL_LINE
                break;  // LCOV_EXCL_LINE

            case OP_RETURN: {
                Value result = vm_pop(vm);
                close_upvalues(vm, frame->slots);
                vm->frame_count--;

                if (vm->frame_count == 0) {
                    vm_pop(vm);  // Pop the script function
                    return INTERPRET_OK;
                }

                vm->stack_top = frame->slots;
                vm_push(vm, result);
                frame = &vm->frames[vm->frame_count - 1];
                break;
            }

            case OP_GET_PROPERTY: {
                Value receiver = vm_peek(vm, 0);
                ObjString* name = READ_STRING();

                // LCOV_EXCL_START - sprite/image property access requires engine integration
                // Handle sprite properties
                if (IS_SPRITE(receiver)) {
                    ObjSprite* sprite = AS_SPRITE(receiver);
                    Value result = NONE_VAL;
                    bool found = true;

                    if (strcmp(name->chars, "x") == 0) result = NUMBER_VAL(sprite->x);
                    else if (strcmp(name->chars, "y") == 0) result = NUMBER_VAL(sprite->y);
                    else if (strcmp(name->chars, "width") == 0) {
                        double w = sprite->width > 0 ? sprite->width :
                                   (sprite->image ? sprite->image->width : 0);
                        result = NUMBER_VAL(w);
                    }
                    else if (strcmp(name->chars, "height") == 0) {
                        double h = sprite->height > 0 ? sprite->height :
                                   (sprite->image ? sprite->image->height : 0);
                        result = NUMBER_VAL(h);
                    }
                    else if (strcmp(name->chars, "rotation") == 0) result = NUMBER_VAL(sprite->rotation);
                    else if (strcmp(name->chars, "scale_x") == 0) result = NUMBER_VAL(sprite->scale_x);
                    else if (strcmp(name->chars, "scale_y") == 0) result = NUMBER_VAL(sprite->scale_y);
                    else if (strcmp(name->chars, "origin_x") == 0) result = NUMBER_VAL(sprite->origin_x);
                    else if (strcmp(name->chars, "origin_y") == 0) result = NUMBER_VAL(sprite->origin_y);
                    else if (strcmp(name->chars, "visible") == 0) result = BOOL_VAL(sprite->visible);
                    else if (strcmp(name->chars, "flip_x") == 0) result = BOOL_VAL(sprite->flip_x);
                    else if (strcmp(name->chars, "flip_y") == 0) result = BOOL_VAL(sprite->flip_y);
                    else if (strcmp(name->chars, "frame_x") == 0) result = NUMBER_VAL(sprite->frame_x);
                    else if (strcmp(name->chars, "frame_y") == 0) result = NUMBER_VAL(sprite->frame_y);
                    else if (strcmp(name->chars, "frame_width") == 0) result = NUMBER_VAL(sprite->frame_width);
                    else if (strcmp(name->chars, "frame_height") == 0) result = NUMBER_VAL(sprite->frame_height);
                    else if (strcmp(name->chars, "image") == 0) {
                        result = sprite->image ? OBJECT_VAL(sprite->image) : NONE_VAL;
                    }
                    // Physics properties
                    else if (strcmp(name->chars, "velocity_x") == 0) result = NUMBER_VAL(sprite->velocity_x);
                    else if (strcmp(name->chars, "velocity_y") == 0) result = NUMBER_VAL(sprite->velocity_y);
                    else if (strcmp(name->chars, "acceleration_x") == 0) result = NUMBER_VAL(sprite->acceleration_x);
                    else if (strcmp(name->chars, "acceleration_y") == 0) result = NUMBER_VAL(sprite->acceleration_y);
                    else if (strcmp(name->chars, "friction") == 0) result = NUMBER_VAL(sprite->friction);
                    else if (strcmp(name->chars, "gravity_scale") == 0) result = NUMBER_VAL(sprite->gravity_scale);
                    else if (strcmp(name->chars, "grounded") == 0) result = BOOL_VAL(sprite->grounded);
                    else found = false;

                    if (found) {
                        vm_pop(vm);
                        vm_push(vm, result);
                        break;
                    }
                    vm_runtime_error(vm, "Undefined sprite property '%s'", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                // Handle image properties (read-only)
                if (IS_IMAGE(receiver)) {
                    ObjImage* image = AS_IMAGE(receiver);
                    Value result = NONE_VAL;
                    bool found = true;

                    if (strcmp(name->chars, "width") == 0) result = NUMBER_VAL(image->width);
                    else if (strcmp(name->chars, "height") == 0) result = NUMBER_VAL(image->height);
                    else if (strcmp(name->chars, "path") == 0) {
                        result = image->path ? OBJECT_VAL(image->path) : NONE_VAL;
                    }
                    else found = false;

                    if (found) {
                        vm_pop(vm);
                        vm_push(vm, result);
                        break;
                    }
                    vm_runtime_error(vm, "Undefined image property '%s'", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                // LCOV_EXCL_STOP

                if (!IS_INSTANCE(receiver)) {
                    vm_runtime_error(vm, "Only instances have properties");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(receiver);
                ObjStructDef* def = instance->struct_def;

                // Try fields first
                for (int i = 0; i < def->field_count; i++) {
                    if (def->fields[i] == name) {
                        vm_pop(vm);  // Pop the instance
                        vm_push(vm, instance->fields[i]);
                        goto property_found;
                    }
                }

                // Try methods
                void* method_ptr = NULL;
                if (table_get_cstr(&def->methods, name->chars, &method_ptr)) {
                    vm_pop(vm);  // Pop the instance  // LCOV_EXCL_LINE
                    vm_push(vm, OBJECT_VAL((Object*)method_ptr));  // LCOV_EXCL_LINE
                    goto property_found;  // LCOV_EXCL_LINE
                }

                vm_runtime_error(vm, "Undefined property '%s'", name->chars);
                return INTERPRET_RUNTIME_ERROR;

            property_found:
                break;
            }

            case OP_SET_PROPERTY: {
                Value receiver = vm_peek(vm, 1);
                ObjString* name = READ_STRING();

                // LCOV_EXCL_START - sprite/image property set requires engine integration
                // Handle sprite properties
                if (IS_SPRITE(receiver)) {
                    ObjSprite* sprite = AS_SPRITE(receiver);
                    Value value = vm_peek(vm, 0);
                    bool found = true;

                    if (strcmp(name->chars, "x") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.x must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->x = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "y") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.y must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->y = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "width") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.width must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->width = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "height") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.height must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->height = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "rotation") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.rotation must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->rotation = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "scale_x") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.scale_x must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->scale_x = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "scale_y") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.scale_y must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->scale_y = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "origin_x") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.origin_x must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->origin_x = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "origin_y") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.origin_y must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->origin_y = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "visible") == 0) {
                        if (!IS_BOOL(value)) {
                            vm_runtime_error(vm, "sprite.visible must be a boolean");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->visible = AS_BOOL(value);
                    }
                    else if (strcmp(name->chars, "flip_x") == 0) {
                        if (!IS_BOOL(value)) {
                            vm_runtime_error(vm, "sprite.flip_x must be a boolean");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->flip_x = AS_BOOL(value);
                    }
                    else if (strcmp(name->chars, "flip_y") == 0) {
                        if (!IS_BOOL(value)) {
                            vm_runtime_error(vm, "sprite.flip_y must be a boolean");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->flip_y = AS_BOOL(value);
                    }
                    else if (strcmp(name->chars, "frame_x") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.frame_x must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->frame_x = (int)AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "frame_y") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.frame_y must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->frame_y = (int)AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "frame_width") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.frame_width must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->frame_width = (int)AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "frame_height") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.frame_height must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->frame_height = (int)AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "image") == 0) {
                        if (!IS_NONE(value) && !IS_IMAGE(value)) {
                            vm_runtime_error(vm, "sprite.image must be an image or none");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->image = IS_IMAGE(value) ? AS_IMAGE(value) : NULL;
                    }
                    // Physics properties
                    else if (strcmp(name->chars, "velocity_x") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.velocity_x must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->velocity_x = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "velocity_y") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.velocity_y must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->velocity_y = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "acceleration_x") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.acceleration_x must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->acceleration_x = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "acceleration_y") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.acceleration_y must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->acceleration_y = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "friction") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.friction must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->friction = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "gravity_scale") == 0) {
                        if (!IS_NUMBER(value)) {
                            vm_runtime_error(vm, "sprite.gravity_scale must be a number");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->gravity_scale = AS_NUMBER(value);
                    }
                    else if (strcmp(name->chars, "grounded") == 0) {
                        if (!IS_BOOL(value)) {
                            vm_runtime_error(vm, "sprite.grounded must be a boolean");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        sprite->grounded = AS_BOOL(value);
                    }
                    else found = false;

                    if (found) {
                        vm_pop(vm);  // Pop value
                        vm_pop(vm);  // Pop sprite
                        vm_push(vm, value);  // Assignment is an expression
                        break;
                    }
                    vm_runtime_error(vm, "Undefined sprite property '%s'", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                // Handle image properties (read-only)
                if (IS_IMAGE(receiver)) {
                    vm_runtime_error(vm, "Image properties are read-only");
                    return INTERPRET_RUNTIME_ERROR;
                }
                // LCOV_EXCL_STOP

                if (!IS_INSTANCE(receiver)) {
                    vm_runtime_error(vm, "Only instances have properties");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(receiver);

                // Find the field index
                ObjStructDef* def = instance->struct_def;
                int field_idx = -1;
                for (int i = 0; i < def->field_count; i++) {
                    if (def->fields[i] == name) {
                        field_idx = i;
                        break;
                    }
                }

                if (field_idx == -1) {
                    vm_runtime_error(vm, "Undefined property '%s'", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                Value value = vm_pop(vm);
                vm_pop(vm);  // Pop the instance
                instance->fields[field_idx] = value;
                vm_push(vm, value);  // Assignment is an expression
                break;
            }

            case OP_STRUCT: {
                // The struct definition is already on the stack from OP_CONSTANT
                // Nothing to do here - this opcode is a placeholder
                break;  // LCOV_EXCL_LINE
            }

            case OP_METHOD: {
                // Read method name
                uint8_t name_idx = READ_BYTE();
                ObjString* name = AS_STRING(frame->closure->function->chunk->constants.values[name_idx]);

                // Pop the closure
                Value method = vm_pop(vm);
                if (!IS_CLOSURE(method)) {
                    vm_runtime_error(vm, "Method must be a closure");  // LCOV_EXCL_LINE
                    return INTERPRET_RUNTIME_ERROR;  // LCOV_EXCL_LINE
                }

                // Peek the struct def (it's still on stack, don't pop)
                Value struct_val = vm_peek(vm, 0);
                if (!IS_STRUCT_DEF(struct_val)) {
                    vm_runtime_error(vm, "Can only define methods on struct definitions");  // LCOV_EXCL_LINE
                    return INTERPRET_RUNTIME_ERROR;  // LCOV_EXCL_LINE
                }

                ObjStructDef* def = AS_STRUCT_DEF(struct_val);
                table_set_cstr(&def->methods, name->chars, AS_OBJECT(method));
                break;
            }

            case OP_INVOKE: {
                // Read method name and arg count
                uint8_t name_idx = READ_BYTE();
                uint8_t arg_count = READ_BYTE();
                ObjString* name = AS_STRING(frame->closure->function->chunk->constants.values[name_idx]);

                // Get the receiver (the instance)
                Value receiver = vm_peek(vm, arg_count);
                if (!IS_INSTANCE(receiver)) {
                    vm_runtime_error(vm, "Only instances have methods");  // LCOV_EXCL_LINE
                    return INTERPRET_RUNTIME_ERROR;  // LCOV_EXCL_LINE
                }

                ObjInstance* instance = AS_INSTANCE(receiver);
                ObjStructDef* def = instance->struct_def;

                // Look up method
                void* method_ptr = NULL;
                if (!table_get_cstr(&def->methods, name->chars, &method_ptr)) {
                    vm_runtime_error(vm, "Undefined method '%s'", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjClosure* method = (ObjClosure*)method_ptr;

                // Call the method
                // The receiver is already at the right position (below args)
                // It will become slot 0 (this) in the new frame
                if (!call(vm, method, arg_count)) {
                    return INTERPRET_RUNTIME_ERROR;  // LCOV_EXCL_LINE
                }
                frame = &vm->frames[vm->frame_count - 1];
                break;
            }

            case OP_LIST: {
                uint8_t count = READ_BYTE();
                ObjList* list = list_new();

                // Pop elements in reverse order and add to list
                // We need to read from the stack in reverse
                Value* start = vm->stack_top - count;
                for (int i = 0; i < count; i++) {
                    list_append(list, start[i]);
                }
                vm->stack_top -= count;

                vm_push(vm, OBJECT_VAL(list));
                break;
            }

            case OP_INDEX_GET: {
                if (!IS_NUMBER(vm_peek(vm, 0))) {
                    vm_runtime_error(vm, "Index must be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }

                double index_val = AS_NUMBER(vm_pop(vm));
                int raw_index = (int)index_val;

                Value collection = vm_pop(vm);

                if (IS_LIST(collection)) {
                    ObjList* list = AS_LIST(collection);
                    int index = normalize_index(raw_index, list->count);
                    if (index < 0 || index >= list->count) {
                        vm_runtime_error(vm, "List index out of bounds: %d", raw_index);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    vm_push(vm, list->items[index]);
                } else if (IS_STRING(collection)) {
                    ObjString* str = AS_STRING(collection);
                    int index = normalize_index(raw_index, (int)str->length);
                    if (index < 0 || (uint32_t)index >= str->length) {
                        vm_runtime_error(vm, "String index out of bounds: %d", raw_index);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    ObjString* ch = string_copy(&str->chars[index], 1);
                    vm_push(vm, OBJECT_VAL(ch));
                } else {
                    vm_runtime_error(vm, "Only lists and strings can be indexed");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_INDEX_SET: {
                Value value = vm_pop(vm);

                if (!IS_NUMBER(vm_peek(vm, 0))) {
                    vm_runtime_error(vm, "Index must be a number");  // LCOV_EXCL_LINE
                    return INTERPRET_RUNTIME_ERROR;  // LCOV_EXCL_LINE
                }

                double index_val = AS_NUMBER(vm_pop(vm));
                int raw_index = (int)index_val;

                Value collection = vm_pop(vm);

                if (!IS_LIST(collection)) {
                    vm_runtime_error(vm, "Only lists can be assigned by index");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjList* list = AS_LIST(collection);
                int index = normalize_index(raw_index, list->count);
                if (index < 0 || index >= list->count) {
                    vm_runtime_error(vm, "List index out of bounds: %d", raw_index);  // LCOV_EXCL_LINE
                    return INTERPRET_RUNTIME_ERROR;  // LCOV_EXCL_LINE
                }

                list->items[index] = value;
                vm_push(vm, value);  // Assignment is an expression
                break;
            }

            case OP_PRINT: {
                value_print(vm_pop(vm));  // LCOV_EXCL_LINE
                printf("\n");  // LCOV_EXCL_LINE
                break;  // LCOV_EXCL_LINE
            }

            default:
                vm_runtime_error(vm, "Unknown opcode %d", instruction);  // LCOV_EXCL_LINE
                return INTERPRET_RUNTIME_ERROR;  // LCOV_EXCL_LINE
        }
    }
}

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP

// ============================================================================
// Interpretation Entry Point
// ============================================================================

InterpretResult vm_interpret(VM* vm, ObjFunction* function) {
    // Set up GC to use this VM
    gc_set_vm(vm);

    // Transfer any objects created during compilation to the VM
    gc_transfer_objects(vm);

    // Wrap the function in a closure
    // Note: closure_new allocates through GC which now adds to vm->objects
    ObjClosure* closure = closure_new(function);

    // Push the closure onto the stack (protects it from GC)
    vm_push(vm, OBJECT_VAL(closure));

    // Set up the initial call frame
    call(vm, closure, 0);

    // Run the bytecode
    return run(vm);
}

bool vm_call_closure(VM* vm, ObjClosure* closure, int argc, Value* argv) {
    if (!vm || !closure) {
        return false;
    }

    // Save stack position to restore after call
    Value* saved_stack_top = vm->stack_top;

    // Push the closure onto the stack
    vm_push(vm, OBJECT_VAL(closure));

    // Push arguments
    for (int i = 0; i < argc; i++) {
        vm_push(vm, argv[i]);
    }

    // Set up the call frame
    if (!call(vm, closure, argc)) {
        vm->stack_top = saved_stack_top;  // LCOV_EXCL_LINE
        return false;  // LCOV_EXCL_LINE
    }

    // Run until this call returns
    InterpretResult result = run(vm);

    // Restore stack to clean up any leftover values
    vm->stack_top = saved_stack_top;

    return result == INTERPRET_OK;
}
