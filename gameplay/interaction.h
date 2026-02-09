#ifndef INTERACTION_H
#define INTERACTION_H

#include "raylib.h"
#include "core/input.h"
#include "world/world.h"
#include "world/raycast.h"

typedef struct InteractionState {
    bool hasHit;
    VoxelHit hit; // zadnji raycast hit (korisno za highlight kasnije)
    bool hasPlace;
    int placeX, placeY, placeZ;

    float reach;
    BlockId placeBlock;  // koji blok postavljamo (za sad fixed)

} InteractionState;

void Interaction_Init(InteractionState *s);
void Interaction_Update(InteractionState *s, World *world, Camera3D cam, const InputState *in);

#endif
