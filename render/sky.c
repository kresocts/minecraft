#include "sky.h"
#include <math.h>
#include <stdint.h>
#include "rlgl.h"

// ----------------- tiny vec helpers -----------------
static Vector3 V3Sub(Vector3 a, Vector3 b) { return (Vector3){ a.x-b.x, a.y-b.y, a.z-b.z }; }
static Vector3 V3Add(Vector3 a, Vector3 b) { return (Vector3){ a.x+b.x, a.y+b.y, a.z+b.z }; }
static Vector3 V3Scale(Vector3 v, float s) { return (Vector3){ v.x*s, v.y*s, v.z*s }; }

static Vector3 V3Cross(Vector3 a, Vector3 b)
{
    return (Vector3){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

static float V3Len(Vector3 v) { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z); }

static Vector3 V3Norm(Vector3 v)
{
    float l = V3Len(v);
    if (l > 0.00001f) return V3Scale(v, 1.0f/l);
    return (Vector3){ 0, 0, 0 };
}

// ----------------- small helpers -----------------
static float Clamp01(float x) { return (x < 0.0f) ? 0.0f : (x > 1.0f) ? 1.0f : x; }

static Color Sky_ColorLerp(Color a, Color b, float t)
{
    t = Clamp01(t);
    Color c;
    c.r = (unsigned char)(a.r + (int)((b.r - a.r) * t));
    c.g = (unsigned char)(a.g + (int)((b.g - a.g) * t));
    c.b = (unsigned char)(a.b + (int)((b.b - a.b) * t));
    c.a = (unsigned char)(a.a + (int)((b.a - a.a) * t));
    return c;
}

static uint32_t LcgNext(uint32_t *state)
{
    *state = (*state * 1664525u) + 1013904223u;
    return *state;
}

static float Rand01(uint32_t *state)
{
    return (float)((LcgNext(state) >> 8) & 0x00FFFFFFu) * (1.0f / 16777216.0f);
}

static Vector3 RandomDirUpperHemisphere(uint32_t *seed)
{
    float y = Rand01(seed) * 2.0f - 1.0f;
    if (y < 0.0f) y = -y;

    float a = Rand01(seed) * 2.0f * PI;
    float r = sqrtf(fmaxf(0.0f, 1.0f - y*y));

    return (Vector3){ cosf(a)*r, y, sinf(a)*r };
}

static Texture2D MakeDiscTexture(int size)
{
    Image img = GenImageColor(size, size, BLANK);
    ImageDrawCircle(&img, size/2, size/2, (size/2) - 1, WHITE);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

// ----------------- public queries -----------------
float Sky_Time01(const Sky *s)
{
    if (!s || s->dayLengthSec <= 0.0f) return 0.0f;
    float t = s->timeSec / s->dayLengthSec;
    t = t - floorf(t);
    return t;
}

Vector3 Sky_SunDir(const Sky *s)
{
    float t = Sky_Time01(s);
    float angle = 2.0f * PI * t - (PI * 0.5f); // noon at t=0.5

    // noon => (0,1,0)
    return (Vector3){ cosf(angle), sinf(angle), 0.0f };
}

Vector3 Sky_MoonDir(const Sky *s)
{
    Vector3 sun = Sky_SunDir(s);
    return (Vector3){ -sun.x, -sun.y, -sun.z };
}

// ----------------- gradient skybox draw -----------------
static void DrawGradientSkyCube(Vector3 center, float size, Color top, Color bottom)
{
    float h = size * 0.5f;

    // Poželjno: flush prije/poslije kad miješaš rlgl i raylib draw
    rlDrawRenderBatchActive();

    rlSetTexture(0);
    rlDisableDepthTest();
    rlDisableDepthMask();
    // culling može ostati i upaljen, ali za svaki slučaj ga ugasi:
    rlDisableBackfaceCulling();

    rlPushMatrix();
    rlTranslatef(center.x, center.y, center.z);

    rlBegin(RL_QUADS);

    // SVE FACEOVE crtamo "iznutra" (invertan redoslijed vrhova)
    // +Z (front)
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a); rlVertex3f(-h, -h,  h);
    rlColor4ub(top.r,    top.g,    top.b,    top.a);    rlVertex3f(-h,  h,  h);
    rlColor4ub(top.r,    top.g,    top.b,    top.a);    rlVertex3f( h,  h,  h);
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a); rlVertex3f( h, -h,  h);

    // -Z (back)
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a); rlVertex3f( h, -h, -h);
    rlColor4ub(top.r,    top.g,    top.b,    top.a);    rlVertex3f( h,  h, -h);
    rlColor4ub(top.r,    top.g,    top.b,    top.a);    rlVertex3f(-h,  h, -h);
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a); rlVertex3f(-h, -h, -h);

    // -X (left)
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a); rlVertex3f(-h, -h, -h);
    rlColor4ub(top.r,    top.g,    top.b,    top.a);    rlVertex3f(-h,  h, -h);
    rlColor4ub(top.r,    top.g,    top.b,    top.a);    rlVertex3f(-h,  h,  h);
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a); rlVertex3f(-h, -h,  h);

    // +X (right)
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a); rlVertex3f( h, -h,  h);
    rlColor4ub(top.r,    top.g,    top.b,    top.a);    rlVertex3f( h,  h,  h);
    rlColor4ub(top.r,    top.g,    top.b,    top.a);    rlVertex3f( h,  h, -h);
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a); rlVertex3f( h, -h, -h);

    // +Y (top)
    rlColor4ub(top.r, top.g, top.b, top.a); rlVertex3f(-h,  h,  h);
    rlColor4ub(top.r, top.g, top.b, top.a); rlVertex3f(-h,  h, -h);
    rlColor4ub(top.r, top.g, top.b, top.a); rlVertex3f( h,  h, -h);
    rlColor4ub(top.r, top.g, top.b, top.a); rlVertex3f( h,  h,  h);

    // -Y (bottom)
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a); rlVertex3f(-h, -h, -h);
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a); rlVertex3f(-h, -h,  h);
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a); rlVertex3f( h, -h,  h);
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a); rlVertex3f( h, -h, -h);

    rlEnd();
    rlPopMatrix();

    rlEnableBackfaceCulling();
    rlEnableDepthMask();
    rlEnableDepthTest();

    rlDrawRenderBatchActive();
}

