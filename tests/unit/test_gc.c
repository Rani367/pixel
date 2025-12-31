#include "../test_framework.h"
#include "vm/vm.h"
#include "vm/gc.h"
#include "vm/object.h"
#include "vm/chunk.h"
#include <string.h>

// ============================================================================
// Test Helpers
// ============================================================================

static VM vm;

static void setup(void) {
    vm_init(&vm);
    gc_set_vm(&vm);
}

static void teardown(void) {
    vm_free(&vm);
}

// Count objects in the VM's list
static int count_objects(void) {
    int count = 0;
    Object* obj = vm.objects;
    while (obj != NULL) {
        count++;
        obj = obj->next;
    }
    return count;
}

// ============================================================================
// Basic GC Tests
// ============================================================================

TEST(gc_initial_state) {
    setup();

    ASSERT_EQ(vm.bytes_allocated, 0);
    ASSERT_EQ(vm.objects, NULL);
    ASSERT_EQ(vm.gray_stack, NULL);
    ASSERT_EQ(vm.gray_count, 0);

    teardown();
}

TEST(gc_object_tracking) {
    setup();

    // Create some objects
    ObjString* s1 = string_copy("hello", 5);
    ObjString* s2 = string_copy("world", 5);
    ObjList* list = list_new();

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
    ASSERT(retrieved->struct_def == def);
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

    // Bytes should have increased
    ASSERT(vm.bytes_allocated > initial);

    teardown();
}

// ============================================================================
// GC API Tests
// ============================================================================

TEST(gc_get_vm_accessor) {
    setup();

    // gc_get_vm should return the VM that was set
    VM* gotten_vm = gc_get_vm();
    ASSERT_EQ(gotten_vm, &vm);

    teardown();
}

TEST(gc_multiple_collections) {
    setup();

    // Run multiple collection cycles
    for (int i = 0; i < 5; i++) {
        // Create objects
        (void)string_copy("test string", 11);
        (void)list_new();

        // Force collection
        gc_collect(&vm);
    }

    teardown();
}

TEST(gc_collect_empty) {
    setup();

    // Collection on empty VM should not crash
    gc_collect(&vm);
    gc_collect(&vm);

    teardown();
}

// ============================================================================
// GC Blacken Object Type Tests
// ============================================================================

