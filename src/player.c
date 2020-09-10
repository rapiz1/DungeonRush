#include "player.h"

#include <stdlib.h>
#include <string.h>

void initSnake(Snake* snake, int step, int team, PlayerType playerType) {
  snake->moveStep = step;
  snake->team = team;
  snake->num = 0;
  memset(snake->buffs, 0, sizeof(snake->buffs));
  snake->sprites = createLinkList();
  snake->score = createScore();
  snake->playerType = playerType;
}

Snake* createSnake(int step, int team, PlayerType playerType) {
  Snake* self = malloc(sizeof(Snake));
  initSnake(self, step, team, playerType);
  return self;
}