static void DrawStars(const Sky *s, Camera3D cam, unsigned char alpha)
{
    if (!s || !s->drawStars || s->starsCount <= 0 || alpha == 0) return;

    float radius = s->skySize * 0.45f;
    float half   = s->starSize * 0.5f;

    // build camera-facing basis
    Vector3 forward = V3Norm(V3Sub(cam.target, cam.position));
    Vector3 right   = V3Norm(V3Cross(forward, cam.up));
    Vector3 up      = V3Cross(right, forward);

    Vector3 rx = V3Scale(right, half);
    Vector3 uy = V3Scale(up,    half);
    rlSetTexture(0);

    rlDisableDepthTest();
    rlDisableDepthMask();
    rlDisableBackfaceCulling();

    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, alpha);

    for (int i = 0; i < s->starsCount; i++) {
        Vector3 d = s->stars[i];
        Vector3 p = (Vector3){
            cam.position.x + d.x * radius,
            cam.position.y + d.y * radius,
            cam.position.z + d.z * radius
        };

        Vector3 p0 = V3Sub(V3Sub(p, rx), uy);
        Vector3 p1 = V3Add(V3Sub(p, uy), rx);
        Vector3 p2 = V3Add(V3Add(p, rx), uy);
        Vector3 p3 = V3Sub(V3Add(p, uy), rx);

        rlVertex3f(p0.x, p0.y, p0.z);
        rlVertex3f(p1.x, p1.y, p1.z);
        rlVertex3f(p2.x, p2.y, p2.z);
        rlVertex3f(p3.x, p3.y, p3.z);
    }

    rlEnd();

    rlEnableBackfaceCulling();
    rlEnableDepthMask();
    rlEnableDepthTest();
}

