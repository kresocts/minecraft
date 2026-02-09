#include "world.h"

static int FloorDivPos(int a, int b) // b > 0
{
    int q = a / b;
    int r = a % b;
    if (r < 0) q -= 1; // bitno za negativne
    return q;
}

static bool World_ToChunkLocal(int x, int z, int *ix, int *iz, int *lx, int *lz)
{
    // chunk koordinate (signed)
    int cx = FloorDivPos(x, CHUNK_X);
    int cz = FloorDivPos(z, CHUNK_Z);

    // local unutar chunka 0..15 (radi i za negativno)
    int localX = x - cx * CHUNK_X;
    int localZ = z - cz * CHUNK_Z;

    // pretvori signed chunk coord u array index
    int arrX = cx - WORLD_MIN_CHUNK_X;
    int arrZ = cz - WORLD_MIN_CHUNK_Z;

    if (arrX < 0 || arrX >= WORLD_CHUNKS_X) return false;
    if (arrZ < 0 || arrZ >= WORLD_CHUNKS_Z) return false;

    *ix = arrX; *iz = arrZ;
    *lx = localX; *lz = localZ;
    return true;
}

bool World_InBounds(int x, int y, int z)
{
    return (x >= WORLD_MIN_X && x < WORLD_MAX_X) &&
           (y >= 0 && y < WORLD_SIZE_Y) &&
           (z >= WORLD_MIN_Z && z < WORLD_MAX_Z);
}

void World_Init(World *w)
{
    // clear svih chunkova
    for (int ix = 0; ix < WORLD_CHUNKS_X; ix++) {
        for (int iz = 0; iz < WORLD_CHUNKS_Z; iz++) {
            Chunk_Clear(&w->chunks[ix][iz], BLOCK_AIR);
        }
    }

    // ravni teren po svim chunkovima: 2 dirt + 1 grass
    for (int ix = 0; ix < WORLD_CHUNKS_X; ix++) {
        for (int iz = 0; iz < WORLD_CHUNKS_Z; iz++) {
            Chunk *c = &w->chunks[ix][iz];

            for (int lx = 0; lx < CHUNK_X; lx++) {
                for (int lz = 0; lz < CHUNK_Z; lz++) {
                    Chunk_SetLocal(c, lx, 0, lz, BLOCK_DIRT);
                    Chunk_SetLocal(c, lx, 1, lz, BLOCK_DIRT);
                    Chunk_SetLocal(c, lx, 2, lz, BLOCK_GRASS);
                }
            }
        }
    }

    // opcionalno: malo “stijene” blizu (0,0)
    for (int y = 3; y < 8; y++) {
        World_SetBlock(w, 0, y, 0, BLOCK_STONE);
        World_SetBlock(w, 1, y, 0, BLOCK_STONE);
    }
}

BlockId World_GetBlock(const World *w, int x, int y, int z)
{
    if (!World_InBounds(x, y, z)) return BLOCK_AIR;

    int ix, iz, lx, lz;
    if (!World_ToChunkLocal(x, z, &ix, &iz, &lx, &lz)) return BLOCK_AIR;

    return Chunk_GetLocal(&w->chunks[ix][iz], lx, y, lz);
}

void World_SetBlock(World *w, int x, int y, int z, BlockId id)
{
    if (!World_InBounds(x, y, z)) return;

    int ix, iz, lx, lz;
    if (!World_ToChunkLocal(x, z, &ix, &iz, &lx, &lz)) return;

    Chunk_SetLocal(&w->chunks[ix][iz], lx, y, lz, id);
}
