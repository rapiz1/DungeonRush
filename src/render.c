#include "render.h"

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#include "ai.h"
#include "game.h"
#include "helper.h"
#include "res.h"
#include "types.h"

#ifdef DBG
#include <assert.h>
#endif

// Sprite
extern Snake* spriteSnake[SPRITES_MAX_NUM];
extern int spritesCount;
extern int playersCount;
extern Effect effects[];
extern SDL_Color BLACK;
extern SDL_Color WHITE;

const int SCALLING_FACTOR = 2;
extern int texturesCount;
extern Texture textures[TEXTURES_SIZE];
extern int textsCount;
extern Text texts[TEXTSET_SIZE];
Text* stageText;
Text* taskText;
Text* scoresText[MAX_PALYERS_NUM];

SDL_Renderer* renderer;
unsigned long long renderFrames;

LinkList animationsList[ANIMATION_LINK_LIST_NUM];
Animation* countDownBar;
void blacken(int duration) {
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
  SDL_SetRenderDrawColor(renderer, RENDER_BG_COLOR, 85);
  for (int i = 0; i < duration; i++) {
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderPresent(renderer);
  }
}
void blackout() { blacken(RENDER_BLACKOUT_DURATION); }
void dim() { blacken(RENDER_DIM_DURATION); }
void initCountDownBar() {
  createAndPushAnimation(
      &animationsList[RENDER_LIST_UI_ID], &textures[RES_SLIDER], NULL,
      LOOP_INFI, 1, SCREEN_WIDTH / 2 - 128, 10, SDL_FLIP_NONE, 0, AT_TOP_LEFT);
  countDownBar = createAndPushAnimation(
      &animationsList[RENDER_LIST_UI_ID], &textures[RES_BAR_BLUE], NULL,
      LOOP_INFI, 1, SCREEN_WIDTH / 2 - 128, 10, SDL_FLIP_NONE, 0, AT_TOP_LEFT);
}
void initInfo() {
  extern int stage;
  char buf[1 << 8];
  sprintf(buf, "Stage:%3d", stage);
  if (stageText)
    setText(stageText, buf);
  else
    stageText = createText(buf, WHITE);
  for (int i = 0; i < playersCount; i++)
    if (!scoresText[i]) scoresText[i] = createText("placeholder", WHITE);
  if (!taskText) taskText = createText("placeholder", WHITE);
}
void initRenderer() {
  renderFrames = 0;
  for (int i = 0; i < ANIMATION_LINK_LIST_NUM; i++) {
    initLinkList(&animationsList[i]);
  }
}
void clearInfo() {
  destroyText(stageText);
  stageText = NULL;
  destroyText(taskText);
  taskText = NULL;
  for (int i = 0; i < playersCount; i++) {
    destroyText(scoresText[i]);
    scoresText[i] = NULL;
  }
}
void clearRenderer() {
  for (int i = 0; i < ANIMATION_LINK_LIST_NUM; i++) {
    destroyAnimationsByLinkList(&animationsList[i]);
  }
  SDL_RenderClear(renderer);
}
void renderCstrCenteredText(const char* str, int x, int y, double scale) {
  Text* text = malloc(sizeof(Text));
  initText(text, str, WHITE);
  renderCenteredText(text, x, y, scale);
}
void renderCstrText(const char* str, int x, int y, double scale) {
  Text* text = malloc(sizeof(Text));
  initText(text, str, WHITE);
  renderText(text, x, y, scale);
}
void renderText(const Text* text, int x, int y, double scale) {
  SDL_Rect dst = {x, y, (int)(text->width * scale + 0.5),
                  (int)(text->height * scale + 0.5)};
  SDL_RenderCopy(renderer, text->origin, NULL, &dst);
}
SDL_Point renderCenteredText(const Text* text, int x, int y, double scale) {
  int width = text->width * scale + 0.5;
  int height = text->height * scale + 0.5;
  SDL_Rect dst = {x - width / 2, y - height / 2, width, height};
  SDL_RenderCopy(renderer, text->origin, NULL, &dst);
  return (SDL_Point){x - width / 2, y - height / 2};
}
void unsetEffect(Texture* texture) {
  SDL_SetTextureBlendMode(texture->origin, SDL_BLENDMODE_BLEND);
  SDL_SetTextureColorMod(texture->origin, 255, 255, 255);
  SDL_SetTextureAlphaMod(texture->origin, 255);
}
void setEffect(Texture* texture, Effect* effect) {
  if (!effect) return;
  SDL_SetTextureBlendMode(texture->origin, effect->mode);

  double interval = effect->duration / (effect->length - 1);
  double progress = effect->currentFrame;
  int stage = progress / interval;
  progress -= stage * interval;
  progress /= interval;

  SDL_Color mixed, prev = effect->keys[stage],
                   nxt = effect->keys[MIN(stage + 1, effect->length - 1)];
  mixed.a = prev.a * (1 - progress) + nxt.a * progress;
  mixed.r = prev.r * (1 - progress) + nxt.r * progress;
  mixed.g = prev.g * (1 - progress) + nxt.g * progress;
  mixed.b = prev.b * (1 - progress) + nxt.b * progress;

  SDL_SetTextureColorMod(texture->origin, mixed.r, mixed.g, mixed.b);
  SDL_SetTextureAlphaMod(texture->origin, mixed.a);
}
void updateAnimationOfSprite(Sprite* self) {
  Animation* ani = self->ani;
  ani->x = self->x;
  ani->y = self->y;
  ani->flip = self->face == RIGHT ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
}
void updateAnimationOfSnake(Snake* snake) {
  for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
    updateAnimationOfSprite(p->element);
  }
}
void updateAnimationOfBlock(Block* self) {
  Animation* ani = self->ani;
  ani->x = self->x;
  ani->y = self->y;
  if (self->bp == BLOCK_TRAP) {
    self->ani->origin = &textures[self->enable ? RES_FLOOR_SPIKE_ENABLED
                                               : RES_FLOOR_SPIKE_DISABLED];
  } else if (self->bp == BLOCK_EXIT) {
    if (self->enable && self->ani->origin != &textures[RES_FLOOR_EXIT]) {
      self->ani->origin = &textures[RES_FLOOR_EXIT];
      createAndPushAnimation(&animationsList[RENDER_LIST_MAP_SPECIAL_ID],
                             &textures[RES_FLOOR_EXIT], &effects[EFFECT_BLINK],
                             LOOP_INFI, 30, self->x, self->y, SDL_FLIP_NONE, 0,
                             AT_TOP_LEFT);
    }
  }
}
void clearBindInAnimationsList(Sprite* sprite, int id) {
  for (LinkNode *p = animationsList[id].head, *nxt; p; p = nxt) {
    nxt = p->nxt;
    Animation* ani = p->element;
    if (ani->bind == sprite) {
      ani->bind = NULL;
      if (ani->strongBind) {
        removeLinkNode(&animationsList[id], p);
        destroyAnimation(ani);
      }
    }
  }
}
void bindAnimationToSprite(Animation* ani, Sprite* sprite, bool isStrong) {
  ani->bind = sprite;
  ani->strongBind = isStrong;
  updateAnimationFromBind(ani);
}
void updateAnimationFromBind(Animation* ani) {
  if (ani->bind) {
    Sprite* sprite = ani->bind;
    ani->x = sprite->x;
    ani->y = sprite->y;
    ani->flip = sprite->ani->flip;
  }
}
void renderAnimation(Animation* ani) {
  if (!ani) return;
  updateAnimationFromBind(ani);
  int width = ani->origin->width;
  int height = ani->origin->height;
  SDL_Point poi = {ani->origin->width, ani->origin->height / 2};
  if (ani->scaled) {
    width *= SCALLING_FACTOR;
    height *= SCALLING_FACTOR;
  }
  SDL_Rect dst = {ani->x - width / 2, ani->y - height, width, height};
  if (ani->at == AT_TOP_LEFT) {
    dst.x = ani->x;
    dst.y = ani->y;
  } else if (ani->at == AT_CENTER) {
    dst.x = ani->x - width / 2;
    dst.y = ani->y - height / 2;
    poi.x = ani->origin->width / 2;
  } else if (ani->at == AT_BOTTOM_LEFT) {
    dst.x = ani->x;
    dst.y = ani->y + UNIT - height - 3;
  }
  if (ani->effect) {
    setEffect(ani->origin, ani->effect);
    ani->effect->currentFrame %= ani->effect->duration;
  }
#ifdef DBG
  assert(ani->duration >= ani->origin->frames);
#endif
  int stage = 0;
  if (ani->origin->frames > 1) {
    double interval = (double)ani->duration / ani->origin->frames;
    stage = ani->currentFrame / interval;
  }
  SDL_RenderCopyEx(renderer, ani->origin->origin, &(ani->origin->crops[stage]),
                   &dst, ani->angle, &poi, ani->flip);
  if (ani->effect) unsetEffect(ani->origin);
#ifdef DBG_CROSS
  if (ani->at == AT_BOTTOM_CENTER) {
    Sprite fake;
    fake.ani = ani;
    SDL_Rect tmp;

    tmp = getSpriteBoundBox(&fake);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 200);
    SDL_RenderDrawRect(renderer, &tmp);

    tmp = getSpriteFeetBox(&fake);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 200);
    SDL_RenderDrawRect(renderer, &tmp);

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 200);
    SDL_RenderDrawRect(renderer, &dst);
  }