static void DrawSunMoon(const Sky *s, Camera3D cam, float dayFactor)
{
    if (!s || !s->drawSunMoon) return;

    float radius = s->skySize * 0.45f;

    Vector3 sunDir  = Sky_SunDir(s);
    Vector3 moonDir = Sky_MoonDir(s);

    float sunVis  = Clamp01(sunDir.y  * 2.0f);
    float moonVis = Clamp01(moonDir.y * 2.0f);

    if (s->sunTex.id != 0 && sunVis > 0.0f) {
        Vector3 pos = (Vector3){
            cam.position.x + sunDir.x * radius,
            cam.position.y + sunDir.y * radius,
            cam.position.z + sunDir.z * radius
        };
        Color c = Fade(s->sunColor, 0.9f * sunVis);
        DrawBillboard(cam, s->sunTex, pos, s->sunSize, c);
    }

    if (s->moonTex.id != 0 && moonVis > 0.0f) {
        Vector3 pos = (Vector3){
            cam.position.x + moonDir.x * radius,
            cam.position.y + moonDir.y * radius,
            cam.position.z + moonDir.z * radius
        };
        float k = (1.0f - dayFactor);
        Color c = Fade(s->moonColor, 0.85f * moonVis * (0.4f + 0.6f*k));
        DrawBillboard(cam, s->moonTex, pos, s->moonSize, c);
    }
}

// ----------------- public API -----------------
void Sky_Init(Sky *s, float dayLengthSec)
{
    if (!s) return;
    *s = (Sky){0};

    s->dayLengthSec = (dayLengthSec > 0.0f) ? dayLengthSec : 60.0f;
    s->timeSec = s->dayLengthSec * 0.5f; // start at noon

    s->dayZenith   = (Color){  64, 144, 255, 255 };
    s->dayHorizon  = (Color){ 170, 220, 255, 255 };
    s->nightZenith = (Color){   6,  10,  24, 255 };
    s->nightHorizon= (Color){  18,  24,  48, 255 };

    s->skySize = 600.0f;

    s->drawStars = true;
    s->starsCount = 350;
    if (s->starsCount > SKY_MAX_STARS) s->starsCount = SKY_MAX_STARS;
    s->starSize = 1.6f; // tweak

    uint32_t seed = 0xC0FFEEu;
    for (int i = 0; i < s->starsCount; i++) {
        s->stars[i] = RandomDirUpperHemisphere(&seed);
    }

    s->drawSunMoon = true;
    s->sunColor  = (Color){ 255, 245, 210, 255 };
    s->moonColor = (Color){ 210, 220, 255, 255 };
    s->sunSize  = 35.0f;
    s->moonSize = 28.0f;

    s->sunTex  = MakeDiscTexture(64);
    s->moonTex = MakeDiscTexture(64);
}

void Sky_Unload(Sky *s)
{
    if (!s) return;

    if (s->sunTex.id != 0)  UnloadTexture(s->sunTex);
    if (s->moonTex.id != 0) UnloadTexture(s->moonTex);

    *s = (Sky){0};
}

void Sky_Update(Sky *s, float dt)
{
    if (!s || s->dayLengthSec <= 0.0f) return;

    s->timeSec += dt;
    if (s->timeSec >= s->dayLengthSec) {
        s->timeSec = fmodf(s->timeSec, s->dayLengthSec);
    }
}

void Sky_Draw3D(const Sky *s, Camera3D cam)
{
    if (!s) return;

    Vector3 sunDir = Sky_SunDir(s);
    float dayFactor = Clamp01(sunDir.y * 0.5f + 0.5f);

    Color zenith  = Sky_ColorLerp(s->nightZenith,  s->dayZenith,  dayFactor);
    Color horizon = Sky_ColorLerp(s->nightHorizon, s->dayHorizon, dayFactor);

    DrawGradientSkyCube(cam.position, s->skySize, zenith, horizon);

    if (s->drawStars) {
        float k = 1.0f - dayFactor;
        unsigned char a = (unsigned char)(255.0f * (k*k));
        DrawStars(s, cam, a);
    }

    DrawSunMoon(s, cam, dayFactor);
}
