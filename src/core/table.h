#ifndef PH_TABLE_H
#define PH_TABLE_H

#include "common.h"

// Hash table entry
typedef struct {
    const char* key;    // NULL means empty slot
    size_t key_length;
    uint32_t hash;
    void* value;
} TableEntry;

// Open-addressed hash table with linear probing
typedef struct {
    TableEntry* entries;
    int count;      // Number of entries (including tombstones for load factor)
    int capacity;
} Table;

// Initialize a table
void table_init(Table* table);

// Free a table's memory (does not free keys or values)
void table_free(Table* table);

// Set a key-value pair (returns true if key was new)
bool table_set(Table* table, const char* key, size_t key_length, void* value);

// Get a value by key (returns true if found, stores value in out_value)
bool table_get(Table* table, const char* key, size_t key_length, void** out_value);

// Delete a key (returns true if key existed)
bool table_delete(Table* table, const char* key, size_t key_length);

// Find a string in the table (for string interning)
// Returns the key pointer if found, NULL otherwise
const char* table_find_string(Table* table, const char* chars,
                               size_t length, uint32_t hash);

// Convenience wrappers for null-terminated strings
bool table_set_cstr(Table* table, const char* key, void* value);
bool table_get_cstr(Table* table, const char* key, void** out_value);
bool table_delete_cstr(Table* table, const char* key);

#endif // PH_TABLE_H
