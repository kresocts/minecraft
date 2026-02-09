#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"
#include <stdbool.h>

// koristi neparan broj da (0,0) bude u srednjem chunku
#define WORLD_CHUNKS_X 9
#define WORLD_CHUNKS_Z 9

// chunk koordinate u svijetu (signed)
#define WORLD_MIN_CHUNK_X (-(WORLD_CHUNKS_X/2))
#define WORLD_MIN_CHUNK_Z (-(WORLD_CHUNKS_Z/2))
#define WORLD_MAX_CHUNK_X (WORLD_MIN_CHUNK_X + WORLD_CHUNKS_X) // exclusive
#define WORLD_MAX_CHUNK_Z (WORLD_MIN_CHUNK_Z + WORLD_CHUNKS_Z) // exclusive

// world granice u blokovima (signed)
#define WORLD_MIN_X (WORLD_MIN_CHUNK_X * CHUNK_X)
#define WORLD_MIN_Z (WORLD_MIN_CHUNK_Z * CHUNK_Z)
#define WORLD_MAX_X (WORLD_MAX_CHUNK_X * CHUNK_X) // exclusive
#define WORLD_MAX_Z (WORLD_MAX_CHUNK_Z * CHUNK_Z) // exclusive

#define WORLD_SIZE_Y (CHUNK_Y)

typedef struct World {
    Chunk chunks[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
} World;

void   World_Init(World *w);

bool   World_InBounds(int x, int y, int z);
BlockId World_GetBlock(const World *w, int x, int y, int z);
void    World_SetBlock(World *w, int x, int y, int z, BlockId id);

#endif // WORLD_H
