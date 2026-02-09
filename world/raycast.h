#ifndef WORLD_RAYCAST_H
#define WORLD_RAYCAST_H

#include <stdbool.h>
#include "raylib.h"
#include "world.h"   // World + BlockId

typedef struct VoxelHit {
    int x, y, z;        // koordinata pogođenog bloka
    Vector3 normal;     // normal lica kroz koje smo ušli u blok (npr. {1,0,0}, {-1,0,0}...)
    float distance;     // udaljenost od origin-a do hita (world units)
    BlockId block;      // koji blok je pogođen
} VoxelHit;

// Vrati true ako je unutar maxDist nađen prvi ne-AIR blok.
bool World_Raycast(const World *w, Vector3 origin, Vector3 direction, float maxDist, VoxelHit *outHit);

#endif
