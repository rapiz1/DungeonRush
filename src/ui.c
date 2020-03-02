#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "game.h"
#include "helper.h"
#include "map.h"
#include "render.h"
#include "res.h"
#include "storage.h"
#include "types.h"
#include "ui.h"
#include "audio.h"

extern LinkList animationsList[];
extern bool hasMap[MAP_SIZE][MAP_SIZE];
extern Text texts[TEXTSET_SIZE];
extern SDL_Renderer* renderer;
extern int renderFrames;
extern SDL_Color WHITE;
extern Texture textures[];
extern Effect effects[];

LinkList animationsList[ANIMATION_LINK_LIST_NUM];
int cursorPos;
bool moveCursor(int optsNum) {
  SDL_Event e;
  bool quit = false;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      quit = true;
      cursorPos = optsNum;
      return quit;
    } else if (e.type == SDL_KEYDOWN) {
      int keyValue = e.key.keysym.sym;
      switch (keyValue) {
        case SDLK_UP:
          cursorPos--;
          playAudio(AUDIO_INTER1);
          break;
        case SDLK_DOWN:
          cursorPos++;
          playAudio(AUDIO_INTER1);
          break;
        case SDLK_RETURN:
          quit = true;
          break;
        case SDLK_ESCAPE:
          quit = true;
          cursorPos = optsNum;
          playAudio(AUDIO_BUTTON1);
          return quit;
          break;
      }
    }
  }
  cursorPos += optsNum;
  cursorPos %= optsNum;
  return quit;
}
int chooseOptions(int optionsNum, Text** options) {
  cursorPos = 0;
  Snake* player = createSnake(2, 0);
  appendSpriteToSnake(player, SPRITE_KNIGHT, SCREEN_WIDTH / 2,
                      SCREEN_HEIGHT / 2, UP);
  int lineGap = FONT_SIZE + FONT_SIZE / 2,
      totalHeight = lineGap * (optionsNum - 1);
  int startY = (SCREEN_HEIGHT - totalHeight) / 2;
  while (!moveCursor(optionsNum)) {
    Sprite* sprite = player->sprites->head->element;
    sprite->ani->at = AT_CENTER;
    sprite->x = SCREEN_WIDTH / 2 - options[cursorPos]->width / 2 - UNIT / 2;
    sprite->y = startY + cursorPos * lineGap;
    updateAnimationOfSprite(sprite);
    renderUi();
    for (int i = 0; i < optionsNum; i++) {
      renderCenteredText(options[i], SCREEN_WIDTH / 2, startY + i * lineGap, 1);
    }
    // Update Screen
    SDL_RenderPresent(renderer);
    renderFrames++;
  }
  playAudio(AUDIO_BUTTON1);
  destroySnake(player);
  destroyAnimationsByLinkList(&animationsList[RENDER_LIST_SPRITE_ID]);
  return cursorPos;
}
void baseUi(int w, int h) {
  initRenderer();
  initBlankMap(w, h);
  pushMapToRender();
}
bool chooseLevelUi() {
  baseUi(30, 12);
  int optsNum = 3;
  Text** opts = malloc(sizeof(Text*) * optsNum);
  for (int i = 0; i < optsNum; i++) opts[i] = texts + i + 10;
  int opt = chooseOptions(optsNum, opts);
  if (opt != optsNum)
    setLevel(opt);
  blackout();
  clearRenderer();
  return opt != optsNum;
}
void mainUi() {
  baseUi(30, 12);
  playBgm(0);
  int startY = SCREEN_HEIGHT/2 - 70;
  int startX = SCREEN_WIDTH/5 + 32;
  createAndPushAnimation(&animationsList[RENDER_LIST_UI_ID], &textures[RES_TITLE], NULL, LOOP_INFI, 80, SCREEN_WIDTH/2, 280, SDL_FLIP_NONE, 0, AT_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_KNIGHT_M], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX,
                         startY, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_EFFECT_ID],
                         &textures[RES_SwordFx], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX + UI_MAIN_GAP_ALT*2,
                         startY - 32, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER)->scaled = false;
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_CHORT], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX + UI_MAIN_GAP_ALT*2,
                         startY - 32, SDL_FLIP_HORIZONTAL, 0, AT_BOTTOM_CENTER);

  startX += UI_MAIN_GAP_ALT*(6 + 2*randd());
  startY += UI_MAIN_GAP*(1 + randd());
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_ELF_M], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX,
                         startY, SDL_FLIP_HORIZONTAL, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_EFFECT_ID],
                         &textures[RES_HALO_EXPLOSION2], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX - UI_MAIN_GAP*1.5,
                         startY, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_ZOMBIE], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX - UI_MAIN_GAP*1.5,
                         startY, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);

  startX -= UI_MAIN_GAP_ALT*(1 + 2*randd());
  startY += UI_MAIN_GAP*(2 + randd());
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_WIZZARD_M], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX,
                         startY, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_EFFECT_ID],
                         &textures[RES_FIREBALL], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX + UI_MAIN_GAP,
                         startY, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);

  startX += UI_MAIN_GAP_ALT*(18 + 4*randd());
  startY -= UI_MAIN_GAP*(1 + 3*randd());
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_LIZARD_M], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX,
                         startY, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_EFFECT_ID],
                         &textures[RES_CLAWFX2], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX,
                         startY - UI_MAIN_GAP + 16, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_MUDDY], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX,
                         startY - UI_MAIN_GAP, SDL_FLIP_HORIZONTAL, 0, AT_BOTTOM_CENTER);

  createAndPushAnimation(&animationsList[RENDER_LIST_EFFECT_ID],
                         &textures[RES_CLAWFX2], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX + UI_MAIN_GAP,
                         startY - UI_MAIN_GAP + 16, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_SWAMPY], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX + UI_MAIN_GAP,
                         startY - UI_MAIN_GAP, SDL_FLIP_HORIZONTAL, 0, AT_BOTTOM_CENTER);

  createAndPushAnimation(&animationsList[RENDER_LIST_EFFECT_ID],
                         &textures[RES_CLAWFX2], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX + UI_MAIN_GAP,
                         startY + 16, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_SWAMPY], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX + UI_MAIN_GAP,
                         startY, SDL_FLIP_HORIZONTAL, 0, AT_BOTTOM_CENTER);
 /* 
  startX = SCREEN_WIDTH/3*2;
  startY = SCREEN_HEIGHT/3 + 10;
  int colNum = 8;
  for (int i = RES_TINY_ZOMBIE; i <= RES_CHORT; i+=2) {
    int col = (i - RES_TINY_ZOMBIE)%colNum;
    int row = (i - RES_TINY_ZOMBIE)/colNum;
    createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                          &textures[i], NULL, LOOP_INFI,
                          SPRITE_ANIMATION_DURATION, startX + col*UI_MAIN_GAP_ALT,
                          startY + row*UI_MAIN_GAP, SDL_FLIP_HORIZONTAL, 0, AT_BOTTOM_CENTER);
  }
  for (int i = RES_BIG_ZOMBIE; i <= RES_BIG_DEMON; i+=2) {
    createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                          &textures[i], NULL, LOOP_INFI,
                          SPRITE_ANIMATION_DURATION, startX + (i-RES_BIG_ZOMBIE)*UNIT,
                          startY + 200, SDL_FLIP_HORIZONTAL, 0, AT_BOTTOM_CENTER);
  }
  */
  int optsNum = 4;
  Text** opts = malloc(sizeof(Text*) * optsNum);
  for (int i = 0; i < optsNum; i++) opts[i] = texts + i + 6;
  int opt = chooseOptions(optsNum, opts);
  free(opts);

  blackout();
  clearRenderer();
  switch (opt) {
    case 0:
    case 1:
      if (!chooseLevelUi()) break;
      Score** scores = startGame(opt + 1);
      rankListUi(opt + 1, scores);
      for (int i = 0; i < opt + 1; i++) updateLocalRanklist(scores[i]);
      destroyRanklist(opt + 1, scores);
      break;
    case 2:
      localRankListUi();
      break;
    case 3:
      break;
  }
  if (opt == optsNum) return;
  if (opt != 3) {
    mainUi();
  }
}
void rankListUi(int count, Score** scores) {
  baseUi(30, 12 + MAX(0, count - 4));
  playBgm(0);
  Text** opts = malloc(sizeof(Text*) * count);
  char buf[1 << 8];
  for (int i = 0; i < count; i++) {
    sprintf(buf, "Score: %-6.0lf Got: %-6d Kill: %-6d Damage: %-6d Stand: %-6d",
            scores[i]->rank, scores[i]->got, scores[i]->killed,
            scores[i]->damage, scores[i]->stand);
    opts[i] = createText(buf, WHITE);
  }

  chooseOptions(count, opts);

  for (int i = 0; i < count; i++) destroyText(opts[i]);
  free(opts);
  blackout();
  clearRenderer();
}
void localRankListUi() {
  int count;
  Score** scores = readRanklist(STORAGE_PATH, &count);
  rankListUi(count, scores);
  destroyRanklist(count, scores);
}