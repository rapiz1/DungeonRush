#ifndef SNAKE_SPRITE_H_
#define SNAKE_SPRITE_H_

#include "adt.h"
#include "types.h"
#include "weapon.h"

typedef struct {
  int x, y;
  Direction direction;
} PositionBufferSlot;

typedef struct {
  PositionBufferSlot buffer[POSITION_BUFFER_SIZE];
  int size;
} PositionBuffer;

void pushToPositionBuffer(PositionBuffer* buffer, PositionBufferSlot slot);
typedef struct {
  int x, y, hp, totalHp;
  Weapon* weapon;
  Animation* ani;
  Direction face;
  Direction direction;
  PositionBuffer posBuffer;
  int lastAttack;  // Timestamp of the last attack
  double dropRate;
} Sprite;

void changeSpriteDirection(LinkNode*, Direction);
void initSprite(Sprite* model, Sprite* self, int x, int y);
Sprite* createSprite(Sprite* model, int x, int y);

#endif
