#include "ai.h"

#include "helper.h"
#include "map.h"
#include "res.h"
#include "sprite.h"

extern Block map[MAP_SIZE][MAP_SIZE];
extern Item itemMap[MAP_SIZE][MAP_SIZE];
extern bool hasMap[MAP_SIZE][MAP_SIZE];
extern bool hasEnemy[MAP_SIZE][MAP_SIZE];
extern int spikeDamage;
extern int playersCount;
extern const int n, m;
extern const int SCALLING_FACTOR;

// Sprite
extern Snake* spriteSnake[SPRITES_MAX_NUM];
extern int spritesCount;
double AI_LOCK_LIMIT;

int trapVerdict(Sprite* sprite) {
  int ret = 0;
  int x = sprite->x, y = sprite->y;
  SDL_Rect block, box = getSpriteFeetBox(sprite);
  for (int dx = -1; dx <= 1; dx++)
    for (int dy = -1; dy <= 1; dy++) {
      int xx = x / UNIT + dx, yy = y / UNIT + dy;
      if (inr(xx, 0, n - 1) && inr(yy, 0, m - 1)) {
        block = getMapRect(xx, yy);
        if (RectRectCross(&box, &block) && hasMap[xx][yy] &&
            map[xx][yy].bp == BLOCK_TRAP) {
          ret += map[xx][yy].enable + 1;
        }
      }
    }
  return ret;
}
int getPowerfulPlayer() {
  int maxNum = 0, mxCount = 0, id = -1;
  for (int i = 0; i < playersCount; i++) {
    int num = spriteSnake[i]->num;
    if (num > maxNum) {
      maxNum = num;
      mxCount = 1;
      id = i;
    } else if (num == maxNum)
      mxCount++;
  }
  return id != -1 && mxCount == 1
             ? (spriteSnake[id]->num >= AI_LOCK_LIMIT ? id : -1)
             : -1;
}
int balanceVerdict(Sprite* sprite, int id) {
  if (id == -1) return 0;
  if (!spriteSnake[id]->sprites->head) return 0;
  int ret = 0;
  Sprite* player = spriteSnake[id]->sprites->head->element;
  if (player->x < sprite->x && sprite->direction == LEFT) ret++;
  if (player->x > sprite->x && sprite->direction == RIGHT) ret++;
  if (player->y > sprite->y && sprite->direction == DOWN) ret++;
  if (player->y < sprite->y && sprite->direction == UP) ret++;
  return ret;
}
int testOneMove(Snake* snake, Direction direction) {
  Sprite* snakeHead = snake->sprites->head->element;
  Direction recover = snakeHead->direction;
  snakeHead->direction = direction;
  int crush = 0, trap = 0, playerBalance = 0;
  int powerful = getPowerfulPlayer();
  for (int i = 1; i <= AI_PREDICT_STEPS; i++) {
    moveSprite(snakeHead, snake->moveStep * i);
    updateAnimationOfSprite(snakeHead);
    crush -= crushVerdict(snakeHead, false, true) * 500;
    trap -= trapVerdict(snakeHead);
    playerBalance += balanceVerdict(snakeHead, powerful) * 10;
    // revoke position
    moveSprite(snakeHead, -snake->moveStep * i);
    updateAnimationOfSprite(snakeHead);
  }
  snakeHead->direction = recover;
  return trap + crush + playerBalance;
}
int compareChoiceByValue(const void* x, const void* y) {
  const Choice *a = x, *b = y;
  return b->value - a->value;
}
void AiInput(Snake* snake) {
  Sprite* snakeHead = snake->sprites->head->element;
  Direction currentDirection = snakeHead->direction;
  int originValue = testOneMove(snake, currentDirection);
  bool change = originValue < 0;
  if (randDouble() < AI_PATH_RANDOM) change = 1;
  if (change) {
    static Choice choices[4];
    int count = 0;
    for (int i = LEFT; i <= DOWN; i++)
      if (i != currentDirection && (i ^ 1) != currentDirection) {
        int value = testOneMove(snake, i);
        if (value >= originValue) choices[count++] = (Choice){value, i};
      }
    if (count) {
      int maxValue = choices[0].value;
      int nowChoice = 0;
      for (int i = 1; i < 4; i++) {
        if (choices[i].value > maxValue) {
          maxValue = choices[i].value;
          nowChoice = i;
        }
      }
      changeSpriteDirection(snake->sprites->head, choices[nowChoice].direction);
    }
  }
}
