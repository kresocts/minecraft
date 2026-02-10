#include "game.h"

#include <stdlib.h>

#include "raylib.h"

#include "core/input.h"
#include "core/player.h"
#include "world/world.h"
#include "render/render.h"
#include "gameplay/interaction.h"

#include "assets/atlas.h"
#include "core/blocks.h"
#include "ui/hotbar.h"

struct Game {
    Player player;
    InputState input;

    World world;

    InteractionState interact;

    RenderConfig rc;

    Atlas atlas;
    BlockRegistry blocks;
    Hotbar hotbar;
};

Game *Game_Create(void)
{
    Game *g = (Game *)malloc(sizeof(Game));
    if (!g) return NULL;

    InitWindow(1280, 720, "MC-like");
    SetTargetFPS(144);
    DisableCursor();
    if (!Atlas_Load(&g->atlas, "assets/blocks.png", 32)) {
        CloseWindow();
        free(g);
        return NULL;
    }

    Blocks_Init(&g->blocks);
    Hotbar_Init(&g->hotbar);

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
    Atlas_Unload(&g->atlas);

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
    Hotbar_Update(&g->hotbar, &g->input);

    Player_Update(&g->player, &g->input, dt);

    Camera3D cam = Player_GetCamera(&g->player);

    g->interact.placeBlock = Hotbar_SelectedBlock(&g->hotbar);
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

    RenderFrameInput rfi = {
        .cam = cam,
        .world = &g->world,
        .ovr = &ovr,
        .atlas = &g->atlas,
        .blocks = &g->blocks,
        .hotbar = &g->hotbar
    };

    Render_DrawFrame(&g->rc, &rfi);
}
