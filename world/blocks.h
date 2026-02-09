#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdbool.h>
#include "chunk.h"

typedef struct BlockDef {
    const char *name;
    int tileX, tileY;   // atlas tile coords (0..)
    bool solid;
} BlockDef;

typedef struct BlockRegistry {
    BlockDef defs[256];
} BlockRegistry;

void Blocks_Init(BlockRegistry *br);
const BlockDef *Blocks_Get(const BlockRegistry *br, BlockId id);

#endif // BLOCKS_H
