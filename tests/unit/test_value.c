#include "../test_framework.h"
#include "vm/value.h"
#include "vm/object.h"
#include "vm/gc.h"
#include <math.h>

// Setup/teardown for string interning
static void setup(void) {
    gc_init();
    strings_init();
}

static void teardown(void) {
    // Free all objects created during test
    gc_free_all();
    strings_free();
}

// ============================================================================
// Value Tests
// ============================================================================

TEST(value_none) {
    Value v = NONE_VAL;
    ASSERT(IS_NONE(v));
    ASSERT(!IS_BOOL(v));
    ASSERT(!IS_NUMBER(v));
    ASSERT(!IS_OBJECT(v));
}

TEST(value_bool) {
    Value t = BOOL_VAL(true);
    Value f = BOOL_VAL(false);

    ASSERT(IS_BOOL(t));
    ASSERT(IS_BOOL(f));
    ASSERT(AS_BOOL(t) == true);
    ASSERT(AS_BOOL(f) == false);
}

TEST(value_number) {
    Value v = NUMBER_VAL(42.5);
    ASSERT(IS_NUMBER(v));
    ASSERT(AS_NUMBER(v) == 42.5);
}

TEST(value_equality) {
    ASSERT(values_equal(NONE_VAL, NONE_VAL));
    ASSERT(values_equal(BOOL_VAL(true), BOOL_VAL(true)));
    ASSERT(values_equal(BOOL_VAL(false), BOOL_VAL(false)));
    ASSERT(!values_equal(BOOL_VAL(true), BOOL_VAL(false)));
    ASSERT(values_equal(NUMBER_VAL(42), NUMBER_VAL(42)));
    ASSERT(!values_equal(NUMBER_VAL(42), NUMBER_VAL(43)));
    ASSERT(!values_equal(NONE_VAL, BOOL_VAL(false)));
    ASSERT(!values_equal(NUMBER_VAL(0), BOOL_VAL(false)));
}

TEST(value_truthiness) {
    ASSERT(!value_is_truthy(NONE_VAL));
    ASSERT(!value_is_truthy(BOOL_VAL(false)));
    ASSERT(value_is_truthy(BOOL_VAL(true)));
    ASSERT(value_is_truthy(NUMBER_VAL(0)));  // Numbers are truthy
    ASSERT(value_is_truthy(NUMBER_VAL(1)));
}

TEST(value_array) {
    ValueArray arr;
    value_array_init(&arr);

    ASSERT_EQ(arr.count, 0);

    value_array_write(&arr, NUMBER_VAL(1));
    value_array_write(&arr, NUMBER_VAL(2));
    value_array_write(&arr, NUMBER_VAL(3));

    ASSERT_EQ(arr.count, 3);
    ASSERT(values_equal(arr.values[0], NUMBER_VAL(1)));
    ASSERT(values_equal(arr.values[1], NUMBER_VAL(2)));
    ASSERT(values_equal(arr.values[2], NUMBER_VAL(3)));

    value_array_free(&arr);
    ASSERT_EQ(arr.count, 0);
}

// ============================================================================
// String Tests
// ============================================================================

TEST(string_create) {
    setup();

    ObjString* s = string_copy("hello", 5);
    ASSERT_NOT_NULL(s);
    ASSERT_EQ(s->length, 5);
    ASSERT_STR_EQ(s->chars, "hello");

    teardown();
}

TEST(string_interning) {
    setup();

    ObjString* s1 = string_copy("hello", 5);
    ObjString* s2 = string_copy("hello", 5);
    ObjString* s3 = string_copy("world", 5);

    // Same string should return same pointer (interned)
    ASSERT(s1 == s2);
    // Different string should be different
    ASSERT(s1 != s3);

    teardown();
}

TEST(string_equality) {
    setup();

    ObjString* s1 = string_copy("hello", 5);
    ObjString* s2 = string_copy("hello", 5);

    // With interning, we can use pointer comparison
    ASSERT(STRING_EQUAL(s1, s2));

    Value v1 = OBJECT_VAL(s1);
    Value v2 = OBJECT_VAL(s2);
    ASSERT(values_equal(v1, v2));

    teardown();
}

TEST(string_concat) {
    setup();

    ObjString* a = string_copy("hello", 5);
    ObjString* b = string_copy(" world", 6);
    ObjString* c = string_concat(a, b);

    ASSERT_EQ(c->length, 11);
    ASSERT_STR_EQ(c->chars, "hello world");

    teardown();
}

TEST(string_hash) {
    uint32_t h1 = string_hash("hello", 5);
    uint32_t h2 = string_hash("hello", 5);
    uint32_t h3 = string_hash("world", 5);

    ASSERT_EQ(h1, h2);
    ASSERT(h1 != h3);
}

