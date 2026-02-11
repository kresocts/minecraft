// world/world.c
#include "world.h"
#include "world_gen.h"  // na vrh

#include <math.h>
#include <stdint.h>
#include <assert.h>
#include <stddef.h> // za NULL

static int WrapMod(int v, int n)
{
    if (n <= 0) return 0;
    v %= n;
    if (v < 0) v += n;
    return v;
}

bool World_TryGetColumnIndex(const World *w, int cx, int cz, int *outIx, int *outIz)
{
    if (!w || !outIx || !outIz) return false;

    // ring-buffer mapping (mora biti isti kao u streaming/load logici)
    int ix = WrapMod(cx, WORLD_CHUNKS_X);
    int iz = WrapMod(cz, WORLD_CHUNKS_Z);

    const ChunkColumn *col = &w->columns[ix][iz];
    if (!col->valid) return false;
    if (col->cx != cx || col->cz != cz) return false;

    *outIx = ix;
    *outIz = iz;
    return true;
}

// ---------- small math helpers ----------
static int FloorDivPos(int a, int b) // b > 0, radi i za negativne
{
    int q = a / b;
    int r = a % b;
    if (r < 0) q -= 1;
    return q;
}

static int ModPos(int a, int m) // uvijek 0..m-1
{
    int r = a % m;
    if (r < 0) r += m;
    return r;
}

bool World_InYBounds(int y)
{
    return (y >= 0 && y < WORLD_HEIGHT);
}




static void Column_Clear(ChunkColumn *col)
{
    for (int sy = 0; sy < WORLD_SECTIONS_Y; sy++) {
        Chunk_Clear(&col->sections[sy], BLOCK_AIR);
    }
}

// ---------- cache lookup ----------
bool World_IsColumnLoaded(const World *w, int cx, int cz)
{
    if (!w) return false;
    int sx = ModPos(cx, WORLD_CHUNKS_X);
    int sz = ModPos(cz, WORLD_CHUNKS_Z);

    const ChunkColumn *col = &w->columns[sx][sz];
    return col->valid && col->cx == cx && col->cz == cz;
}

const ChunkColumn *World_GetColumnConst(const World *w, int cx, int cz)
{
    if (!World_IsColumnLoaded(w, cx, cz)) return NULL;
    int sx = ModPos(cx, WORLD_CHUNKS_X);
    int sz = ModPos(cz, WORLD_CHUNKS_Z);
    return &w->columns[sx][sz];
}

ChunkColumn *World_GetColumn(World *w, int cx, int cz)
{
    if (!World_IsColumnLoaded(w, cx, cz)) return NULL;
    int sx = ModPos(cx, WORLD_CHUNKS_X);
    int sz = ModPos(cz, WORLD_CHUNKS_Z);
    return &w->columns[sx][sz];
}

static ChunkColumn *World_EnsureColumn(World *w, int cx, int cz)
{
    int sx = ModPos(cx, WORLD_CHUNKS_X);
    int sz = ModPos(cz, WORLD_CHUNKS_Z);

    ChunkColumn *col = &w->columns[sx][sz];

    // ako slot drži nešto drugo -> pregazi i regeneriraj
    if (!(col->valid && col->cx == cx && col->cz == cz)) {
        col->cx = cx;
        col->cz = cz;
        col->valid = true;
        WorldGen_GenerateColumn(&w->gen, cx, cz, col->sections, WORLD_SECTIONS_Y);
    }

    return col;
}

// ---------- streaming ----------
void World_UpdateStreaming(World *w, int centerCX, int centerCZ, int viewDistChunks)
{
    if (!w) return;

    // cache mora pokriti view square
    assert((2 * viewDistChunks + 1) <= WORLD_CHUNKS_X);
    assert((2 * viewDistChunks + 1) <= WORLD_CHUNKS_Z);

    for (int dx = -viewDistChunks; dx <= viewDistChunks; dx++) {
        for (int dz = -viewDistChunks; dz <= viewDistChunks; dz++) {
            int cx = centerCX + dx;
            int cz = centerCZ + dz;
            (void)World_EnsureColumn(w, cx, cz);
        }
    }
}

// ---------- init ----------
void World_Init(World *w)
{
    if (!w) return;
    WorldGen_Init(&w->gen, 0xC0FFEEu, WorldGen_DefaultParams());

    // invalidate sve slotove
    for (int sx = 0; sx < WORLD_CHUNKS_X; sx++) {
        for (int sz = 0; sz < WORLD_CHUNKS_Z; sz++) {
            w->columns[sx][sz].valid = false;
            w->columns[sx][sz].cx = 0;
            w->columns[sx][sz].cz = 0;
            Column_Clear(&w->columns[sx][sz]);
        }
    }

    // inicijalno učitaj cijeli cache oko (0,0)
    int vdX = WORLD_CHUNKS_X / 2;
    int vdZ = WORLD_CHUNKS_Z / 2;
    int vd  = (vdX < vdZ) ? vdX : vdZ;
    World_UpdateStreaming(w, 0, 0, vd);
}

// ---------- block access ----------
static bool World_ToColumnLocal(int x, int y, int z,
                                int *outCX, int *outCZ,
                                int *outLX, int *outLY, int *outLZ,
                                int *outSY)
{
    if (!World_InYBounds(y)) return false;

    int cx = FloorDivPos(x, CHUNK_X);
    int cz = FloorDivPos(z, CHUNK_Z);

    int lx = x - cx * CHUNK_X;   // 0..15 i za negativne radi zbog FloorDivPos
    int lz = z - cz * CHUNK_Z;

    int sy = y / CHUNK_Y;
    int ly = y - sy * CHUNK_Y;

    if (outCX) *outCX = cx;
    if (outCZ) *outCZ = cz;
    if (outLX) *outLX = lx;
    if (outLY) *outLY = ly;
    if (outLZ) *outLZ = lz;
    if (outSY) *outSY = sy;
    return true;
}

BlockId World_GetBlock(const World *w, int x, int y, int z)
{
    int cx, cz, lx, ly, lz, sy;
    if (!World_ToColumnLocal(x, y, z, &cx, &cz, &lx, &ly, &lz, &sy)) return BLOCK_AIR;

    const ChunkColumn *col = World_GetColumnConst(w, cx, cz);
    if (!col) return BLOCK_AIR;

    return Chunk_GetLocal(&col->sections[sy], lx, ly, lz);
}

void World_SetBlock(World *w, int x, int y, int z, BlockId id)
{
    int cx, cz, lx, ly, lz, sy;
    if (!World_ToColumnLocal(x, y, z, &cx, &cz, &lx, &ly, &lz, &sy)) return;

    ChunkColumn *col = World_GetColumn(w, cx, cz);
    if (!col) return; // izvan učitanog windowa -> ignoriraj (za sada)

    Chunk_SetLocal(&col->sections[sy], lx, ly, lz, id);
}