#endif
}
void pushAnimationToRender(int id, Animation* ani) {
  LinkNode* p = createLinkNode(ani);
  pushLinkNode(&animationsList[id], p);
}
Animation* createAndPushAnimation(LinkList* list, Texture* texture,
                                  const Effect* effect, LoopType lp,
                                  int duration, int x, int y,
                                  SDL_RendererFlip flip, double angle, At at) {
  Animation* ani =
      createAnimation(texture, effect, lp, duration, x, y, flip, angle, at);
  LinkNode* node = createLinkNode(ani);
  pushLinkNode(list, node);
  return ani;
}
void updateAnimationLinkList(LinkList* list) {
  LinkNode* p = list->head;
  while (p) {
    Animation* ani = p->element;
    LinkNode* nxt = p->nxt;
    ani->currentFrame++;
    ani->lifeSpan--;
    if (ani->effect) {
      ani->effect->currentFrame++;
      ani->effect->currentFrame %= ani->effect->duration;
    }
    if (ani->lp == LOOP_ONCE) {
      if (ani->currentFrame == ani->duration) {
        destroyAnimation(p->element);
        removeLinkNode(list, p);
      }
    } else {
      if (ani->lp == LOOP_LIFESPAN && !ani->lifeSpan) {
        destroyAnimation(p->element);
        removeLinkNode(list, p);
      } else
        ani->currentFrame %= ani->duration;
    }
    p = nxt;
  }
}
int compareAnimationByY(const void* x, const void* y) {
  Animation* a = *(Animation**)x;
  Animation* b = *(Animation**)y;
  return b->y - a->y;
}
void renderAnimationLinkList(LinkList* list) {
  LinkNode* p = list->head;
  while (p) {
    renderAnimation(p->element);
    p = p->nxt;
  }
}
void renderAnimationLinkListWithSort(LinkList* list) {
  static Animation* buffer[RENDER_BUFFER_SIZE];
  int count = 0;
  LinkNode* p = list->head;
  while (p) {
    buffer[count++] = p->element;
    p = p->nxt;
  }
  qsort(buffer, count, sizeof(Animation*), compareAnimationByY);
  while (count) renderAnimation(buffer[--count]);
}
void renderSnakeHp(Snake* snake) {
  for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
    Sprite* sprite = p->element;
    if (sprite->hp == sprite->totoalHp) continue;
    double percent = (double)sprite->hp / sprite->totoalHp;
    for (int i = 0; percent > 1e-8; i++, percent -= 1) {
      int r = 0, g = 0, b = 0;
      if (i == 0) {
        if (percent < 1) {
          r = MIN((1 - percent) * 2 * 255, 255),
          g = MAX(0, 255 - (MAX(0.5 - percent, 0)) * 2 * 255);
        } else {
          g = 255;
        }
      } else {
        r = g = 0, b = 255;
      }
      SDL_SetRenderDrawColor(renderer, r, g, b, 255);
      int width = RENDER_HP_BAR_WIDTH;
      int spriteHeight = sprite->ani->origin->height * SCALLING_FACTOR;
      SDL_Rect bar = {sprite->x - UNIT / 2 + (UNIT - width) / 2,
                      sprite->y - spriteHeight - RENDER_HP_BAR_HEIGHT * (i + 1),
                      width * MIN(1, percent), RENDER_HP_BAR_HEIGHT};
      SDL_RenderDrawRect(renderer, &bar);
    }
  }
}
void renderHp() {
  for (int i = 0; i < spritesCount; i++) renderSnakeHp(spriteSnake[i]);
}
void renderCenteredTextBackground(Text* text, int x, int y, double scale) {
  int width = text->width * scale + 0.5;
  int height = text->height * scale + 0.5;
  SDL_Rect dst = {x - width / 2, y - height / 2, width, height};
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 200);
  SDL_RenderFillRect(renderer, &dst);
}
void renderId() {
  int powerful = getPowerfulPlayer();
  for (int i = 0; i < playersCount; i++) {
    Snake* snake = spriteSnake[i];
    if (snake->sprites->head) {
      Sprite* snakeHead = snake->sprites->head->element;
      if (i == powerful)
        renderCenteredTextBackground(&texts[4 + i], snakeHead->x, snakeHead->y,
                                     0.5);
      renderCenteredText(&texts[4 + i], snakeHead->x, snakeHead->y, 0.5);
    }
  }
}
void renderCountDown() {
  double percent =
      (double)(renderFrames % GAME_MAP_RELOAD_PERIOD) / GAME_MAP_RELOAD_PERIOD;
  int width = percent * UI_COUNTDOWN_BAR_WIDTH;
  countDownBar->origin->crops[0].w = countDownBar->origin->width = width;
}
void renderInfo() {
  int startY = 0, startX = 10;
  int lineGap = FONT_SIZE;
  renderText(stageText, startX, startY, 1);
  startY += lineGap;
  for (int i = 0; i < playersCount; i++) {
    char buf[1 << 8];
    calcScore(spriteSnake[i]->score);
    sprintf(buf, "Player%d:%5d", i + 1,
            (int)(spriteSnake[i]->score->rank + 0.5));
    setText(scoresText[i], buf);
    renderText(scoresText[i], startX, startY, 1);
    startY += lineGap;
  }
  if (playersCount == 1) {
    extern int GAME_WIN_NUM;
    char buf[1 << 8];
    sprintf(buf, "Find %d more heros!",
            GAME_WIN_NUM > spriteSnake[0]->num
                ? GAME_WIN_NUM - spriteSnake[0]->num
                : 0);
    setText(taskText, buf);
    renderText(taskText, startX, startY, 1);
    startY += lineGap;
  }
}
void render() {
  SDL_SetRenderDrawColor(renderer, 25, 17, 23, 255);
  SDL_RenderClear(renderer);

  for (int i = 0; i < ANIMATION_LINK_LIST_NUM; i++) {
    updateAnimationLinkList(&animationsList[i]);
    if (i == RENDER_LIST_SPRITE_ID)
      renderAnimationLinkListWithSort(&animationsList[i]);
    else
      renderAnimationLinkList(&animationsList[i]);
  }
  renderHp();
  renderCountDown();
  renderInfo();
  renderId();
  // Update Screen
  SDL_RenderPresent(renderer);
  renderFrames++;
}
void renderUi() {
  SDL_SetRenderDrawColor(renderer, RENDER_BG_COLOR, 255);
  SDL_RenderClear(renderer);

  for (int i = 0; i < ANIMATION_LINK_LIST_NUM; i++) {
    updateAnimationLinkList(&animationsList[i]);
    if (i == RENDER_LIST_SPRITE_ID)
      renderAnimationLinkListWithSort(&animationsList[i]);
    else
      renderAnimationLinkList(&animationsList[i]);
  }
}
