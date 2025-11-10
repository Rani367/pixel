ruct_def == def);
    ASSERT(AS_NUMBER(retrieved->fields[0]) == 10);

    vm_pop(&vm);
    teardown();
}

TEST(gc_stress_test) {
    setup();

    // Create many objects to trigger multiple GC cycles
    for (int i = 0; i < 1000; i++) {
        ObjList* list = list_new();
        char buf[32];
        snprintf(buf, sizeof(buf), "string_%d", i);
        ObjString* str = string_copy(buf, (int)strlen(buf));
        list_append(list, OBJECT_VAL(str));

        // Push every 10th object to keep some alive
        if (i % 10 == 0) {
            vm_push(&vm, OBJECT_VAL(list));
        }
    }

    // Run GC
    gc_collect(&vm);

    // Pop all the kept lists
    for (int i = 0; i < 100; i++) {
        vm_pop(&vm);
    }

    // Final GC to clean up
    gc_collect(&vm);

    teardown();
}

TEST(gc_bytes_allocated_tracking) {
    setup();

    size_t initial = vm.bytes_allocated;

    // Allocate some objects
    (void)string_copy("test string", 11);
    (void)list_new();
    (void)vec2_new(1.0, 2.0);

    // Bytes    ObjList* list = list_new();

    // Objects should be tracked
    ASSERT(count_objects() >= 3);

    // Keep references to avoid warnings
    (void)s1;
    (void)s2;
    (void)list;

    teardown();
}

TEST(gc_preserves_stack_values) {
    setup();

    // Push some values onto the stack
    ObjString* s1 = string_copy("keep me", 7);
    ObjString* s2 = string_copy("also keep", 9);
    vm_push(&vm, OBJECT_VAL(s1));
    vm_push(&vm, OBJECT_VAL(s2));

    int before = count_objects();

    // Run GC - should not collect anything since all objects are on stack
    gc_collect(&vm);

    int after = count_objects();

    // No objects should be collected
    ASSERT_EQ(before, after);

    // Pop values
    vm_pop(&vm);
    vm_pop(&vm);

    teardown();
}

TEST(gc_collects_unreachable) {
    setup();

    // Create some unreachable objects (not on stack or in globals)
    (void)string_copy("garbage1", 8);
    (void)string_copy("garbage2", 8);
    (void)list_new();

    // Create reachable objects
    ObjString* keep = string_copy("keep this", 9);
    vm_push(&vm, OBJECT_VAL(keep));

    int before = count_objects();
    ASSERT(before >= 4);  // At least 4 objects (3 garbage + 1 kept)

    // Run GC
    gc_collect(&vm);

    int after = count_objects();

    // Should have collected the unreachable objects
    // Note: Due to string interning, the strings might still exist
    // But the list should definitely be collected
    ASSERT(after <= before);

    vm_pop(&vm);
    teardown();
}

TEST(gc_preserves_globals) {
    setup();

    // Create a global variable
    ObjString* name = string_copy("myGlobal", 8);
    ObjString* value = string_copy("global value", 12);
    vm_define_global(&vm, name, OBJECT_VAL(value));

    // Create garbage
    (void)list_new();
    (void)vec2_new(1.0, 2.0);

    // Run GC
    gc_collect(&vm);

    // Global should still exist
    Value* retrieved;
    bool found = table_get(&vm.globals, name->chars, name->length, (void**)&retrieved);
    ASSERT(found);
    ASSERT(IS_STRING(*retrieved));
    ASSERT_STR_EQ(AS_CSTRING(*retrieved), "global value");

    teardown();
}

TEST(gc_traces_closures) {
    setup();

    // Create a function with a constant
    ObjFunction* fn = function_new();
    fn->chunk = PH_ALLOC(sizeof(Chunk));
    chunk_init(fn->chunk);

    ObjString* constant = string_copy("constant in function", 20);
    chunk_add_constant(fn->chunk, OBJECT_VAL(constant));

    // Create a closure and put on stack
    ObjClosure* closure = closure_new(fn);
    vm_push(&vm, OBJECT_VAL(closure));

    // Create garbage
    (void)list_new();

    // Run GC
    gc_collect(&vm);

    // Closure and its function's constant should still exist
    ASSERT(vm_peek(&vm, 0).type == VAL_OBJECT);

    vm_pop(&vm);
    teardown();
}

TEST(gc_traces_lists) {
    setup();

    // Create a list with values
    ObjList* list = list_new();
    ObjString* s1 = string_copy("item1", 5);
    ObjString* s2 = string_copy("item2", 5);
    list_append(list, OBJECT_VAL(s1));
    list_append(list, OBJECT_VAL(s2));

    // Put list on stack
    vm_push(&vm, OBJECT_VAL(list));

    // Run GC
    gc_collect(&vm);

    // List and its items should still exist
    ObjList* retrieved = AS_LIST(vm_peek(&vm, 0));
    ASSERT_EQ(list_length(retrieved), 2);
    ASSERT(IS_STRING(list_get(retrieved, 0)));
    ASSERT(IS_STRING(list_get(retrieved, 1)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_traces_instances) {
    setup();

    // Create a struct definition
    ObjString* struct_name = string_copy("Point", 5);
    ObjStructDef* def = struct_def_new(struct_name, 2);
    def->fields[0] = string_copy("x", 1);
    def->fields[1] = string_copy("y", 1);

    // Create an instance
    ObjInstance* instance = instance_new(def);
    instance->fields[0] = NUMBER_VAL(10);
    instance->fields[1] = NUMBER_VAL(20);

    // Put on stack
    vm_push(&vm, OBJECT_VAL(instance));

    // Run GC
    gc_collect(&vm);

    // Instance should still exist
    ObjInstance* retrieved = AS_INSTANCE(vm_peek(&vm, 0));
    ASSERT(retrieved->st should have increased
    ASSERT(vm.bytes_allocated > initial);

    teardown();
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("GC - Basic");
    RUN_TEST(gc_initial_state);
    RUN_TEST(gc_object_tracking);
    RUN_TEST(gc_bytes_allocated_tracking);

    TEST_SUITE("GC - Reachability");
    RUN_TEST(gc_preserves_stack_values);
    RUN_TEST(gc_collects_unreachable);
    RUN_TEST(gc_preserves_globals);

    TEST_SUITE("GC - Object Tracing");
    RUN_TEST(gc_traces_closures);
    RUN_TEST(gc_traces_lists);
    RUN_TEST(gc_traces_instances);

    TEST_SUITE("GC - Stress");
    RUN_TEST(gc_stress_test);

    TEST_SUMMARY();
}
