#include "table.h"
#include "strings.h"
#include <string.h>

#define TABLE_MAX_LOAD 0.75

// Tombstone marker for deleted entries
#define TOMBSTONE_KEY ((const char*)1)

void table_init(Table* table) {
    table->entries = NULL;
    table->count = 0;
    table->capacity = 0;
}

void table_free(Table* table) {
    PH_FREE(table->entries);
    table_init(table);
}

static TableEntry* find_entry(TableEntry* entries, int capacity,
                               const char* key, size_t key_length, uint32_t hash) {
    uint32_t index = hash & (capacity - 1);
    TableEntry* tombstone = NULL;

    for (;;) {
        TableEntry* entry = &entries[index];

        if (entry->key == NULL) {
            // Empty slot
            return tombstone != NULL ? tombstone : entry;
        } else if (entry->key == TOMBSTONE_KEY) {
            // Tombstone - remember it but keep looking
            if (tombstone == NULL) {
                tombstone = entry;
            }
        } else if (entry->key_length == key_length &&
                   entry->hash == hash &&
                   memcmp(entry->key, key, key_length) == 0) {
            // Found the key
            return entry;
        }

        index = (index + 1) & (capacity - 1);
    }
}

static void adjust_capacity(Table* table, int new_capacity) {
    TableEntry* new_entries = PH_ALLOC(sizeof(TableEntry) * new_capacity);
    for (int i = 0; i < new_capacity; i++) {
        new_entries[i].key = NULL;
        new_entries[i].key_length = 0;
        new_entries[i].hash = 0;
        new_entries[i].value = NULL;
    }

    // Re-insert all existing entries
    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        TableEntry* entry = &table->entries[i];
        if (entry->key == NULL || entry->key == TOMBSTONE_KEY) {
            continue;
        }

        TableEntry* dest = find_entry(new_entries, new_capacity,
                                       entry->key, entry->key_length, entry->hash);
        dest->key = entry->key;
        dest->key_length = entry->key_length;
        dest->hash = entry->hash;
        dest->value = entry->value;
        table->count++;
    }

    PH_FREE(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
}

bool table_set(Table* table, const char* key, size_t key_length, void* value) {
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int new_capacity = PH_GROW_CAPACITY(table->capacity);
        adjust_capacity(table, new_capacity);
    }

    uint32_t hash = ph_hash_string(key, key_length);
    TableEntry* entry = find_entry(table->entries, table->capacity,
                                    key, key_length, hash);

    bool is_new = (entry->key == NULL);
    if (is_new || entry->key == TOMBSTONE_KEY) {
        table->count++;
    }

    entry->key = key;
    entry->key_length = key_length;
    entry->hash = hash;
    entry->value = value;
    return is_new;
}

bool table_get(Table* table, const char* key, size_t key_length, void** out_value) {
    if (table->count == 0) {
        return false;
    }

    uint32_t hash = ph_hash_string(key, key_length);
    TableEntry* entry = find_entry(table->entries, table->capacity,
                                    key, key_length, hash);

    if (entry->key == NULL || entry->key == TOMBSTONE_KEY) {
        return false;
    }

    if (out_value != NULL) {
        *out_value = entry->value;
    }
    return true;
}

bool table_delete(Table* table, const char* key, size_t key_length) {
    if (table->count == 0) {
        return false;
    }

    uint32_t hash = ph_hash_string(key, key_length);
    TableEntry* entry = find_entry(table->entries, table->capacity,
                                    key, key_length, hash);

    if (entry->key == NULL || entry->key == TOMBSTONE_KEY) {
        return false;
    }

    // Place a tombstone
    entry->key = TOMBSTONE_KEY;
    entry->key_length = 0;
    entry->value = NULL;
    return true;
}

const char* table_find_string(Table* table, const char* chars,
                               size_t length, uint32_t hash) {
    if (table->count == 0) {
        return NULL;
    }

    uint32_t index = hash & (table->capacity - 1);

    for (;;) {
        TableEntry* entry = &table->entries[index];

        if (entry->key == NULL) {
            // Empty slot - not found
            return NULL;
        } else if (entry->key != TOMBSTONE_KEY &&
                   entry->key_length == length &&
                   entry->hash == hash &&
                   memcmp(entry->key, chars, length) == 0) {
            // Found it
            return entry->key;
        }

        index = (index + 1) & (table->capacity - 1);
    }
}

bool table_set_cstr(Table* table, const char* key, void* value) {
    return table_set(table, key, strlen(key), value);
}

bool table_get_cstr(Table* table, const char* key, void** out_value) {
    return table_get(table, key, strlen(key), out_value);
}

bool table_delete_cstr(Table* table, const char* key) {
    return table_delete(table, key, strlen(key));
}
