#include "interaction.h"

void Interaction_Init(InteractionState *s)
{
    s->hasHit = false;

    s->hasPlace = false;
    s->placeX = s->placeY = s->placeZ = 0;

    s->reach = 8.0f;
    s->placeBlock = BLOCK_DIRT;
}



void Interaction_Update(InteractionState *s, World *world, Camera3D cam, const InputState *in)
{
    Vector2 cross = { (float)GetScreenWidth()*0.5f, (float)GetScreenHeight()*0.5f };
    Ray ray = GetMouseRay(cross, cam);

    s->hasHit = World_Raycast(world, ray.position, ray.direction, s->reach, &s->hit);
    
    // izračun "hit + normal" (gdje bi se postavio blok)
        s->hasPlace = false;
        if (s->hasHit) {
            int px = s->hit.x + (int)s->hit.normal.x;
            int py = s->hit.y + (int)s->hit.normal.y;
            int pz = s->hit.z + (int)s->hit.normal.z;

            if (World_InYBounds(py) && World_GetBlock(world, px, py, pz) == BLOCK_AIR) {
                s->hasPlace = true;
                s->placeX = px;
                s->placeY = py;
                s->placeZ = pz;
            }
        }
    // LMB: destroy
    if (in->lmbPressed && s->hasHit) {
        World_SetBlock(world, s->hit.x, s->hit.y, s->hit.z, BLOCK_AIR);
    }
    // E: place na (hit + normal)
    if (in->place_block && s->hasPlace) {
        World_SetBlock(world, s->placeX, s->placeY, s->placeZ, s->placeBlock);
    }
}
