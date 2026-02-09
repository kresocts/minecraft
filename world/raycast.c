#include "raycast.h"
#include <math.h>
#include <float.h>

static Vector3 Vec3NormalizeSafe(Vector3 v)
{
    float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (len > 0.000001f) return (Vector3){ v.x/len, v.y/len, v.z/len };
    return (Vector3){ 0, 0, 0 };
}

bool World_Raycast(const World *w, Vector3 origin, Vector3 direction, float maxDist, VoxelHit *outHit)
{
    if (maxDist <= 0.0f || outHit == 0) return false;

    Vector3 dir = Vec3NormalizeSafe(direction);
    if (dir.x == 0.0f && dir.y == 0.0f && dir.z == 0.0f) return false;

    int x = (int)floorf(origin.x);
    int y = (int)floorf(origin.y);
    int z = (int)floorf(origin.z);

    BlockId first = World_GetBlock(w, x, y, z);
    if (first != BLOCK_AIR) {
        *outHit = (VoxelHit){ x, y, z, (Vector3){0,0,0}, 0.0f, first };
        return true;
    }

    int stepX = (dir.x > 0.0f) ? 1 : (dir.x < 0.0f) ? -1 : 0;
    int stepY = (dir.y > 0.0f) ? 1 : (dir.y < 0.0f) ? -1 : 0;
    int stepZ = (dir.z > 0.0f) ? 1 : (dir.z < 0.0f) ? -1 : 0;

    float tDeltaX = (stepX != 0) ? fabsf(1.0f / dir.x) : INFINITY;
    float tDeltaY = (stepY != 0) ? fabsf(1.0f / dir.y) : INFINITY;
    float tDeltaZ = (stepZ != 0) ? fabsf(1.0f / dir.z) : INFINITY;

    float nextBoundaryX = (stepX > 0) ? ((float)x + 1.0f) : (float)x;
    float nextBoundaryY = (stepY > 0) ? ((float)y + 1.0f) : (float)y;
    float nextBoundaryZ = (stepZ > 0) ? ((float)z + 1.0f) : (float)z;

    float tMaxX = (stepX != 0) ? (nextBoundaryX - origin.x) / dir.x : INFINITY;
    float tMaxY = (stepY != 0) ? (nextBoundaryY - origin.y) / dir.y : INFINITY;
    float tMaxZ = (stepZ != 0) ? (nextBoundaryZ - origin.z) / dir.z : INFINITY;

    // safety cap (da nikad ne zapne)
    const int maxSteps = 4096;
    float t = 0.0f;
    Vector3 normal = (Vector3){0,0,0};

    for (int i = 0; i < maxSteps; i++)
    {
        // odaberi koju grid ravninu prvo presijecamo
        if (tMaxX <= tMaxY && tMaxX <= tMaxZ) {
            x += stepX;
            t = tMaxX;
            tMaxX += tDeltaX;
            normal = (Vector3){ (float)-stepX, 0.0f, 0.0f };
        } else if (tMaxY <= tMaxX && tMaxY <= tMaxZ) {
            y += stepY;
            t = tMaxY;
            tMaxY += tDeltaY;
            normal = (Vector3){ 0.0f, (float)-stepY, 0.0f };
        } else {
            z += stepZ;
            t = tMaxZ;
            tMaxZ += tDeltaZ;
            normal = (Vector3){ 0.0f, 0.0f, (float)-stepZ };
        }

        if (t > maxDist) return false;

        BlockId b = World_GetBlock(w, x, y, z);
        if (b != BLOCK_AIR) {
            *outHit = (VoxelHit){ x, y, z, normal, t, b };
            return true;
        }
    }

    return false;
}
