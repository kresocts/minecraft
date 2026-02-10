#ifndef HOTBAR_H
#define HOTBAR_H

#include "raylib.h"
#include "core/input.h"
#include "assets/atlas.h"
#include "core/blocks.h"

#define HOTBAR_SLOTS 9

typedef struct Hotbar {
    BlockId slots[HOTBAR_SLOTS];
    int selected;     // 0..8
    float scale;      // npr 2.0f (ikonice)
} Hotbar;

void Hotbar_Init(Hotbar *h);
void Hotbar_Update(Hotbar *h, const InputState *in);
BlockId Hotbar_SelectedBlock(const Hotbar *h);

void Hotbar_Draw(const Hotbar *h, const Atlas *atlas, const BlockRegistry *blocks);

#endif // HOTBAR_H
