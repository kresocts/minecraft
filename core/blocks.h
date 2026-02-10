#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdbool.h>
#include "core/block_id.h"

typedef struct BlockDef {
    const char *name;
    int tileX, tileY;
    bool solid;
} BlockDef;

typedef struct BlockRegistry {
    BlockDef defs[256];
} BlockRegistry;

void Blocks_Init(BlockRegistry *br);
const BlockDef *Blocks_Get(const BlockRegistry *br, BlockId id);

#endif
