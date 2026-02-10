#include "blocks.h"

static BlockDef Def(const char *name, int tx, int ty, bool solid)
{
    BlockDef d;
    d.name = name;
    d.tileX = tx;
    d.tileY = ty;
    d.solid = solid;
    return d;
}

void Blocks_Init(BlockRegistry *br)
{
    // default sve na AIR
    for (int i = 0; i < 256; i++) {
        br->defs[i] = Def("Unknown", -1, -1, false);
    }

    br->defs[BLOCK_AIR]   = Def("Air",   -1, -1, false);

    // OVDJE MAPIRAŠ TILEOVE IZ spritesheeta (primjer):
    br->defs[BLOCK_DIRT]  = Def("Dirt",   0, 0, true);
    br->defs[BLOCK_GRASS] = Def("Grass",  1, 0, true);
    br->defs[BLOCK_STONE] = Def("Stone",  2, 0, true);
}

const BlockDef *Blocks_Get(const BlockRegistry *br, BlockId id)
{
    return &br->defs[id];
}
