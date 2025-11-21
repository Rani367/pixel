#include "../test_framework.h"
#include "core/table.h"
#include "core/strings.h"

TEST(table_init_empty) {
    Table table;
    table_init(&table);

    ASSERT_NULL(table.entries);
    ASSERT_EQ(table.count, 0);
    ASSERT_EQ(table.capacity, 0);

    table_free(&table);
}

TEST(table_set_get_single) {
    Table table;
    table_init(&table);

    int value = 42;
    bool is_new = table_set_cstr(&table, "key", &value);
    ASSERT(is_new);

    void* result;
    bool found = table_get_cstr(&table, "key", &result);
    ASSERT(found);
    ASSERT_EQ(*(int*)result, 42);

    table_free(&table);
}

TEST(table_set_get_multiple) {
    Table table;
    table_init(&table);

    int values[] = {1, 2, 3, 4, 5};
    table_set_cstr(&table, "one", &values[0]);
    table_set_cstr(&table, "two", &values[1]);
    table_set_cstr(&table, "three", &values[2]);
    table_set_cstr(&table, "four", &values[3]);
    table_set_cstr(&table, "five", &values[4]);

    void* result;

    ASSERT(table_get_cstr(&table, "one", &result));
    ASSERT_EQ(*(int*)result, 1);

    ASSERT(table_get_cstr(&table, "three", &result));
    ASSERT_EQ(*(int*)result, 3);

    ASSERT(table_get_cstr(&table, "five", &result));
    ASSERT_EQ(*(int*)result, 5);

    table_free(&table);
}

TEST(table_get_not_found) {
    Table table;
    table_init(&table);

    int value = 42;
    table_set_cstr(&table, "key", &value);

    void* result;
    bool found = table_get_cstr(&table, "other", &result);
    ASSERT(!found);

    table_free(&table);
}

TEST(table_overwrite) {
    Table table;
    table_init(&table);

    int value1 = 1;
    int value2 = 2;

    bool is_new1 = table_set_cstr(&table, "key", &value1);
    ASSERT(is_new1);

    bool is_new2 = table_set_cstr(&table, "key", &value2);
    ASSERT(!is_new2);

    void* result;
    table_get_cstr(&table, "key", &result);
    ASSERT_EQ(*(int*)result, 2);

    table_free(&table);
}

TEST(table_delete) {
    Table table;
    table_init(&table);

    int value = 42;
    table_set_cstr(&table, "key", &value);

    bool deleted = table_delete_cstr(&table, "key");
    ASSERT(deleted);

    void* result;
    bool found = table_get_cstr(&table, "key", &result);
    ASSERT(!found);

    table_free(&table);
}

TEST(table_delete_not_found) {
    Table table;
    table_init(&table);

    bool deleted = table_delete_cstr(&table, "nonexistent");
    ASSERT(!deleted);

    table_free(&table);
}

TEST(table_grows_automatically) {
    Table table;
    table_init(&table);

    char keys[100][16];
    int values[100];

    for (int i = 0; i < 100; i++) {
        snprintf(keys[i], sizeof(keys[i]), "key%d", i);
        values[i] = i * 10;
        table_set_cstr(&table, keys[i], &values[i]);
    }

    // Verify all values are still accessible
    for (int i = 0; i < 100; i++) {
        void* result;
        bool found = table_get_cstr(&table, keys[i], &result);
        ASSERT(found);
        ASSERT_EQ(*(int*)result, i * 10);
    }

    table_free(&table);
}

TEST(table_find_string) {
    Table table;
    table_init(&table);

    const char* key1 = "hello";
    const char* key2 = "world";
    int dummy = 0;

    table_set_cstr(&table, key1, &dummy);
    table_set_cstr(&table, key2, &dummy);

    uint32_t hash = ph_hash_string("hello", 5);
    const char* found = table_find_string(&table, "hello", 5, hash);
    ASSERT_NOT_NULL(found);
    ASSERT_STR_EQ(found, "hello");

    hash = ph_hash_string("notfound", 8);
    found = table_find_string(&table, "notfound", 8, hash);
    ASSERT_NULL(found);

    table_free(&table);
}

TEST(table_with_length) {
    Table table;
    table_init(&table);

    const char* data = "hello world";
    int value = 42;

    // Set using first 5 chars ("hello")
    table_set(&table, data, 5, &value);

    // Should find with same length
    void* result;
    bool found = table_get(&table, data, 5, &result);
    ASSERT(found);
    ASSERT_EQ(*(int*)result, 42);

    // Should not find with different length
    found = table_get(&table, data, 6, &result);
    ASSERT(!found);

    table_free(&table);
}

int main(void) {
    TEST_SUITE("Table");

    RUN_TEST(table_init_empty);
    RUN_TEST(table_set_get_single);
    RUN_TEST(table_set_get_multiple);
    RUN_TEST(table_get_not_found);
    RUN_TEST(table_overwrite);
    RUN_TEST(table_delete);
    RUN_TEST(table_delete_not_found);
    RUN_TEST(table_grows_automatically);
    RUN_TEST(table_find_string);
    RUN_TEST(table_with_length);

    TEST_SUMMARY();
}
