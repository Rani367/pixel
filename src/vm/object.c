    case OBJ_MUSIC: {
            ObjMusic* music = (ObjMusic*)object;
            music_destroy_handle(music);
            break;
        }
        case OBJ_CAMERA:
            // Camera has no external resources to free
            break;
        case OBJ_ANIMATION: {
            ObjAnimation* anim = (ObjAnimation*)object;
            if (anim->frames) {
                PH_FREE(anim->frames);
            }
            break;
        }
        case OBJ_PARTICLE_EMITTER:
            // Particle emitter has no external resources to free
            // (particles are stored in fixed-size array inside the object)
            break;
    }

    PH_FREE(object);
}

// ============================================================================
// String Interning - Weak Reference Support for GC
// ============================================================================

// Tombstone marker (must match table.c)
#define TOMBSTONE_KEY ((const char*)1)

void strings_remove_white(void) {
    // Remove unmarked strings from the intern table
    // This is called during GC before the sweep phase
    for (int i = 0; i < strings.capacity; i++) {
        TableEntry* entry = &strings.entries[i];
        if (entry->key != NULL && entry->key != TOMBSTONE_KEY) {
            // The value is an ObjString*
            ObjString* string = (ObjString*)entry->value;
            if (!string->obj.marked) {
                // String is unmarked (will be freed), remove from table
                entry->key = TOMBSTONE_KEY;
                entry->key_length = 0;
                entry->value = NULL;
            }
        }
    }
}
