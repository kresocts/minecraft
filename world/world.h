// world/world.h
#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include "chunk.h"

// cache veličina (prozor u memoriji)
#define WORLD_CHUNKS_X 9
#define WORLD_CHUNKS_Z 9

// visina svijeta (Minecraft-like)
#define WORLD_HEIGHT 256

#if (WORLD_HEIGHT % CHUNK_Y) != 0
#error WORLD_HEIGHT must be divisible by CHUNK_Y (16)
#endif

#define WORLD_SECTIONS_Y (WORLD_HEIGHT / CHUNK_Y)

typedef struct ChunkColumn {
    int cx, cz;                 // koje chunk koordinate ova kolona trenutno predstavlja
    bool valid;                 // je li slot popunjen
    Chunk sections[WORLD_SECTIONS_Y];
} ChunkColumn;

typedef struct World {
    ChunkColumn columns[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
} World;

// init: očisti sve + učita inicijalni “window” oko (0,0)
void World_Init(World *w);

// streaming: osiguraj da su kolone oko (centerCX, centerCZ) učitane
// viewDistChunks mora stati u cache: (2*viewDist+1) <= WORLD_CHUNKS_X/Z
void World_UpdateStreaming(World *w, int centerCX, int centerCZ, int viewDistChunks);

// queries
bool World_InYBounds(int y);
bool World_TryGetColumnIndex(const World *w, int cx, int cz, int *outIx, int *outIz);
bool World_IsColumnLoaded(const World *w, int cx, int cz);
const ChunkColumn *World_GetColumnConst(const World *w, int cx, int cz);
ChunkColumn *World_GetColumn(World *w, int cx, int cz);

// block access (ako kolona nije učitana: Get=BLOCK_AIR, Set=ignore)
BlockId World_GetBlock(const World *w, int x, int y, int z);
void    World_SetBlock(World *w, int x, int y, int z, BlockId id);

#endif // WORLD_H