// ============================================================================
// List Tests
// ============================================================================

TEST(list_create) {
    setup();

    ObjList* list = list_new();
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(list_length(list), 0);

    teardown();
}

TEST(list_append) {
    setup();

    ObjList* list = list_new();
    list_append(list, NUMBER_VAL(1));
    list_append(list, NUMBER_VAL(2));
    list_append(list, NUMBER_VAL(3));

    ASSERT_EQ(list_length(list), 3);
    ASSERT(values_equal(list_get(list, 0), NUMBER_VAL(1)));
    ASSERT(values_equal(list_get(list, 1), NUMBER_VAL(2)));
    ASSERT(values_equal(list_get(list, 2), NUMBER_VAL(3)));

    teardown();
}

TEST(list_set) {
    setup();

    ObjList* list = list_new();
    list_append(list, NUMBER_VAL(1));
    list_append(list, NUMBER_VAL(2));

    list_set(list, 0, NUMBER_VAL(10));
    ASSERT(values_equal(list_get(list, 0), NUMBER_VAL(10)));

    teardown();
}

TEST(list_bounds) {
    setup();

    ObjList* list = list_new();
    list_append(list, NUMBER_VAL(1));

    // Out of bounds should return none
    ASSERT(IS_NONE(list_get(list, -1)));
    ASSERT(IS_NONE(list_get(list, 1)));

    teardown();
}

// ============================================================================
// Vec2 Tests
// ============================================================================

TEST(vec2_create) {
    setup();

    ObjVec2* v = vec2_new(3.0, 4.0);
    ASSERT_NOT_NULL(v);
    ASSERT(v->x == 3.0);
    ASSERT(v->y == 4.0);

    teardown();
}

TEST(vec2_add) {
    setup();

    ObjVec2* a = vec2_new(1.0, 2.0);
    ObjVec2* b = vec2_new(3.0, 4.0);
    ObjVec2* c = vec2_add(a, b);

    ASSERT(c->x == 4.0);
    ASSERT(c->y == 6.0);

    teardown();
}

TEST(vec2_sub) {
    setup();

    ObjVec2* a = vec2_new(5.0, 7.0);
    ObjVec2* b = vec2_new(2.0, 3.0);
    ObjVec2* c = vec2_sub(a, b);

    ASSERT(c->x == 3.0);
    ASSERT(c->y == 4.0);

    teardown();
}

TEST(vec2_scale) {
    setup();

    ObjVec2* v = vec2_new(3.0, 4.0);
    ObjVec2* scaled = vec2_scale(v, 2.0);

    ASSERT(scaled->x == 6.0);
    ASSERT(scaled->y == 8.0);

    teardown();
}

TEST(vec2_length) {
    setup();

    ObjVec2* v = vec2_new(3.0, 4.0);
    double len = vec2_length(v);

    ASSERT(len == 5.0);

    teardown();
}

TEST(vec2_normalize) {
    setup();

    ObjVec2* v = vec2_new(3.0, 4.0);
    ObjVec2* n = vec2_normalize(v);

    double len = vec2_length(n);
    ASSERT(fabs(len - 1.0) < 0.0001);

    teardown();
}

TEST(vec2_dot) {
    setup();

    ObjVec2* a = vec2_new(1.0, 2.0);
    ObjVec2* b = vec2_new(3.0, 4.0);
    double d = vec2_dot(a, b);

    ASSERT(d == 11.0);  // 1*3 + 2*4

    teardown();
}

TEST(vec2_distance) {
    setup();

    ObjVec2* a = vec2_new(0.0, 0.0);
    ObjVec2* b = vec2_new(3.0, 4.0);
    double d = vec2_distance(a, b);

    ASSERT(d == 5.0);

    teardown();
}

// ============================================================================
// Function Tests
// ============================================================================

TEST(function_create) {
    setup();

    ObjFunction* fn = function_new();
    ASSERT_NOT_NULL(fn);
    ASSERT_EQ(fn->arity, 0);
    ASSERT_EQ(fn->upvalue_count, 0);
    ASSERT_NULL(fn->name);
    ASSERT_NULL(fn->chunk);

    teardown();
}

TEST(function_with_name) {
    setup();

    ObjFunction* fn = function_new();
    fn->name = string_copy("myFunc", 6);
    fn->arity = 2;

    ASSERT_STR_EQ(fn->name->chars, "myFunc");
    ASSERT_EQ(fn->arity, 2);

    teardown();
}

// ============================================================================
// Native Function Tests
// ============================================================================

static Value test_native(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;
    return NUMBER_VAL(42);
}

