#include "game_manager/game.h"

int main(void)
{
    Game *game = Game_Create();
    if (!game) return 1;

    while (!Game_ShouldClose(game)) {
        Game_Tick(game);
    }

    Game_Destroy(game);
    return 0;
}
