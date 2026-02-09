#ifndef ATLAS_H
#define ATLAS_H

#include <stdbool.h>
#include "raylib.h"

typedef struct Atlas {
    int tileSize;   // 32
    int tilesX;     // sheetWidth / tileSize
    int tilesY;     // sheetHeight / tileSize

    Texture2D *tiles;   // tilesX*tilesY (svaki tile je zasebna tekstura)
} Atlas;

bool Atlas_Load(Atlas *a, const char *path, int tileSize);
void Atlas_Unload(Atlas *a);

// vrati tile teksturu; ako je out-of-range vrati (Texture2D){0}
Texture2D Atlas_GetTile(const Atlas *a, int tileX, int tileY);

#endif // ATLAS_H
