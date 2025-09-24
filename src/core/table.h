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
bool table_set(Table* table, co