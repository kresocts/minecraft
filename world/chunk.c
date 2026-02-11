#include "chunk.h"

static int Chunk_Index(int lx, int ly, int lz)
{
    return lx + CHUNK_X * (ly + CHUNK_Y * lz);
}

void Chunk_Clear(Chunk *c, BlockId fill)
{
    for (int i = 0; i < CHUNK_VOLUME; i++) {
        c->blocks[i] = fill;
    }
}

BlockId Chunk_GetLocal(const Chunk *c, int lx, int ly, int lz)
{
    if (lx < 0 || lx >= CHUNK_X) return BLOCK_AIR;
    if (ly < 0 || ly >= CHUNK_Y) return BLOCK_AIR;
    if (lz < 0 || lz >= CHUNK_Z) return BLOCK_AIR;
    return c->blocks[Chunk_Index(lx, ly, lz)];
}

void Chunk_SetLocal(Chunk *c, int lx, int ly, int lz, BlockId id)
{
    if (lx < 0 || lx >= CHUNK_X) return;
    if (ly < 0 || ly >= CHUNK_Y) return;
    if (lz < 0 || lz >= CHUNK_Z) return;
    c->blocks[Chunk_Index(lx, ly, lz)] = id;
}
