// world/world.c
#include "world.h"

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

// ---------- very simple placeholder generator (kasnije ide u world_gen.*) ----------
static float Hash01(int x, int z)
{
    // deterministic hash -> 0..1
    uint32_t h = (uint32_t)(x) * 374761393u + (uint32_t)(z) * 668265263u;
    h = (h ^ (h >> 13u)) * 1274126177u;
    h ^= (h >> 16u);
    return (float)(h & 0x00FFFFFFu) * (1.0f / 16777216.0f);
}

static int HeightAt(int x, int z)
{
    // baseline da ti spawn i postojeći gameplay ne “padnu” (grass oko y≈2..8)
    float n = Hash01(x, z);          // 0..1
    int base = 3;                    // grass na y=2 kad je height=3
    int amp  = 7;                    // do ~10
    return base + (int)floorf(n * (float)amp);
}

static void Column_Clear(ChunkColumn *col)
{
    for (int sy = 0; sy < WORLD_SECTIONS_Y; sy++) {
        Chunk_Clear(&col->sections[sy], BLOCK_AIR);
    }
}

static void Column_SetLocal(ChunkColumn *col, int lx, int y, int lz, BlockId id)
{
    if (!World_InYBounds(y)) return;
    int sy = y / CHUNK_Y;
    int ly = y - sy * CHUNK_Y;
    Chunk_SetLocal(&col->sections[sy], lx, ly, lz, id);
}

static void World_GenerateColumn(ChunkColumn *col)
{
    Column_Clear(col);

    int baseX = col->cx * CHUNK_X;
    int baseZ = col->cz * CHUNK_Z;

    for (int lx = 0; lx < CHUNK_X; lx++) {
        for (int lz = 0; lz < CHUNK_Z; lz++) {

            int wx = baseX + lx;
            int wz = baseZ + lz;

            int h = HeightAt(wx, wz);               // “surface” = h-1
            if (h < 1) h = 1;
            if (h > WORLD_HEIGHT) h = WORLD_HEIGHT;

            int topY = h - 1;

            // stone duboko
            for (int y = 0; y <= topY - 3; y++) {
                Column_SetLocal(col, lx, y, lz, BLOCK_STONE);
            }
            // 2 sloja dirt
            if (topY - 2 >= 0) Column_SetLocal(col, lx, topY - 2, lz, BLOCK_DIRT);
            if (topY - 1 >= 0) Column_SetLocal(col, lx, topY - 1, lz, BLOCK_DIRT);

            // grass na vrhu
            Column_SetLocal(col, lx, topY, lz, BLOCK_GRASS);
        }
    }

    // mali debug “pillar” na (0,0) kao prije (da znaš gdje si)
    if (col->cx == 0 && col->cz == 0) {
        for (int y = 3; y < 16 && y < WORLD_HEIGHT; y++) {
            Column_SetLocal(col, 0, y, 0, BLOCK_STONE);
            Column_SetLocal(col, 1, y, 0, BLOCK_STONE);
        }
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
        World_GenerateColumn(col);
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
