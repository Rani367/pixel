#ifndef PH_DEBUG_H
#define PH_DEBUG_H

#include "vm/chunk.h"

// Disassemble an entire chunk
// Example output:
// == main ==
// 0000    1 OP_CONSTANT     0 '1'
// 0002    | OP_CONSTANT     1 '2'
// 0004    | OP_ADD
// 0005    | OP_RETURN
void disassemble_chunk(Chunk* chun