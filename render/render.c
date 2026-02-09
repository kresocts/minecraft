#include "render.h"
#include <math.h>
#include <stddef.h>
#include "rlgl.h"

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


static Color BlockColor(BlockId id)
{
    switch (id) {
        case BLOCK_DIRT:  return (Color){ 120,  85,  60, 255 };
        case BLOCK_GRASS: return (Color){  70, 170,  70, 255 };
        case BLOCK_STONE: return (Color){ 130, 130, 140, 255 };
        default:          return (Color){   0,   0,   0,   0 };
    }
}

static inline BlockId GetNeighborFast(const World *w, const Chunk *c,
                                      int baseX, int baseZ,
                                      int lx, int y, int lz,
                                      int dx, int dy, int dz)
{
    int nx = lx + dx;
    int ny = y  + dy;
    int nz = lz + dz;

    if (ny < 0 || ny >= CHUNK_Y) return BLOCK_AIR;

    // unutar istog chunka -> brzo
    if ((unsigned)nx < CHUNK_X && (unsigned)nz < CHUNK_Z) {
        return Chunk_GetLocal(c, nx, ny, nz);
    }

    // preko ruba -> world lookup
    return World_GetBlock(w, baseX + nx, ny, baseZ + nz);
}

static bool IsExposedFast(const World *w, const Chunk *c,
                          int baseX, int baseZ,
                          int lx, int y, int lz)
{
    static const int dx[6] = {  1, -1,  0,  0,  0,  0 };
    static const int dy[6] = {  0,  0,  1, -1,  0,  0 };
    static const int dz[6] = {  0,  0,  0,  0,  1, -1 };

    for (int i = 0; i < 6; i++) {
        if (GetNeighborFast(w, c, baseX, baseZ, lx, y, lz, dx[i], dy[i], dz[i]) == BLOCK_AIR) {
            return true;
        }
    }
    return false;
}
static void DrawCubeTextured(Texture2D tex, Vector3 center, float w, float h, float l)
{
    float x0 = center.x - w*0.5f, x1 = center.x + w*0.5f;
    float y0 = center.y - h*0.5f, y1 = center.y + h*0.5f;
    float z0 = center.z - l*0.5f, z1 = center.z + l*0.5f;

    rlSetTexture(tex.id);
    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, 255);

    // Front (+Z)
    rlNormal3f(0, 0, 1);
    rlTexCoord2f(0, 1); rlVertex3f(x0, y0, z1);
    rlTexCoord2f(1, 1); rlVertex3f(x1, y0, z1);
    rlTexCoord2f(1, 0); rlVertex3f(x1, y1, z1);
    rlTexCoord2f(0, 0); rlVertex3f(x0, y1, z1);

    // Back (-Z)
    rlNormal3f(0, 0, -1);
    rlTexCoord2f(0, 1); rlVertex3f(x1, y0, z0);
    rlTexCoord2f(1, 1); rlVertex3f(x0, y0, z0);
    rlTexCoord2f(1, 0); rlVertex3f(x0, y1, z0);
    rlTexCoord2f(0, 0); rlVertex3f(x1, y1, z0);

    // Right (+X)
    rlNormal3f(1, 0, 0);
    rlTexCoord2f(0, 1); rlVertex3f(x1, y0, z1);
    rlTexCoord2f(1, 1); rlVertex3f(x1, y0, z0);
    rlTexCoord2f(1, 0); rlVertex3f(x1, y1, z0);
    rlTexCoord2f(0, 0); rlVertex3f(x1, y1, z1);

    // Left (-X)
    rlNormal3f(-1, 0, 0);
    rlTexCoord2f(0, 1); rlVertex3f(x0, y0, z0);
    rlTexCoord2f(1, 1); rlVertex3f(x0, y0, z1);
    rlTexCoord2f(1, 0); rlVertex3f(x0, y1, z1);
    rlTexCoord2f(0, 0); rlVertex3f(x0, y1, z0);

    // Top (+Y)
    rlNormal3f(0, 1, 0);
    rlTexCoord2f(0, 1); rlVertex3f(x0, y1, z1);
    rlTexCoord2f(1, 1); rlVertex3f(x1, y1, z1);
    rlTexCoord2f(1, 0); rlVertex3f(x1, y1, z0);
    rlTexCoord2f(0, 0); rlVertex3f(x0, y1, z0);

    // Bottom (-Y)
    rlNormal3f(0, -1, 0);
    rlTexCoord2f(0, 1); rlVertex3f(x0, y0, z0);
    rlTexCoord2f(1, 1); rlVertex3f(x1, y0, z0);
    rlTexCoord2f(1, 0); rlVertex3f(x1, y0, z1);
    rlTexCoord2f(0, 0); rlVertex3f(x0, y0, z1);

    rlEnd();
    rlSetTexture(0);
}