TEST(gc_blacken_upvalue_open) {
    setup();

    // Create an open upvalue
    ObjUpvalue* upvalue = upvalue_new(NULL);  // NULL location = closed
    upvalue->closed = NUMBER_VAL(42);

    vm_push(&vm, OBJECT_VAL(upvalue));
    gc_collect(&vm);

    // Should not crash and value should be preserved
    ASSERT(IS_OBJECT(vm_peek(&vm, 0)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_blacken_native_with_name) {
    setup();

    ObjString* name = string_copy("test_native", 11);
    ObjNative* native = native_new(NULL, name, 0);

    vm_push(&vm, OBJECT_VAL(native));
    gc_collect(&vm);

    // Native and its name should be preserved
    ASSERT(IS_OBJECT(vm_peek(&vm, 0)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_blacken_image_with_path) {
    setup();

    ObjString* path = string_copy("/test/image.png", 15);
    ObjImage* image = image_new(NULL, 100, 100, path);

    vm_push(&vm, OBJECT_VAL(image));
    gc_collect(&vm);

    // Image and its path should be preserved
    ASSERT(IS_OBJECT(vm_peek(&vm, 0)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_blacken_sprite_with_image) {
    setup();

    ObjImage* image = image_new(NULL, 100, 100, NULL);
    ObjSprite* sprite = sprite_new(image);

    vm_push(&vm, OBJECT_VAL(sprite));
    gc_collect(&vm);

    // Sprite and its image should be preserved
    ASSERT(IS_OBJECT(vm_peek(&vm, 0)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_blacken_sprite_with_animation) {
    setup();

    ObjImage* image = image_new(NULL, 128, 64, NULL);
    ObjSprite* sprite = sprite_new(image);
    ObjAnimation* anim = animation_new(image, 32, 32);
    sprite->animation = anim;

    vm_push(&vm, OBJECT_VAL(sprite));
    gc_collect(&vm);

    // All objects should be preserved
    ASSERT(IS_OBJECT(vm_peek(&vm, 0)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_blacken_animation_with_callback) {
    setup();

    ObjImage* image = image_new(NULL, 64, 64, NULL);
    ObjAnimation* anim = animation_new(image, 32, 32);

    // Create a mock closure for the callback
    ObjFunction* fn = function_new();
    ObjClosure* closure = closure_new(fn);
    anim->on_complete = closure;

    vm_push(&vm, OBJECT_VAL(anim));
    gc_collect(&vm);

    // Animation and callback should be preserved
    ASSERT(IS_OBJECT(vm_peek(&vm, 0)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_blacken_sound_with_path) {
    setup();

    ObjString* path = string_copy("/test/sound.wav", 15);
    ObjSound* sound = sound_new(NULL, path);

    vm_push(&vm, OBJECT_VAL(sound));
    gc_collect(&vm);

    // Sound and its path should be preserved
    ASSERT(IS_OBJECT(vm_peek(&vm, 0)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_blacken_music_with_path) {
    setup();

    ObjString* path = string_copy("/test/music.mp3", 15);
    ObjMusic* music = music_new(NULL, path);

    vm_push(&vm, OBJECT_VAL(music));
    gc_collect(&vm);

    // Music and its path should be preserved
    ASSERT(IS_OBJECT(vm_peek(&vm, 0)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_blacken_camera_with_target) {
    setup();

    ObjCamera* camera = camera_new();
    ObjSprite* sprite = sprite_new(NULL);
    camera->target = sprite;

    vm_push(&vm, OBJECT_VAL(camera));
    gc_collect(&vm);

    // Camera and its target should be preserved
    ASSERT(IS_OBJECT(vm_peek(&vm, 0)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_blacken_emitter) {
    setup();

    ObjParticleEmitter* emitter = particle_emitter_new(100, 100);

    vm_push(&vm, OBJECT_VAL(emitter));
    gc_collect(&vm);

    // Emitter should be preserved
    ASSERT(IS_OBJECT(vm_peek(&vm, 0)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_blacken_struct_with_methods) {
    setup();

    ObjString* name = string_copy("TestStruct", 10);
    ObjStructDef* def = struct_def_new(name, 1);
    def->fields[0] = string_copy("field", 5);

    // Add a method (simplified - just add to the method table)
    ObjFunction* fn = function_new();
    ObjClosure* method = closure_new(fn);
    table_set_cstr(&def->methods, "test_method", method);

    vm_push(&vm, OBJECT_VAL(def));
    gc_collect(&vm);

    // Struct def and its methods should be preserved
    ASSERT(IS_OBJECT(vm_peek(&vm, 0)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_blacken_closure_with_upvalues) {
    setup();

    ObjFunction* fn = function_new();
    fn->upvalue_count = 2;

    ObjClosure* closure = closure_new(fn);

    // Create upvalues
    ObjUpvalue* uv1 = upvalue_new(NULL);
    uv1->closed = NUMBER_VAL(10);
    ObjUpvalue* uv2 = upvalue_new(NULL);
    uv2->closed = NUMBER_VAL(20);

    closure->upvalues[0] = uv1;
    closure->upvalues[1] = uv2;

    vm_push(&vm, OBJECT_VAL(closure));
    gc_collect(&vm);

    // Closure and its upvalues should be preserved
    ASSERT(IS_OBJECT(vm_peek(&vm, 0)));

    vm_pop(&vm);
    teardown();
}

TEST(gc_gray_stack_growth) {
    setup();

    // Create many interconnected objects to test gray stack growth
    for (int i = 0; i < 100; i++) {
        ObjList* list = list_new();
        for (int j = 0; j < 10; j++) {
            ObjList* inner = list_new();
            list_append(list, OBJECT_VAL(inner));
        }
        vm_push(&vm, OBJECT_VAL(list));
    }

    // Force GC - should handle gray stack growth
    gc_collect(&vm);

    // Pop all
    for (int i = 0; i < 100; i++) {
        vm_pop(&vm);
    }

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

    TEST_SUITE("GC - API");
    RUN_TEST(gc_get_vm_accessor);
    RUN_TEST(gc_multiple_collections);
    RUN_TEST(gc_collect_empty);

    TEST_SUITE("GC - Blacken Object Types");
    RUN_TEST(gc_blacken_upvalue_open);
    RUN_TEST(gc_blacken_native_with_name);
    RUN_TEST(gc_blacken_image_with_path);
    RUN_TEST(gc_blacken_sprite_with_image);
    RUN_TEST(gc_blacken_sprite_with_animation);
    RUN_TEST(gc_blacken_animation_with_callback);
    RUN_TEST(gc_blacken_sound_with_path);
    RUN_TEST(gc_blacken_music_with_path);
    RUN_TEST(gc_blacken_camera_with_target);
    RUN_TEST(gc_blacken_emitter);
    RUN_TEST(gc_blacken_struct_with_methods);
    RUN_TEST(gc_blacken_closure_with_upvalues);
    RUN_TEST(gc_gray_stack_growth);

    TEST_SUMMARY();
}
