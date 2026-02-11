// world/world_gen.c
#include "world_gen.h"

#include <math.h>
#include <stddef.h>

// ------------------ tiny helpers ------------------
static int ClampInt(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float Clamp01(float x)
{
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

static float SmoothStep(float t)
{
    t = Clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}

// hash (x,z,seed) -> uint32
static uint32_t HashU32(uint32_t seed, int x, int z)
{
    // malo “mixanja”, deterministički
    uint32_t h = seed;
    h ^= (uint32_t)x * 374761393u;
    h ^= (uint32_t)z * 668265263u;
    h = (h ^ (h >> 13u)) * 1274126177u;
    h ^= (h >> 16u);
    return h;
}

// hash -> [0,1)
static float Hash01(uint32_t seed, int x, int z)
{
    uint32_t h = HashU32(seed, x, z);
    return (float)(h & 0x00FFFFFFu) * (1.0f / 16777216.0f);
}

// value-noise (bilinear interp) u [0,1]
static float ValueNoise2D(uint32_t seed, float x, float z)
{
    int x0 = (int)floorf(x);
    int z0 = (int)floorf(z);
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    float fx = x - (float)x0;
    float fz = z - (float)z0;

    float sx = SmoothStep(fx);
    float sz = SmoothStep(fz);

    float v00 = Hash01(seed, x0, z0);
    float v10 = Hash01(seed, x1, z0);
    float v01 = Hash01(seed, x0, z1);
    float v11 = Hash01(seed, x1, z1);

    float ix0 = Lerp(v00, v10, sx);
    float ix1 = Lerp(v01, v11, sx);
    return Lerp(ix0, ix1, sz);
}

// fbm u [0,1]
static float Fbm01(const WorldGen *g, float x, float z)
{
    float amp = 1.0f;
    float freq = 1.0f;

    float sum = 0.0f;
    float maxSum = 0.0f;

    for (int o = 0; o < g->p.octaves; o++) {
        sum += ValueNoise2D(g->seed + (uint32_t)(o * 1013), x * freq, z * freq) * amp;
        maxSum += amp;
        amp *= g->p.persistence;
        freq *= g->p.lacunarity;
    }

    if (maxSum <= 0.0f) return 0.0f;
    return sum / maxSum;
}

// postavi blok u sections[] na global y
static void SetBlockInSections(Chunk *sections, int sectionsY, int lx, int y, int lz, BlockId id)
{
    if (!sections) return;
    if (y < 0) return;
    int worldHeight = sectionsY * CHUNK_Y;
    if (y >= worldHeight) return;

    int sy = y / CHUNK_Y;
    int ly = y - sy * CHUNK_Y;

    if (sy < 0 || sy >= sectionsY) return;
    Chunk_SetLocal(&sections[sy], lx, ly, lz, id);
}

WorldGenParams WorldGen_DefaultParams(void)
{
    WorldGenParams p;
    p.baseHeight = 20;
    p.amplitude = 18;
    p.noiseScale = 0.03f;

    p.octaves = 4;
    p.persistence = 0.5f;
    p.lacunarity = 2.0f;

    p.dirtDepth = 2;

    p.debugPillar = true;
    p.debugPillarHeight = 16;
    return p;
}

void WorldGen_Init(WorldGen *g, uint32_t seed, WorldGenParams p)
{
    if (!g) return;
    g->seed = seed;
    g->p = p;

    // basic safety
    if (g->p.octaves < 1) g->p.octaves = 1;
    if (g->p.noiseScale <= 0.0f) g->p.noiseScale = 0.03f;
    if (g->p.persistence <= 0.0f) g->p.persistence = 0.5f;
    if (g->p.lacunarity <= 1.0f) g->p.lacunarity = 2.0f;
    if (g->p.dirtDepth < 1) g->p.dirtDepth = 1;
}

void WorldGen_GenerateColumn(const WorldGen *g, int cx, int cz, Chunk *sections, int sectionsY)
{
    if (!g || !sections || sectionsY <= 0) return;

    // clear
    for (int sy = 0; sy < sectionsY; sy++) {
        Chunk_Clear(&sections[sy], BLOCK_AIR);
    }

    int worldHeight = sectionsY * CHUNK_Y;

    int baseX = cx * CHUNK_X;
    int baseZ = cz * CHUNK_Z;

    for (int lx = 0; lx < CHUNK_X; lx++) {
        for (int lz = 0; lz < CHUNK_Z; lz++) {

            int wx = baseX + lx;
            int wz = baseZ + lz;

            float nx = (float)wx * g->p.noiseScale;
            float nz = (float)wz * g->p.noiseScale;

            // fbm 0..1 -> centriraj u -1..1 za udoline + brda
            float n01 = Fbm01(g, nx, nz);
            float centered = (n01 * 2.0f - 1.0f);

            int h = g->p.baseHeight + (int)floorf(centered * (float)g->p.amplitude);

            // “height” je broj blokova do površine (surface = h-1)
            h = ClampInt(h, 1, worldHeight);
            int topY = h - 1;

            // stone do (topY - dirtDepth - 1)
            int stoneTop = topY - (g->p.dirtDepth + 1);
            for (int y = 0; y <= stoneTop; y++) {
                SetBlockInSections(sections, sectionsY, lx, y, lz, BLOCK_STONE);
            }

            // dirt slojevi
            for (int d = g->p.dirtDepth; d >= 1; d--) {
                int y = topY - d;
                if (y >= 0) SetBlockInSections(sections, sectionsY, lx, y, lz, BLOCK_DIRT);
            }

            // grass na vrhu
            SetBlockInSections(sections, sectionsY, lx, topY, lz, BLOCK_GRASS);
        }
    }

    // debug pillar na (0,0) kao marker
    if (g->p.debugPillar && cx == 0 && cz == 0) {
        int h = g->p.debugPillarHeight;
        if (h > worldHeight) h = worldHeight;

        for (int y = 3; y < h; y++) {
            SetBlockInSections(sections, sectionsY, 0, y, 0, BLOCK_STONE);
            SetBlockInSections(sections, sectionsY, 1, y, 0, BLOCK_STONE);
        }
    }
}
