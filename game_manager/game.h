#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

typedef struct Game Game;

Game *Game_Create(void);
void Game_Destroy(Game *g);

void Game_Tick(Game *g);

bool Game_ShouldClose(const Game *g);

#endif // GAME_H
