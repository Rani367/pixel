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

    // Bytes