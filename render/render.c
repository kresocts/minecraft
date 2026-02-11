#include "render.h"
#include <math.h>
#include "rlgl.h"

#include "assets/atlas.h"
#include "core/blocks.h"
#include "ui/hotbar.h"
#include "render/sky.h"

// Ako ti World_TryGetColumnIndex nije u world.h (ili IntelliSense kuka),
// ovo garantira da render.c zna potpis.
// (Ako već postoji u headeru, ovo se i dalje slaže dok je potpis isti.)
bool World_TryGetColumnIndex(const World *w, int cx, int cz, int *outIx, int *outIz);

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

static int FloorDivPosInt(int a, int b) // b > 0
{
    int q = a / b;
    int r = a % b;
    if (r < 0) q -= 1;
    return q;
}

// --- neighbor lookup: fast in same column (including vertical section borders),
//     World_GetBlock only when crossing X/Z column boundary ---
static inline BlockId GetNeighborFast(const World *w,
                                      int ix, int iz,
                                      int baseX, int baseZ,
                                      int sy,
                                      int lx, int ly, int lz,
                                      int dx, int dy, int dz)
{
    int nx = lx + dx;
    int nz = lz + dz;

    int y  = sy * CHUNK_Y + ly;   // global y
    int ny = y + dy;              // neighbor global y

    if (ny < 0 || ny >= WORLD_HEIGHT) return BLOCK_AIR;

    // within same column (X/Z local ok) -> direct section access
    if ((unsigned)nx < CHUNK_X && (unsigned)nz < CHUNK_Z) {
        int nsy = ny / CHUNK_Y;
        int nly = ny - nsy * CHUNK_Y;
        const Chunk *sec = &w->columns[ix][iz].sections[nsy];
        return Chunk_GetLocal(sec, nx, nly, nz);
    }

    // crossing column boundary -> world lookup
    return World_GetBlock(w, baseX + nx, ny, baseZ + nz);
}

static bool IsExposedFast(const World *w,
                          int ix, int iz,
                          int baseX, int baseZ,
                          int sy,
                          int lx, int ly, int lz)
{
    static const int dx[6] = {  1, -1,  0,  0,  0,  0 };
    static const int dy[6] = {  0,  0,  1, -1,  0,  0 };
    static const int dz[6] = {  0,  0,  0,  0,  1, -1 };

    for (int i = 0; i < 6; i++) {
        if (GetNeighborFast(w, ix, iz, baseX, baseZ, sy, lx, ly, lz, dx[i], dy[i], dz[i]) == BLOCK_AIR) {
            return true;
        }
    }
    return false;
}

typedef struct FaceUV { float u0, v0, u1, v1; } FaceUV;

static FaceUV UVFromSrcPx(const Atlas *a, Rectangle srcPx)
{
    float u0 = srcPx.x / (float)a->texW;
    float u1 = (srcPx.x + srcPx.width) / (float)a->texW;

    // invert V (srcPx.y is top-down, OpenGL UV is bottom-up)
    float v0 = 1.0f - (srcPx.y + srcPx.height) / (float)a->texH;
    float v1 = 1.0f - (srcPx.y) / (float)a->texH;

    FaceUV uv = { u0, v0, u1, v1 };
    return uv;
}

