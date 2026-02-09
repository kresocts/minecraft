#include "render.h"
#include <math.h>

// ---------- helpers ----------
static void DrawCrosshair(void)
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    int cx = w / 2;
    int cy = h / 2;

    DrawLine(cx - 6, cy, cx + 6, cy, RAYWHITE);
    DrawLine(cx, cy - 6, cx, cy + 6, RAYWHITE);
}

static Color BlockColor(BlockId id)
{
    switch (id) {
        case BLOCK_DIRT:  return (Color){ 120,  85,  60, 255 };
        case BLOCK_GRASS: return (Color){  70, 170,  70, 255 };
        case BLOCK_STONE: return (Color){ 130, 130, 140, 255 };
        default:          return (Color){   0,   0,   0,   0 };
    }
}

static bool IsExposed(const World *w, int x, int y, int z)
{
    static const int dx[6] = {  1, -1,  0,  0,  0,  0 };
    static const int dy[6] = {  0,  0,  1, -1,  0,  0 };
    static const int dz[6] = {  0,  0,  0,  0,  1, -1 };

    for (int i = 0; i < 6; i++) {
        if (World_GetBlock(w, x + dx[i], y + dy[i], z + dz[i]) == BLOCK_AIR) {
            return true;
        }
    }
    return false;
}

static int ClampInt(int v, int mn, int mx)
{
    if (v < mn) return mn;
    if (v > mx) return mx;
    return v;
}

static int FloorDivPosInt(int a, int b) // b > 0
{
    int q = a / b;
    int r = a % b;
    if (r < 0) q -= 1;
    return q;
}

static void Render_DrawWorld(const World *w, Vector3 camPos, int viewDistChunks)
{
    int camX = (int)floorf(camPos.x);
    int camZ = (int)floorf(camPos.z);

    // signed chunk coords (mogu biti negativni)
    int centerCX = FloorDivPosInt(camX, CHUNK_X);
    int centerCZ = FloorDivPosInt(camZ, CHUNK_Z);

    int minCX = ClampInt(centerCX - viewDistChunks, WORLD_MIN_CHUNK_X, WORLD_MAX_CHUNK_X - 1);
    int maxCX = ClampInt(centerCX + viewDistChunks, WORLD_MIN_CHUNK_X, WORLD_MAX_CHUNK_X - 1);
    int minCZ = ClampInt(centerCZ - viewDistChunks, WORLD_MIN_CHUNK_Z, WORLD_MAX_CHUNK_Z - 1);
    int maxCZ = ClampInt(centerCZ + viewDistChunks, WORLD_MIN_CHUNK_Z, WORLD_MAX_CHUNK_Z - 1);

    for (int cx = minCX; cx <= maxCX; cx++) {
        for (int cz = minCZ; cz <= maxCZ; cz++) {

            // pretvori signed chunk coord -> array index
            int ix = cx - WORLD_MIN_CHUNK_X;
            int iz = cz - WORLD_MIN_CHUNK_Z;

            const Chunk *c = &w->chunks[ix][iz];

            int baseX = cx * CHUNK_X;   // OVO je sad ispravno i može biti negativno
            int baseZ = cz * CHUNK_Z;

            for (int lx = 0; lx < CHUNK_X; lx++) {
                for (int y = 0; y < CHUNK_Y; y++) {
                    for (int lz = 0; lz < CHUNK_Z; lz++) {

                        BlockId id = Chunk_GetLocal(c, lx, y, lz);
                        if (id == BLOCK_AIR) continue;

                        int x = baseX + lx;
                        int z = baseZ + lz;

                        if (!IsExposed(w, x, y, z)) continue;

                        Vector3 center = (Vector3){ x + 0.5f, y + 0.5f, z + 0.5f };
                        Color col = BlockColor(id);

                        DrawCube(center, 1.0f, 1.0f, 1.0f, col);
                        DrawCubeWires(center, 1.0f, 1.0f, 1.0f, (Color){ 0, 0, 0, 80 });
                    }
                }
            }
        }
    }
}

static void Render_DrawOverlay3D(const RenderOverlay *ovr)
{
    if (!ovr) return;

    if (ovr->hasHit) {
        Vector3 c = (Vector3){ ovr->hitX + 0.5f, ovr->hitY + 0.5f, ovr->hitZ + 0.5f };
        DrawCubeWires(c, 1.05f, 1.05f, 1.05f, YELLOW);
    }

    if (ovr->hasPlace) {
        Vector3 c = (Vector3){ ovr->placeX + 0.5f, ovr->placeY + 0.5f, ovr->placeZ + 0.5f };
        DrawCubeWires(c, 1.05f, 1.05f, 1.05f, GREEN);
    }
}


// ---------- public API ----------
void Render_Init(RenderConfig *rc)
{
    rc->clearColor = (Color){ 20, 24, 28, 255 };
    rc->gridSlices = 20;
    rc->gridSpacing = 1.0f;
    rc->drawGrid = false;
    rc->drawHud = true;
    rc->drawCrosshair = true;
    rc->viewDistChunks = 4;

}

void Render_DrawFrame(const RenderConfig *rc, Camera3D cam, const World *world, const RenderOverlay *ovr)
{
    BeginDrawing();
    ClearBackground(rc->clearColor);

    BeginMode3D(cam);

    Render_DrawWorld(world, cam.position, rc->viewDistChunks);

    // BITNO: overlay mora biti unutar BeginMode3D/EndMode3D
    Render_DrawOverlay3D(ovr);

    if (rc->drawGrid) {
        DrawGrid(rc->gridSlices, rc->gridSpacing);
    }

    EndMode3D();

    if (rc->drawHud) {
        DrawText("WASD move | SHIFT sprint | Mouse look | LMB break | RMB place (ili E) | ESC quit", 20, 20, 20, RAYWHITE);
    }

    if (rc->drawCrosshair) {
        DrawCrosshair();
    }

    // Debug (opcionalno): pokaži jel ima hit
    // if (ovr && ovr->hasHit) DrawText("HIT", 20, 50, 20, GREEN);
    // else DrawText("NO HIT", 20, 50, 20, RED);

    EndDrawing();
}
