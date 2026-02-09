#include "game.h"

#include <stdlib.h>

#include "raylib.h"

#include "core/input.h"
#include "core/player.h"
#include "world/world.h"
#include "render/render.h"
#include "gameplay/interaction.h"

struct Game {
    Player player;
    InputState input;

    World world;

    InteractionState interact;

    RenderConfig rc;
};

Game *Game_Create(void)
{
    Game *g = (Game *)malloc(sizeof(Game));
    if (!g) return NULL;

    InitWindow(1280, 720, "MC-like");
    SetTargetFPS(144);
    DisableCursor();

    Player_Init(&g->player);
    g->input = (InputState){0};

    World_Init(&g->world);

    Render_Init(&g->rc);

    Interaction_Init(&g->interact);

    return g;
}

void Game_Destroy(Game *g)
{
    if (!g) return;

    CloseWindow();
    free(g);
}

bool Game_ShouldClose(const Game *g)
{
    (void)g;
    return WindowShouldClose();
}

void Game_Tick(Game *g)
{
    float dt = GetFrameTime();

    Input_Update(&g->input);
    Player_Update(&g->player, &g->input, dt);

    Camera3D cam = Player_GetCamera(&g->player);

    Interaction_Update(&g->interact, &g->world, cam, &g->input);

    RenderOverlay ovr = (RenderOverlay){0};

    ovr.hasHit = g->interact.hasHit;
    if (ovr.hasHit) {
        ovr.hitX = g->interact.hit.x;
        ovr.hitY = g->interact.hit.y;
        ovr.hitZ = g->interact.hit.z;
    }

    ovr.hasPlace = g->interact.hasPlace;
    if (ovr.hasPlace) {
        ovr.placeX = g->interact.placeX;
        ovr.placeY = g->interact.placeY;
        ovr.placeZ = g->interact.placeZ;
    }

    Render_DrawFrame(&g->rc, cam, &g->world, &ovr);
}
