#include "hotbar.h"
#include <stddef.h>


static int Wrap(int v, int n)
{
    if (n <= 0) return 0;
    v %= n;
    if (v < 0) v += n;
    return v;
}

void Hotbar_Init(Hotbar *h)
{
    h->scale = 2.0f;
    h->selected = 0;

    // default slotovi (primjer)
    h->slots[0] = BLOCK_DIRT;
    h->slots[1] = BLOCK_GRASS;
    h->slots[2] = BLOCK_STONE;
    for (int i = 3; i < HOTBAR_SLOTS; i++) h->slots[i] = BLOCK_DIRT;
}

void Hotbar_Update(Hotbar *h, const InputState *in)
{
    if (in->selectSlot >= 0 && in->selectSlot < HOTBAR_SLOTS) {
        h->selected = in->selectSlot;
    }

    if (in->wheelMove > 0.0f) h->selected = Wrap(h->selected - 1, HOTBAR_SLOTS);
    if (in->wheelMove < 0.0f) h->selected = Wrap(h->selected + 1, HOTBAR_SLOTS);
}

BlockId Hotbar_SelectedBlock(const Hotbar *h)
{
    return h->slots[h->selected];
}

void Hotbar_Draw(const Hotbar *h, const Atlas *atlas, const BlockRegistry *blocks)
{
    if (!h) return;

    const int pad = 6;
    const int slotInner = 32;
    const int slotSize = (int)(slotInner * h->scale) + pad*2;

    int totalW = HOTBAR_SLOTS * slotSize;
    int startX = (GetScreenWidth() - totalW) / 2;
    int y = GetScreenHeight() - slotSize - 20;

    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        int x = startX + i * slotSize;

        DrawRectangle(x, y, slotSize, slotSize, (Color){ 0, 0, 0, 150 });

        Color border = (i == h->selected) ? (Color){ 255, 255, 255, 220 } : (Color){ 255, 255, 255, 80 };
        DrawRectangleLines(x, y, slotSize, slotSize, border);

        BlockId id = h->slots[i];
        const BlockDef *def = blocks ? Blocks_Get(blocks, id) : NULL;

        // nacrtaj tile ikonu
        if (atlas && atlas->tiles != NULL && def && def->tileX >= 0){
            Texture2D tile = Atlas_GetTile(atlas, def->tileX, def->tileY);
            if (tile.id != 0) {
                Vector2 pos = (Vector2){ (float)(x + pad), (float)(y + pad) };
                DrawTextureEx(tile, pos, 0.0f, h->scale, WHITE);
            }
        }

        // mali broj slot-a
        DrawText(TextFormat("%d", i+1), x + 4, y + 2, 16, (Color){ 255,255,255,180 });
            // Label iznad hotbara: ime + tile coords selektiranog bloka
        BlockId selId = h->slots[h->selected];
        const BlockDef *selDef = blocks ? Blocks_Get(blocks, selId) : NULL;

        if (selDef) {
            const char *label = TextFormat("%s  (id=%d, tile=%d,%d)",
                                        selDef->name, (int)selId, selDef->tileX, selDef->tileY);
            int tw = MeasureText(label, 20);
            DrawText(label, (GetScreenWidth() - tw) / 2, y - 28, 20, RAYWHITE);
        }

    }
}