static void Render_DrawWorld_Textured(
    const World *w,
    Vector3 camPos,
    int viewDistChunks,
    const Atlas *atlas,
    const BlockRegistry *blocks
)
{
    int camX = (int)floorf(camPos.x);
    int camZ = (int)floorf(camPos.z);

    int centerCX = FloorDivPosInt(camX, CHUNK_X);
    int centerCZ = FloorDivPosInt(camZ, CHUNK_Z);

    int minCX = ClampInt(centerCX - viewDistChunks, WORLD_MIN_CHUNK_X, WORLD_MAX_CHUNK_X - 1);
    int maxCX = ClampInt(centerCX + viewDistChunks, WORLD_MIN_CHUNK_X, WORLD_MAX_CHUNK_X - 1);
    int minCZ = ClampInt(centerCZ - viewDistChunks, WORLD_MIN_CHUNK_Z, WORLD_MAX_CHUNK_Z - 1);
    int maxCZ = ClampInt(centerCZ + viewDistChunks, WORLD_MIN_CHUNK_Z, WORLD_MAX_CHUNK_Z - 1);

    bool hasAtlas = (atlas && atlas->tiles != NULL && blocks);

    for (int cx = minCX; cx <= maxCX; cx++) {
        for (int cz = minCZ; cz <= maxCZ; cz++) {

            int ix = cx - WORLD_MIN_CHUNK_X;
            int iz = cz - WORLD_MIN_CHUNK_Z;

            const Chunk *c = &w->chunks[ix][iz];

            int baseX = cx * CHUNK_X;
            int baseZ = cz * CHUNK_Z;

            for (int lx = 0; lx < CHUNK_X; lx++) {
                for (int y = 0; y < CHUNK_Y; y++) {
                    for (int lz = 0; lz < CHUNK_Z; lz++) {

                        BlockId id = Chunk_GetLocal(c, lx, y, lz);
                        if (id == BLOCK_AIR) continue;

                        // koristi svoj IsExposedFast(...) ovdje:
                        // if (!IsExposedFast(w, c, baseX, baseZ, lx, y, lz)) continue;

                        int x = baseX + lx;
                        int z = baseZ + lz;

                        Vector3 center = (Vector3){ x + 0.5f, y + 0.5f, z + 0.5f };

                        if (hasAtlas) {
                            const BlockDef *def = Blocks_Get(blocks, id);
                            if (def->tileX >= 0) {
                                Texture2D tile = Atlas_GetTile(atlas, def->tileX, def->tileY);
                            if (tile.id != 0) {
                                DrawCubeTextured(tile, center, 1.0f, 1.0f, 1.0f);

                                continue;
                            }

                                continue;
                            }
                        }

                        // fallback ako nema atlasa / tile nije definiran
                        DrawCube(center, 1.0f, 1.0f, 1.0f, (Color){ 120,120,120,255 });
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

void Render_DrawFrame(
    const RenderConfig *rc, 
    Camera3D cam, 
    const World *world, 
    const RenderOverlay *ovr,
    const Atlas *atlas,
    const BlockRegistry *blocks,
    const Hotbar *hotbar
)
{
    BeginDrawing();
    ClearBackground(rc->clearColor);

    BeginMode3D(cam);

    Render_DrawWorld_Textured(world, cam.position, rc->viewDistChunks, atlas, blocks);

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

    // hotbar UI (2D)
    if (hotbar && atlas && blocks) {
        Hotbar_Draw(hotbar, atlas, blocks);
    }
    // Debug (opcionalno): pokaži jel ima hit
    // if (ovr && ovr->hasHit) DrawText("HIT", 20, 50, 20, GREEN);
    // else DrawText("NO HIT", 20, 50, 20, RED);

    EndDrawing();
}
