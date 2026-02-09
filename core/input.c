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

}