static void DrawCubeTexturedAtlas6(const Atlas *atlas, const Rectangle srcPx[FACE_COUNT],
                                  Vector3 center, float w, float h, float l)
{
    float x0 = center.x - w*0.5f, x1 = center.x + w*0.5f;
    float y0 = center.y - h*0.5f, y1 = center.y + h*0.5f;
    float z0 = center.z - l*0.5f, z1 = center.z + l*0.5f;

    FaceUV uv[FACE_COUNT];
    for (int i = 0; i < FACE_COUNT; i++) uv[i] = UVFromSrcPx(atlas, srcPx[i]);

    rlSetTexture(atlas->tex.id);
    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, 255);

    // FRONT (+Z)
    {
        FaceUV t = uv[FACE_FRONT];
        rlNormal3f(0, 0, 1);
        rlTexCoord2f(t.u0, t.v1); rlVertex3f(x0, y0, z1);
        rlTexCoord2f(t.u1, t.v1); rlVertex3f(x1, y0, z1);
        rlTexCoord2f(t.u1, t.v0); rlVertex3f(x1, y1, z1);
        rlTexCoord2f(t.u0, t.v0); rlVertex3f(x0, y1, z1);
    }

    // BACK (-Z)
    {
        FaceUV t = uv[FACE_BACK];
        rlNormal3f(0, 0, -1);
        rlTexCoord2f(t.u0, t.v1); rlVertex3f(x1, y0, z0);
        rlTexCoord2f(t.u1, t.v1); rlVertex3f(x0, y0, z0);
        rlTexCoord2f(t.u1, t.v0); rlVertex3f(x0, y1, z0);
        rlTexCoord2f(t.u0, t.v0); rlVertex3f(x1, y1, z0);
    }

    // LEFT (-X)
    {
        FaceUV t = uv[FACE_LEFT];
        rlNormal3f(-1, 0, 0);
        rlTexCoord2f(t.u0, t.v1); rlVertex3f(x0, y0, z0);
        rlTexCoord2f(t.u1, t.v1); rlVertex3f(x0, y0, z1);
        rlTexCoord2f(t.u1, t.v0); rlVertex3f(x0, y1, z1);
        rlTexCoord2f(t.u0, t.v0); rlVertex3f(x0, y1, z0);
    }

    // RIGHT (+X)
    {
        FaceUV t = uv[FACE_RIGHT];
        rlNormal3f(1, 0, 0);
        rlTexCoord2f(t.u0, t.v1); rlVertex3f(x1, y0, z1);
        rlTexCoord2f(t.u1, t.v1); rlVertex3f(x1, y0, z0);
        rlTexCoord2f(t.u1, t.v0); rlVertex3f(x1, y1, z0);
        rlTexCoord2f(t.u0, t.v0); rlVertex3f(x1, y1, z1);
    }

    // TOP (+Y)
    {
        FaceUV t = uv[FACE_TOP];
        rlNormal3f(0, 1, 0);
        rlTexCoord2f(t.u0, t.v1); rlVertex3f(x0, y1, z1);
        rlTexCoord2f(t.u1, t.v1); rlVertex3f(x1, y1, z1);
        rlTexCoord2f(t.u1, t.v0); rlVertex3f(x1, y1, z0);
        rlTexCoord2f(t.u0, t.v0); rlVertex3f(x0, y1, z0);
    }

    // BOTTOM (-Y)
    {
        FaceUV t = uv[FACE_BOTTOM];
        rlNormal3f(0, -1, 0);
        rlTexCoord2f(t.u0, t.v1); rlVertex3f(x0, y0, z0);
        rlTexCoord2f(t.u1, t.v1); rlVertex3f(x1, y0, z0);
        rlTexCoord2f(t.u1, t.v0); rlVertex3f(x1, y0, z1);
        rlTexCoord2f(t.u0, t.v0); rlVertex3f(x0, y0, z1);
    }

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

    int minCX = centerCX - viewDistChunks;
    int maxCX = centerCX + viewDistChunks;
    int minCZ = centerCZ - viewDistChunks;
    int maxCZ = centerCZ + viewDistChunks;

    bool hasAtlas = Atlas_IsLoaded(atlas) && blocks;

    for (int cx = minCX; cx <= maxCX; cx++) {
        for (int cz = minCZ; cz <= maxCZ; cz++) {

            int ix, iz;
            if (!World_TryGetColumnIndex(w, cx, cz, &ix, &iz)) continue;

            int baseX = cx * CHUNK_X;
            int baseZ = cz * CHUNK_Z;

            for (int sy = 0; sy < WORLD_SECTIONS_Y; sy++) {
                const Chunk *sec = &w->columns[ix][iz].sections[sy];
                int baseY = sy * CHUNK_Y;

                for (int lx = 0; lx < CHUNK_X; lx++) {
                    for (int ly = 0; ly < CHUNK_Y; ly++) {
                        for (int lz = 0; lz < CHUNK_Z; lz++) {

                            BlockId id = Chunk_GetLocal(sec, lx, ly, lz);
                            if (id == BLOCK_AIR) continue;

                            if (!IsExposedFast(w, ix, iz, baseX, baseZ, sy, lx, ly, lz)) continue;

                            int x = baseX + lx;
                            int y = baseY + ly;
                            int z = baseZ + lz;

                            Vector3 center = (Vector3){ x + 0.5f, y + 0.5f, z + 0.5f };

                            if (hasAtlas) {
                                const BlockDef *def = Blocks_Get(blocks, id);
                                if (def) {
                                    Rectangle srcFront = Atlas_SourceRectFromTileId(atlas, def->tile[FACE_FRONT]);
                                    if (srcFront.width > 0.0f) {

                                        Rectangle src[FACE_COUNT];
                                        src[FACE_FRONT] = srcFront;

                                        for (int f = 1; f < FACE_COUNT; f++) {
                                            Rectangle r = Atlas_SourceRectFromTileId(atlas, def->tile[f]);
                                            src[f] = (r.width > 0.0f) ? r : srcFront;
                                        }

                                        DrawCubeTexturedAtlas6(atlas, src, center, 1.0f, 1.0f, 1.0f);
                                        continue;
                                    }
                                }
                            }

                            DrawCube(center, 1.0f, 1.0f, 1.0f, (Color){ 120,120,120,255 });
                        }
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

void Render_DrawFrame(const RenderConfig *rc, const RenderFrameInput *in)
{
    if (!rc || !in) return;

    const RenderOverlay *ovr = in->ovr;
    Camera3D cam = in->cam;
    const Sky *sky = in->sky;

    const World *world = in->world;
    const Atlas *atlas = in->atlas;
    const BlockRegistry *blocks = in->blocks;
    const Hotbar *hotbar = in->hotbar;

    BeginDrawing();
    ClearBackground(rc->clearColor);

    BeginMode3D(cam);

    if (sky) Sky_Draw3D(sky, cam);

    Render_DrawWorld_Textured(world, cam.position, rc->viewDistChunks, atlas, blocks);
    Render_DrawOverlay3D(ovr);

    if (rc->drawGrid) DrawGrid(rc->gridSlices, rc->gridSpacing);

    EndMode3D();

    if (rc->drawHud) {
        DrawText("WASD move | SHIFT sprint | Mouse look | LMB break | E place | ESC quit",
                 20, 20, 20, RAYWHITE);
    }

    if (rc->drawCrosshair) DrawCrosshair();

    if (hotbar && blocks && Atlas_IsLoaded(atlas)) {
        Hotbar_Draw(hotbar, atlas, blocks);
    }

    EndDrawing();
}
