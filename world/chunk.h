//spremnik blokova
#ifndef CHUNK_H
#define CHUNK_H

#include <stdbool.h>

typedef unsigned char BlockId;

enum {
    BLOCK_AIR   = 0,
    BLOCK_DIRT  = 1,
    BLOCK_GRASS = 2,
    BLOCK_STONE = 3
};

#define CHUNK_X 16
#define CHUNK_Y 16
#define CHUNK_Z 16

typedef struct Chunk {
    BlockId blocks[CHUNK_X * CHUNK_Y * CHUNK_Z];
} Chunk;

void Chunk_Clear(Chunk *c, BlockId fill);
BlockId Chunk_GetLocal(const Chunk *c, int lx, int ly, int lz);
void Chunk_SetLocal(Chunk *c, int lx, int ly, int lz, BlockId id);

#endif // CHUNK_H
