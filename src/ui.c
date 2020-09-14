#include "ui.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "game.h"
#include "helper.h"
#include "map.h"
#include "net.h"
#include "render.h"
#include "res.h"
#include "storage.h"
#include "text.h"
#include "types.h"

extern LinkList animationsList[];
extern bool hasMap[MAP_SIZE][MAP_SIZE];
extern Text texts[TEXTSET_SIZE];
extern SDL_Renderer* renderer;
extern int renderFrames;
extern SDL_Color WHITE;
extern Texture textures[];
extern Effect effects[];

extern LinkList animationsList[ANIMATION_LINK_LIST_NUM];
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
  Snake* player = createSnake(2, 0, LOCAL);
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
  if (opt != optsNum) setLevel(opt);
  clearRenderer();
  return opt != optsNum;
}

void launchLocalGame(int localPlayerNum) {
  Score** scores = startGame(localPlayerNum, 0, true);
  rankListUi(localPlayerNum, scores);
  for (int i = 0; i < localPlayerNum; i++) updateLocalRanklist(scores[i]);
  destroyRanklist(localPlayerNum, scores);
}
int rangeOptions(int start, int end) {
  int optsNum = end - start + 1;
  Text** opts = malloc(sizeof(Text*) * optsNum);
  for (int i = 0; i < optsNum; i++) opts[i] = texts + i + start;
  int opt = chooseOptions(optsNum, opts);
  free(opts);
  return opt;
}

char* inputUi() {
  const int MAX_LEN = 30;

  baseUi(20, 10);

  char* ret = malloc(MAX_LEN);
  int retLen = 0;
  memset(ret, 0, MAX_LEN);

  extern SDL_Color WHITE;
  Text* text = NULL;
  Text* placeholder = createText("Enter IP", WHITE);

  SDL_StartTextInput();
  SDL_Event e;
  bool quit = false;
  bool finished = false;
  while (!quit && !finished) {
    const Text* displayText = NULL;
    if (ret[0]) {
      if (text)
        setText(text, ret);
      else
        text = createText(ret, WHITE);
      displayText = text;
    } else {
      displayText = placeholder;
    }
    renderCenteredText(displayText, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 2);
    SDL_RenderPresent(renderer);
    clearRenderer();

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT ||
          (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
        quit = true;
        break;
      } else if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_BACKSPACE) {
          if (retLen) ret[--retLen] = 0;
        } else if (e.key.keysym.sym == SDLK_RETURN) {
          finished = true;
          break;
        }
      } else if (e.type == SDL_TEXTINPUT) {
        strcpy(ret + retLen, e.text.text);
        retLen += strlen(e.text.text);
      }
    }
  }

  SDL_StopTextInput();
  destroyText(placeholder);
  destroyText(text);

  if (quit) {
    free(ret);
    return NULL;
  }

  return ret;
}

void launchLanGame() {
  baseUi(10, 10);
  int opt = rangeOptions(LAN_HOSTGAME, LAN_JOINGAME);
  blackout();
  clearRenderer();
  if (opt == 0) {
    hostGame();
  } else {
    char* ip = inputUi();
    if (ip == NULL) return;
    joinGame(ip, LAN_LISTEN_PORT);
    free(ip);
  }
}

int chooseOnLanUi() {
  baseUi(10, 10);
  int opt = rangeOptions(MULTIPLAYER_LOCAL, MULTIPLAYER_LAN);
  clearRenderer();
  return opt;
}

