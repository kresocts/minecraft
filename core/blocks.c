#include "blocks.h"

static BlockDef DefAll(const char *name, int tileId, bool solid)
{
    BlockDef d;
    d.name = name;
    d.solid = solid;
    for (int i = 0; i < FACE_COUNT; i++) d.tile[i] = tileId;
    return d;
}

static BlockDef DefFaces(const char *name, bool solid,
                         int front, int back, int left, int right, int top, int bottom)
{
    BlockDef d;
    d.name = name;
    d.solid = solid;

    d.tile[FACE_FRONT]  = front;
    d.tile[FACE_BACK]   = back;
    d.tile[FACE_LEFT]   = left;
    d.tile[FACE_RIGHT]  = right;
    d.tile[FACE_TOP]    = top;
    d.tile[FACE_BOTTOM] = bottom;

    return d;
}

void Blocks_Init(BlockRegistry *br)
{
    // default: sve “nepoznato” -> nema tile + nije solid
    for (int i = 0; i < 256; i++) {
        br->defs[i] = DefAll("Unknown", -1, false);
    }

    br->defs[BLOCK_AIR] = DefAll("Air", -1, false);

    // --- OVDJE definiraš blokove ---
    // redoslijed: front, back, left, right, top, bottom

    br->defs[BLOCK_GRASS] = DefFaces("Grass", true,
        2, 2, 2, 2,   2, 2
    );

    br->defs[BLOCK_DIRT] = DefFaces("Dirt", true,
        1, 1, 1, 1,   1, 1
    );

    br->defs[BLOCK_STONE] = DefFaces("Stone", true,
        0, 0, 0, 0,   0, 0
    );
}

const BlockDef *Blocks_Get(const BlockRegistry *br, BlockId id)
{
    return &br->defs[id];
}
