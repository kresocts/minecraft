#ifndef RENDER_H
#define RENDER_H

#include "raylib.h"
#include "world/world.h"

// forward declarations (bez include-a)
typedef struct Atlas Atlas;
typedef struct BlockRegistry BlockRegistry;
typedef struct Hotbar Hotbar;


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

typedef struct RenderFrameInput {
    Camera3D cam;
    const World *world;
    const RenderOverlay *ovr;
    const Atlas *atlas;
    const BlockRegistry *blocks;
    const Hotbar *hotbar;
} RenderFrameInput;

void Render_Init(RenderConfig *rc);

void Render_DrawFrame(const RenderConfig *rc, const RenderFrameInput *in);

#endif // RENDER_H