void mainUi() {
  baseUi(30, 12);
  playBgm(0);
  int startY = SCREEN_HEIGHT / 2 - 70;
  int startX = SCREEN_WIDTH / 5 + 32;
  createAndPushAnimation(&animationsList[RENDER_LIST_UI_ID],
                         &textures[RES_TITLE], NULL, LOOP_INFI, 80,
                         SCREEN_WIDTH / 2, 280, SDL_FLIP_NONE, 0, AT_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_KNIGHT_M], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX, startY,
                         SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(
      &animationsList[RENDER_LIST_EFFECT_ID], &textures[RES_SwordFx], NULL,
      LOOP_INFI, SPRITE_ANIMATION_DURATION, startX + UI_MAIN_GAP_ALT * 2,
      startY - 32, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER)
      ->scaled = false;
  createAndPushAnimation(
      &animationsList[RENDER_LIST_SPRITE_ID], &textures[RES_CHORT], NULL,
      LOOP_INFI, SPRITE_ANIMATION_DURATION, startX + UI_MAIN_GAP_ALT * 2,
      startY - 32, SDL_FLIP_HORIZONTAL, 0, AT_BOTTOM_CENTER);

  startX += UI_MAIN_GAP_ALT * (6 + 2 * randDouble());
  startY += UI_MAIN_GAP * (1 + randDouble());
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_ELF_M], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX, startY,
                         SDL_FLIP_HORIZONTAL, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_EFFECT_ID],
                         &textures[RES_HALO_EXPLOSION2], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX - UI_MAIN_GAP * 1.5,
                         startY, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_ZOMBIE], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX - UI_MAIN_GAP * 1.5,
                         startY, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);

  startX -= UI_MAIN_GAP_ALT * (1 + 2 * randDouble());
  startY += UI_MAIN_GAP * (2 + randDouble());
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_WIZZARD_M], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX, startY,
                         SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(&animationsList[RENDER_LIST_EFFECT_ID],
                         &textures[RES_FIREBALL], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX + UI_MAIN_GAP,
                         startY, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);

  startX += UI_MAIN_GAP_ALT * (18 + 4 * randDouble());
  startY -= UI_MAIN_GAP * (1 + 3 * randDouble());
  createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                         &textures[RES_LIZARD_M], NULL, LOOP_INFI,
                         SPRITE_ANIMATION_DURATION, startX, startY,
                         SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(
      &animationsList[RENDER_LIST_EFFECT_ID], &textures[RES_CLAWFX2], NULL,
      LOOP_INFI, SPRITE_ANIMATION_DURATION, startX, startY - UI_MAIN_GAP + 16,
      SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(
      &animationsList[RENDER_LIST_SPRITE_ID], &textures[RES_MUDDY], NULL,
      LOOP_INFI, SPRITE_ANIMATION_DURATION, startX, startY - UI_MAIN_GAP,
      SDL_FLIP_HORIZONTAL, 0, AT_BOTTOM_CENTER);

  createAndPushAnimation(
      &animationsList[RENDER_LIST_EFFECT_ID], &textures[RES_CLAWFX2], NULL,
      LOOP_INFI, SPRITE_ANIMATION_DURATION, startX + UI_MAIN_GAP,
      startY - UI_MAIN_GAP + 16, SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  createAndPushAnimation(
      &animationsList[RENDER_LIST_SPRITE_ID], &textures[RES_SWAMPY], NULL,
      LOOP_INFI, SPRITE_ANIMATION_DURATION, startX + UI_MAIN_GAP,
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
                           SPRITE_ANIMATION_DURATION, startX +
   col*UI_MAIN_GAP_ALT, startY + row*UI_MAIN_GAP, SDL_FLIP_HORIZONTAL, 0,
   AT_BOTTOM_CENTER);
   }
   for (int i = RES_BIG_ZOMBIE; i <= RES_BIG_DEMON; i+=2) {
     createAndPushAnimation(&animationsList[RENDER_LIST_SPRITE_ID],
                           &textures[i], NULL, LOOP_INFI,
                           SPRITE_ANIMATION_DURATION, startX +
   (i-RES_BIG_ZOMBIE)*UNIT, startY + 200, SDL_FLIP_HORIZONTAL, 0,
   AT_BOTTOM_CENTER);
   }
   */
  int optsNum = 4;
  Text** opts = malloc(sizeof(Text*) * optsNum);
  for (int i = 0; i < optsNum; i++) opts[i] = texts + i + 6;
  int opt = chooseOptions(optsNum, opts);
  free(opts);

  blackout();
  clearRenderer();
  int lan;
  switch (opt) {
    case 0:
      if (!chooseLevelUi()) break;
      launchLocalGame(1);
      break;
    case 1:
      lan = chooseOnLanUi();
      if (lan == 0) {
        if (!chooseLevelUi()) break;
        launchLocalGame(2);
      } else if (lan == 1) {
        launchLanGame();
      }
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
