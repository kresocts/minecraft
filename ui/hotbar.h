#ifndef HOTBAR_H
#define HOTBAR_H

#include "core/input.h"
#include "core/block_id.h"

#define HOTBAR_SLOTS 9

typedef struct Atlas Atlas;
typedef struct BlockRegistry BlockRegistry;

typedef struct Hotbar {
    BlockId slots[HOTBAR_SLOTS];
    int selected;
    float scale;
} Hotbar;

void Hotbar_Init(Hotbar *h);
void Hotbar_Update(Hotbar *h, const InputState *in);
BlockId Hotbar_SelectedBlock(const Hotbar *h);
void Hotbar_Draw(const Hotbar *h, const Atlas *atlas, const BlockRegistry *blocks);

#endif
