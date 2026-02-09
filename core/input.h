//Svaki frame pročita tipke i miš i spremi u jednu strukturu.
//kasnije možeš dodati još tipki (space, mouse click…) 
//bez da ih “širiš” po playeru, svijetu i UI-u — svi samo čitaju InputState.
#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include "raylib.h"

typedef struct InputState {
    Vector2 mouseDelta;

    bool w, a, s, d;
    bool shift;
    bool space;
    bool ctrl;
    bool lmbPressed;
    bool place_block;
    int selectSlot;     // 0..8 ili -1
    float wheelMove;    // scroll (hotbar)

} InputState;

void Input_Update(InputState *in);

#endif // INPUT_H
