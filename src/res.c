#include "res.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_net.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "render.h"
#include "weapon.h"



// Constants
const int n = SCREEN_WIDTH/UNIT;
const int m = SCREEN_HEIGHT/UNIT;

const char tilesetPath[TILESET_SIZE][PATH_LEN] = {
    "res/drawable/0x72_DungeonTilesetII_v1_3",
    "res/drawable/fireball_explosion1",
    "res/drawable/halo_explosion1",
    "res/drawable/halo_explosion2",
    "res/drawable/fireball",
    "res/drawable/floor_spike",
    "res/drawable/floor_exit",
    "res/drawable/HpMed",
    "res/drawable/SwordFx",
    "res/drawable/ClawFx",
    "res/drawable/Shine",
    "res/drawable/Thunder",
    "res/drawable/BloodBound",
    "res/drawable/arrow",
    "res/drawable/explosion-2",
    "res/drawable/ClawFx2",
    "res/drawable/Axe",
    "res/drawable/cross_hit",
    "res/drawable/blood",
    "res/drawable/SolidFx",
    "res/drawable/IcePick",
    "res/drawable/IceShatter",
    "res/drawable/Ice",
    "res/drawable/SwordPack",
    "res/drawable/HolyShield",
    "res/drawable/golden_cross_hit",
    "res/drawable/ui",
    "res/drawable/title",
    "res/drawable/purple_ball",
    "res/drawable/purple_exp",
    "res/drawable/staff",
    "res/drawable/Thunder_Yellow",
    "res/drawable/attack_up",
    "res/drawable/powerful_bow"};
const char fontPath[] = "res/font/m5x7.ttf";
const char textsetPath[] = "res/text.txt";

const int bgmNums = 4;
const char bgmsPath[AUDIO_BGM_SIZE][PATH_LEN] = {
  "res/audio/main_title.ogg",
  "res/audio/bg1.ogg",
  "res/audio/bg2.ogg",
  "res/audio/bg3.ogg"
};
const char soundsPath[PATH_LEN] = "res/audio/sounds";
const char soundsPathPrefix[PATH_LEN] = "res/audio/";
// Gloabls
int texturesCount;
Texture textures[TEXTURES_SIZE];
int textsCount;
Text texts[TEXTSET_SIZE];

extern SDL_Color BLACK;
extern SDL_Color WHITE;
extern SDL_Renderer* renderer;
extern Weapon weapons[WEAPONS_SIZE];

SDL_Window* window;
SDL_Texture* originTextures[TILESET_SIZE];
TTF_Font* font;

Effect effects[EFFECTS_SIZE];

Sprite commonSprites[COMMON_SPRITE_SIZE];

Mix_Music *mainTitle;
Mix_Music *bgms[AUDIO_BGM_SIZE];
int soundsCount;
Mix_Chunk *sounds[AUDIO_SOUND_SIZE];