TEST(native_create) {
    setup();

    ObjString* name = string_copy("testFn", 6);
    ObjNative* native = native_new(test_native, name, 0);

    ASSERT_NOT_NULL(native);
    ASSERT(native->function == test_native);
    ASSERT_EQ(native->arity, 0);

    // Call it
    Value result = native->function(0, NULL);
    ASSERT(values_equal(result, NUMBER_VAL(42)));

    teardown();
}

// ============================================================================
// Struct Tests
// ============================================================================

TEST(struct_def_create) {
    setup();

    ObjString* name = string_copy("Point", 5);
    ObjStructDef* def = struct_def_new(name, 2);

    ASSERT_NOT_NULL(def);
    ASSERT_STR_EQ(def->name->chars, "Point");
    ASSERT_EQ(def->field_count, 2);

    // Set field names
    def->fields[0] = string_copy("x", 1);
    def->fields[1] = string_copy("y", 1);

    ASSERT_STR_EQ(def->fields[0]->chars, "x");
    ASSERT_STR_EQ(def->fields[1]->chars, "y");

    teardown();
}

TEST(instance_create) {
    setup();

    ObjString* name = string_copy("Point", 5);
    ObjStructDef* def = struct_def_new(name, 2);
    def->fields[0] = string_copy("x", 1);
    def->fields[1] = string_copy("y", 1);

    ObjInstance* instance = instance_new(def);

    ASSERT_NOT_NULL(instance);
    ASSERT(instance->struct_def == def);

    // Fields should be none initially
    ASSERT(IS_NONE(instance->fields[0]));
    ASSERT(IS_NONE(instance->fields[1]));

    // Set field values
    instance->fields[0] = NUMBER_VAL(10);
    instance->fields[1] = NUMBER_VAL(20);

    ASSERT(values_equal(instance->fields[0], NUMBER_VAL(10)));
    ASSERT(values_equal(instance->fields[1], NUMBER_VAL(20)));

    teardown();
}

// ============================================================================
// Closure Tests
// ============================================================================

TEST(closure_create) {
    setup();

    ObjFunction* fn = function_new();
    fn->upvalue_count = 2;

    ObjClosure* closure = closure_new(fn);

    ASSERT_NOT_NULL(closure);
    ASSERT(closure->function == fn);
    ASSERT_EQ(closure->upvalue_count, 2);
    ASSERT_NULL(closure->upvalues[0]);
    ASSERT_NULL(closure->upvalues[1]);

    teardown();
}

TEST(upvalue_create) {
    setup();

    Value val = NUMBER_VAL(42);
    ObjUpvalue* upvalue = upvalue_new(&val);

    ASSERT_NOT_NULL(upvalue);
    ASSERT(upvalue->location == &val);
    ASSERT(IS_NONE(upvalue->closed));

    teardown();
}

// ============================================================================
// Object Type Names
// ============================================================================

TEST(object_type_names) {
    ASSERT_STR_EQ(object_type_name(OBJ_STRING), "string");
    ASSERT_STR_EQ(object_type_name(OBJ_FUNCTION), "function");
    ASSERT_STR_EQ(object_type_name(OBJ_LIST), "list");
    ASSERT_STR_EQ(object_type_name(OBJ_VEC2), "vec2");
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Value - Basic Types");
    RUN_TEST(value_none);
    RUN_TEST(value_bool);
    RUN_TEST(value_number);
    RUN_TEST(value_equality);
    RUN_TEST(value_truthiness);
    RUN_TEST(value_array);

    TEST_SUITE("Value - Strings");
    RUN_TEST(string_create);
    RUN_TEST(string_interning);
    RUN_TEST(string_equality);
    RUN_TEST(string_concat);
    RUN_TEST(string_hash);

    TEST_SUITE("Value - Lists");
    RUN_TEST(list_create);
    RUN_TEST(list_append);
    RUN_TEST(list_set);
    RUN_TEST(list_bounds);

    TEST_SUITE("Value - Vec2");
    RUN_TEST(vec2_create);
    RUN_TEST(vec2_add);
    RUN_TEST(vec2_sub);
    RUN_TEST(vec2_scale);
    RUN_TEST(vec2_length);
    RUN_TEST(vec2_normalize);
    RUN_TEST(vec2_dot);
    RUN_TEST(vec2_distance);

    TEST_SUITE("Value - Functions");
    RUN_TEST(function_create);
    RUN_TEST(function_with_name);
    RUN_TEST(native_create);

    TEST_SUITE("Value - Structs");
    RUN_TEST(struct_def_create);
    RUN_TEST(instance_create);

    TEST_SUITE("Value - Closures");
    RUN_TEST(closure_create);
    RUN_TEST(upvalue_create);

    TEST_SUITE("Value - Utilities");
    RUN_TEST(object_type_names);

    TEST_SUMMARY();
}
