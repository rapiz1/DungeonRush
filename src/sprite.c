#include "sprite.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "render.h"

void pushToPositionBuffer(PositionBuffer* b, PositionBufferSlot slot) {
  assert(b->size < POSITION_BUFFER_SIZE);
  b->buffer[b->size++] = slot;
}

void initSprite(Sprite* model, Sprite* self, int x, int y) {
  memcpy(self, model, sizeof(Sprite));
  self->x = x;
  self->y = y;
  self->posBuffer.size = 0;
  Animation* ani = malloc(sizeof(Animation));
  copyAnimation(model->ani, ani);
  self->ani = ani;
  updateAnimationOfSprite(self);
}
Sprite* createSprite(Sprite* model, int x, int y) {
  Sprite* self = malloc(sizeof(Sprite));
  initSprite(model, self, x, y);
  return self;
}