bool init() {
  // Initialization flag
  bool success = true;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    success = false;
  } else {
    // Create window
    window = SDL_CreateWindow("Dungeon Rush "VERSION_STRING, SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
      success = false;
    } else {
      // Software Render
#ifndef SOFTWARE_ACC
      renderer = SDL_CreateRenderer(
          window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
#endif
#ifdef SOFTWARE_ACC
      printf("define software acc\n");
      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
#endif
      if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n",
               SDL_GetError());
        success = false;
      } else {
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        // Initialize PNG loading
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
          printf("SDL_image could not initialize! SDL_image Error: %s\n",
                 IMG_GetError());
          success = false;
        }
        // Initialize SDL_ttf
        if (TTF_Init() == -1) {
          printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n",
                 TTF_GetError());
          success = false;
        }
        //Initialize SDL_mixer
        if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) {
          printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
          success = false;
        }
        //Initialize SDL_net
        if (SDLNet_Init() == -1) {
          printf("SDL_Net_Init: %s\n", SDLNet_GetError());
          success = false;
        }
      }
    }
  }
  return success;
}
SDL_Texture* loadSDLTexture(const char* path) {
  // The final texture
  SDL_Texture* newTexture = NULL;

  // Load image at specified path
  SDL_Surface* loadedSurface = IMG_Load(path);
  if (loadedSurface == NULL) {
    printf("Unable to load image %s! SDL_image Error: %s\n", path,
           IMG_GetError());
  } else {
    // Create texture from surface pixels
    newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (newTexture == NULL) {
      printf("Unable to create texture from %s! SDL Error: %s\n", path,
             SDL_GetError());
    }

    // Get rid of old loaded surface
    SDL_FreeSurface(loadedSurface);
  }

  return newTexture;
}
bool loadTextset() {
  bool success = true;
  FILE* file = fopen(textsetPath, "r");
  char str[TEXT_LEN];
  while (fgets(str, TEXT_LEN, file)) {
    int n = strlen(str);
    while (n - 1 >= 0 && !isprint(str[n - 1])) str[--n] = 0;
    if (!n) continue;
    if (!initText(&texts[textsCount++], str, WHITE)) {
      success = false;
    }
#ifdef DBG
    printf("Texts #%d: %s loaded\n", textsCount - 1, str);
#endif
  }
  fclose(file);
  return success;
}
bool loadTileset(const char* path, SDL_Texture* origin) {
  FILE* file = fopen(path, "r");
  int x, y, w, h, f;
  char resName[256];
  while (fscanf(file, "%s %d %d %d %d %d", resName, &x, &y, &w, &h, &f) == 6) {
    Texture* p = &textures[texturesCount++];
    initTexture(p, origin, w, h, f);
    for (int i = 0; i < f; i++) {
      p->crops[i].x = x + i * w;
      p->crops[i].y = y;
      p->crops[i].h = h;
      p->crops[i].w = w;
    }
#ifdef DBG
    printf("Resources #%d: %s %d %d %d %d %d loaded\n", texturesCount - 1,
           resName, x, y, w, h, f);
#endif
  }
  fclose(file);
  return true;
}
bool loadAudio() {
  bool success = true;
  for (int i = 0; i < bgmNums; i++) {
    bgms[i] = Mix_LoadMUS(bgmsPath[i]);
    success &= bgms[i] != NULL;
    if (!bgms[i]) printf("Failed to load %s: SDL_mixer Error: %s\n", bgmsPath[i], Mix_GetError());
    #ifdef DBG
    else printf("BGM %s loaded\n", bgmsPath[i]);
    #endif
  }
  FILE* f = fopen(soundsPath,"r");
  char buf[PATH_LEN], path[PATH_LEN<<1];
  while (~fscanf(f, "%s", buf)) {
    sprintf(path, "%s%s", soundsPathPrefix, buf);
    sounds[soundsCount] = Mix_LoadWAV(path);
    success &= sounds[soundsCount] != NULL;
    if (!sounds[soundsCount]) printf("Failed to load %s: : SDL_mixer Error: %s\n", path, Mix_GetError());
    #ifdef DBG
    else printf("Sound #%d: %s\n", soundsCount, path);
    #endif
    soundsCount++;
  }
  fclose(f);
  return success;
}
bool loadMedia() {
  // Loading success flag
  bool success = true;
  // load effects
  initCommonEffects();
  // Load tileset
  char imgPath[PATH_LEN + 4];
  for (int i = 0; i < TILESET_SIZE; i++) {
    if (!strlen(tilesetPath[i])) break;
    sprintf(imgPath, "%s.png", tilesetPath[i]);
    originTextures[i] = loadSDLTexture(imgPath);
    loadTileset(tilesetPath[i], originTextures[i]);
    success &= (bool)originTextures[i];
  }
  // Open the font
  font = TTF_OpenFont(fontPath, FONT_SIZE);
  if (font == NULL) {
    printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
    success = false;
  } else {
    if (!loadTextset()) {
      printf("Failed to load textset!\n");
      success = false;
    }
  }
  // Init common sprites
  initWeapons();
  initCommonSprites();

  if (!loadAudio()) {
    printf("Failed to load audio!\n");
    success = false;
  }

  return success;
}
void cleanup() {
  // Deallocate surface
  for (int i = 0; i < TILESET_SIZE; i++) {
    SDL_DestroyTexture(originTextures[i]);
    originTextures[i] = NULL;
  }
  // Destroy window
  SDL_DestroyRenderer(renderer);
  renderer = NULL;
  SDL_DestroyWindow(window);
  window = NULL;

  // Quit SDL subsystems
  TTF_Quit();
  IMG_Quit();
  Mix_CloseAudio();
  SDLNet_Quit();
  SDL_Quit();
}
void initCommonEffects() {
  // Effect #0: Death
  initEffect(&effects[0], 30, 4, SDL_BLENDMODE_BLEND);
  SDL_Color death = {255, 255, 255, 255};
  effects[0].keys[0] = death;
  death.g = death.b = 0;
  death.r = 168;
  effects[0].keys[1] = death;
  death.r = 80;
  effects[0].keys[2] = death;
  death.r = death.a = 0;
  effects[0].keys[3] = death;
#ifdef DBG
  puts("Effect #0: Death loaded");
#endif

  // Effect #1: Blink ( white )
  initEffect(&effects[1], 30, 3, SDL_BLENDMODE_ADD);
  SDL_Color blink = {0, 0, 0, 255};
  effects[1].keys[0] = blink;
  blink.r = blink.g = blink.b = 200;
  effects[1].keys[1] = blink;
  blink.r = blink.g = blink.b = 0;
  effects[1].keys[2] = blink;
#ifdef DBG
  puts("Effect #1: Blink (white) loaded");
#endif
  initEffect(&effects[2], 30, 2, SDL_BLENDMODE_BLEND);
  SDL_Color vanish = {255, 255, 255, 255};
  effects[2].keys[0] = vanish;
  vanish.a = 0;
  effects[2].keys[1] = vanish;
#ifdef DBG
  puts("Effect #2: Vanish (30fm) loaded");
#endif
}
void initCommonSprite(Sprite* sprite, Weapon* weapon, int res_id, int hp) {
  Animation* ani = createAnimation(&textures[res_id], NULL, LOOP_INFI,
                SPRITE_ANIMATION_DURATION, 0, 0, SDL_FLIP_NONE, 0,
                AT_BOTTOM_CENTER);
  *sprite = (Sprite){0, 0, hp, hp, weapon, ani, RIGHT, RIGHT};
  sprite->lastAttack = 0;
  sprite->dropRate = 1;
}
void initCommonSprites() {
  initCommonSprite(&commonSprites[SPRITE_KNIGHT], &weapons[WEAPON_SWORD], RES_KNIGHT_M, 150);
  initCommonSprite(&commonSprites[SPRITE_ELF], &weapons[WEAPON_ARROW],RES_ELF_M, 100);
  initCommonSprite(&commonSprites[SPRITE_WIZZARD], &weapons[WEAPON_FIREBALL],RES_WIZZARD_M, 95);
  initCommonSprite(&commonSprites[SPRITE_LIZARD], &weapons[WEAPON_MONSTER_CLAW], RES_LIZARD_M, 120);
  initCommonSprite(&commonSprites[SPRITE_TINY_ZOMBIE], &weapons[WEAPON_MONSTER_CLAW2], RES_TINY_ZOMBIE, 50);
  initCommonSprite(&commonSprites[SPRITE_GOBLIN], &weapons[WEAPON_MONSTER_CLAW2], RES_GOBLIN, 100);
  initCommonSprite(&commonSprites[SPRITE_IMP], &weapons[WEAPON_MONSTER_CLAW2], RES_IMP, 100);
  initCommonSprite(&commonSprites[SPRITE_SKELET], &weapons[WEAPON_MONSTER_CLAW2], RES_SKELET, 100);
  initCommonSprite(&commonSprites[SPRITE_MUDDY], &weapons[WEAPON_SOLID], RES_MUDDY, 150);
  initCommonSprite(&commonSprites[SPRITE_SWAMPY], &weapons[WEAPON_SOLID_GREEN], RES_SWAMPY, 150);
  initCommonSprite(&commonSprites[SPRITE_ZOMBIE], &weapons[WEAPON_MONSTER_CLAW2], RES_ZOMBIE, 120);
  initCommonSprite(&commonSprites[SPRITE_ICE_ZOMBIE], &weapons[WEAPON_ICEPICK], RES_ICE_ZOMBIE, 120);
  initCommonSprite(&commonSprites[SPRITE_MASKED_ORC], &weapons[WEAPON_THROW_AXE], RES_MASKED_ORC, 120);
  initCommonSprite(&commonSprites[SPRITE_ORC_WARRIOR], &weapons[WEAPON_MONSTER_CLAW2], RES_ORC_WARRIOR, 200);
  initCommonSprite(&commonSprites[SPRITE_ORC_SHAMAN], &weapons[WEAPON_MONSTER_CLAW2], RES_ORC_SHAMAN, 120);
  initCommonSprite(&commonSprites[SPRITE_NECROMANCER], &weapons[WEAPON_PURPLE_BALL], RES_NECROMANCER, 120);
  initCommonSprite(&commonSprites[SPRITE_WOGOL], &weapons[WEAPON_MONSTER_CLAW2], RES_WOGOL, 150);
  initCommonSprite(&commonSprites[SPRITE_CHROT], &weapons[WEAPON_MONSTER_CLAW2], RES_CHORT, 150);
  Sprite* now;
  initCommonSprite(now=&commonSprites[SPRITE_BIG_ZOMBIE], &weapons[WEAPON_THUNDER], RES_BIG_ZOMBIE, 3000);
  now->dropRate = 100;
  initCommonSprite(now=&commonSprites[SPRITE_ORGRE], &weapons[WEAPON_MANY_AXES], RES_ORGRE, 3000);
  now->dropRate = 100;
  initCommonSprite(now=&commonSprites[SPRITE_BIG_DEMON], &weapons[WEAPON_THUNDER], RES_BIG_DEMON, 2500);
  now->dropRate = 100;
}
