#include "atlas.h"
#include <stdlib.h>
#include "raylib.h"


static int Atlas_Index(const Atlas *a, int tx, int ty)
{
    return tx + a->tilesX * ty;
}

bool Atlas_Load(Atlas *a, const char *path, int tileSize)
{
    a->tileSize = tileSize;
    a->tilesX = 0;
    a->tilesY = 0;
    a->tiles = NULL;

    Image img = LoadImage(path);
    if (img.data == NULL || img.width <= 0 || img.height <= 0) {
        return false;
    }

    a->tilesX = img.width / tileSize;
    a->tilesY = img.height / tileSize;


    TraceLog(LOG_INFO, "Atlas_Load: %s | img=%dx%d | tileSize=%d | tiles=%dx%d",
         path, img.width, img.height, tileSize, a->tilesX, a->tilesY);

    if (a->tilesX <= 0 || a->tilesY <= 0) {
        UnloadImage(img);
        return false;
    }

    int count = a->tilesX * a->tilesY;
    a->tiles = (Texture2D *)malloc(sizeof(Texture2D) * count);
    if (!a->tiles) {
        UnloadImage(img);
        return false;
    }

    // inicijaliziraj na 0 da Atlas_Unload bude safe i ako nešto pukne
    for (int i = 0; i < count; i++) a->tiles[i] = (Texture2D){0};

    for (int ty = 0; ty < a->tilesY; ty++) {
        for (int tx = 0; tx < a->tilesX; tx++) {
            Rectangle r = {
                (float)(tx * tileSize),
                (float)(ty * tileSize),
                (float)tileSize,
                (float)tileSize
            };

            Image tileImg = ImageFromImage(img, r);
            Texture2D t = LoadTextureFromImage(tileImg);
            UnloadImage(tileImg);

            a->tiles[Atlas_Index(a, tx, ty)] = t;
        }
    }

    UnloadImage(img);
    return true;
}

void Atlas_Unload(Atlas *a)
{
    if (!a) return;

    if (a->tiles) {
        int count = a->tilesX * a->tilesY;
        for (int i = 0; i < count; i++) {
            if (a->tiles[i].id != 0) UnloadTexture(a->tiles[i]);
        }
        free(a->tiles);
    }

    a->tiles = NULL;
    a->tilesX = 0;
    a->tilesY = 0;
    a->tileSize = 0;
}

Texture2D Atlas_GetTile(const Atlas *a, int tileX, int tileY)
{
    if (!a || !a->tiles) return (Texture2D){0};
    if (tileX < 0 || tileX >= a->tilesX) return (Texture2D){0};
    if (tileY < 0 || tileY >= a->tilesY) return (Texture2D){0};
    return a->tiles[Atlas_Index(a, tileX, tileY)];
}
