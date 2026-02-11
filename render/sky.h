#ifndef SKY_H
#define SKY_H

#include <stdbool.h>
#include "raylib.h"

#define SKY_MAX_STARS 512

typedef struct Sky {
    float dayLengthSec;
    float timeSec;

    Color dayZenith;
    Color dayHorizon;
    Color nightZenith;
    Color nightHorizon;

    bool  drawSunMoon;
    Color sunColor;
    Color moonColor;
    float sunSize;
    float moonSize;
    Texture2D sunTex;
    Texture2D moonTex;

    bool  drawStars;
    int   starsCount;
    float starSize;                 // world-units (npr. 1.0 .. 2.0)
    Vector3 stars[SKY_MAX_STARS];    // unit dir vectors (y>=0)

    float skySize;                  // npr. 600
} Sky;

void   Sky_Init(Sky *s, float dayLengthSec);
void   Sky_Unload(Sky *s);
void   Sky_Update(Sky *s, float dt);

float   Sky_Time01(const Sky *s);
Vector3 Sky_SunDir(const Sky *s);
Vector3 Sky_MoonDir(const Sky *s);

void   Sky_Draw3D(const Sky *s, Camera3D cam);

#endif
