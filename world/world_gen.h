// world/world_gen.h
#ifndef WORLD_GEN_H
#define WORLD_GEN_H

#include <stdbool.h>
#include <stdint.h>

#include "chunk.h"      // Chunk, CHUNK_X/Y/Z
#include "core/block_id.h"

typedef struct WorldGenParams {
    // osnovna visina terena (otprilike gdje je “sea level” u tvojoj demo verziji)
    int   baseHeight;     // npr 20

    // koliko ide gore/dolje od baseHeight (brda/udoline)
    int   amplitude;      // npr 18

    // “scale” noise-a (manje = veće planine, veće = sitniji detalji)
    float noiseScale;     // npr 0.03f

    // FBM (više slojeva noise-a)
    int   octaves;        // npr 4
    float persistence;    // npr 0.5f
    float lacunarity;     // npr 2.0f

    // slojevi tla
    int dirtDepth;        // npr 2 (2 dirt + grass)

    // debug marker na (0,0)
    bool debugPillar;
    int  debugPillarHeight; // npr 16
} WorldGenParams;

typedef struct WorldGen {
    uint32_t seed;
    WorldGenParams p;
} WorldGen;

WorldGenParams WorldGen_DefaultParams(void);
void WorldGen_Init(WorldGen *g, uint32_t seed, WorldGenParams p);

// Napuni jednu kolonu: (cx,cz) + pointer na sections[] (WORLD_SECTIONS_Y komada)
// sectionsY = broj vertikalnih sekcija (npr WORLD_SECTIONS_Y)
void WorldGen_GenerateColumn(const WorldGen *g, int cx, int cz, Chunk *sections, int sectionsY);

#endif // WORLD_GEN_H
