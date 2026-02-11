#include "atlas.h"

bool Atlas_Load(Atlas *a, const char *path, int tileSize)
{
    if (!a || tileSize <= 0) return false;

    a->tex = (Texture2D){0};
    a->texW = a->texH = 0;
    a->tileSize = tileSize;
    a->tilesX = a->tilesY = 0;

    Image img = LoadImage(path);
    if (!img.data || img.width <= 0 || img.height <= 0) return false;

    // (preporuka) zahtijevaj da je spritesheet djeljiv s tileSize
    if ((img.width % tileSize) != 0 || (img.height % tileSize) != 0) {
        UnloadImage(img);
        return false;
    }

    a->texW = img.width;
    a->texH = img.height;
    a->tilesX = img.width / tileSize;
    a->tilesY = img.height / tileSize;

    a->tex = LoadTextureFromImage(img);
    UnloadImage(img);

    return a->tex.id != 0;
}

void Atlas_Unload(Atlas *a)
{
    if (!a) return;
    if (a->tex.id != 0) UnloadTexture(a->tex);

    *a = (Atlas){0};
}

bool Atlas_IsLoaded(const Atlas *a)
{
    return a && a->tex.id != 0 && a->tilesX > 0 && a->tilesY > 0;
}

bool Atlas_TileIdToXY(const Atlas *a, int tileId, int *outX, int *outY)
{
    if (!Atlas_IsLoaded(a) || tileId < 0) return false;

    int tx = tileId % a->tilesX;
    int ty = tileId / a->tilesX;

    if (ty < 0 || ty >= a->tilesY) return false;

    if (outX) *outX = tx;
    if (outY) *outY = ty;
    return true;
}

Rectangle Atlas_SourceRect(const Atlas *a, int tileX, int tileY)
{
    if (!Atlas_IsLoaded(a)) return (Rectangle){0};

    if (tileX < 0 || tileX >= a->tilesX) return (Rectangle){0};
    if (tileY < 0 || tileY >= a->tilesY) return (Rectangle){0};

    return (Rectangle){
        (float)(tileX * a->tileSize),
        (float)(tileY * a->tileSize),
        (float)a->tileSize,
        (float)a->tileSize
    };
}

Rectangle Atlas_SourceRectFromTileId(const Atlas *a, int tileId)
{
    int tx, ty;
    if (!Atlas_TileIdToXY(a, tileId, &tx, &ty)) return (Rectangle){0};
    return Atlas_SourceRect(a, tx, ty);
}
