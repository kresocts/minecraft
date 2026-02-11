#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdbool.h>
#include "core/block_id.h"   // BlockId + BLOCK_* konstante

typedef enum BlockFace {
    FACE_FRONT  = 0, // +Z
    FACE_BACK   = 1, // -Z
    FACE_LEFT   = 2, // -X
    FACE_RIGHT  = 3, // +X
    FACE_TOP    = 4, // +Y
    FACE_BOTTOM = 5, // -Y
    FACE_COUNT  = 6
} BlockFace;

typedef struct BlockDef {
    const char *name;
    bool solid;

    // linearni tile index u atlasu (32x32 grid), -1 = nema
    // tileId -> (tx = tileId % atlas->tilesX, ty = tileId / atlas->tilesX)
    int tile[FACE_COUNT];
} BlockDef;

typedef struct BlockRegistry {
    BlockDef defs[256];
} BlockRegistry;

void Blocks_Init(BlockRegistry *br);
const BlockDef *Blocks_Get(const BlockRegistry *br, BlockId id);

#endif
