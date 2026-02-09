#include "input.h"

void Input_Update(InputState *in)
{
    in->mouseDelta = GetMouseDelta();

    in->w = IsKeyDown(KEY_W);
    in->a = IsKeyDown(KEY_A);
    in->s = IsKeyDown(KEY_S);
    in->d = IsKeyDown(KEY_D);
    

    in->shift = IsKeyDown(KEY_LEFT_SHIFT);

    in->space = IsKeyDown(KEY_SPACE);
    in->ctrl = IsKeyDown(KEY_LEFT_CONTROL);

    in->lmbPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    in->place_block = IsKeyPressed(KEY_E);

    in->wheelMove = GetMouseWheelMove();

    in->selectSlot = -1;
    if (IsKeyPressed(KEY_ONE))   in->selectSlot = 0;
    if (IsKeyPressed(KEY_TWO))   in->selectSlot = 1;
    if (IsKeyPressed(KEY_THREE)) in->selectSlot = 2;
    if (IsKeyPressed(KEY_FOUR))  in->selectSlot = 3;
    if (IsKeyPressed(KEY_FIVE))  in->selectSlot = 4;
    if (IsKeyPressed(KEY_SIX))   in->selectSlot = 5;
    if (IsKeyPressed(KEY_SEVEN)) in->selectSlot = 6;
    if (IsKeyPressed(KEY_EIGHT)) in->selectSlot = 7;
    if (IsKeyPressed(KEY_NINE))  in->selectSlot = 8;

}
