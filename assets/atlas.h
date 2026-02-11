#ifndef ATLAS_H
#define ATLAS_H

#include <stdbool.h>
#include "raylib.h"

typedef struct Atlas {
    Texture2D tex;      // cijeli spritesheet kao jedna textura
    int texW, texH;      // dimenzije u pixelima
    int tileSize;        // npr. 32
    int tilesX, tilesY;  // koliko tileova po osi
} Atlas;

bool Atlas_Load(Atlas *a, const char *path, int tileSize);
void Atlas_Unload(Atlas *a);

bool Atlas_IsLoaded(const Atlas *a);
bool Atlas_TileIdToXY(const Atlas *a, int tileId, int *outX, int *outY);

// source rect u pixelima (za DrawTexturePro / uv računanje)
Rectangle Atlas_SourceRect(const Atlas *a, int tileX, int tileY);

// source rect direktno iz tileId (linearni index u 32x32 gridu)
Rectangle Atlas_SourceRectFromTileId(const Atlas *a, int tileId);
#endif
