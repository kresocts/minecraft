#ifndef RENDER_H
#define RENDER_H

#include "raylib.h"
#include "world/world.h"

#include "assets/atlas.h"
#include "world/blocks.h"
#include "ui/hotbar.h"

typedef struct RenderConfig {
    Color clearColor;
    int gridSlices;
    float gridSpacing;
    bool drawGrid;
    bool drawHud;
    bool drawCrosshair;
    int viewDistChunks; // koliko chunkova u svaku stranu (npr. 4 = 9x9 chunkova)

} RenderConfig;

typedef struct RenderOverlay {
    bool hasHit;
    int hitX, hitY, hitZ;

    bool hasPlace;
    int placeX, placeY, placeZ;
} RenderOverlay;


void Render_Init(RenderConfig *rc);
void Render_DrawFrame(
    const RenderConfig *rc, 
    Camera3D cam, 
    const World *world, 
    const RenderOverlay *ovr,
    const Atlas *atlas,
    const BlockRegistry *blocks,
    const Hotbar *hotbar
);

#endif // RENDER_H
