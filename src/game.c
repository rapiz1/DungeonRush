#include "game.h"

#include <SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ai.h"
#include "audio.h"
#include "bullet.h"
#include "helper.h"
#include "map.h"
#include "net.h"
#include "render.h"
#include "res.h"
#include "sprite.h"
#include "types.h"
#include "weapon.h"

#ifdef DBG
#include <assert.h>
#endif
extern const int SCALE_FACTOR;
extern const int n, m;

extern Texture textures[TEXTURES_SIZE];
const int MOVE_STEP = 2;

extern LinkList animationsList[];
extern Effect effects[];

extern Weapon weapons[WEAPONS_SIZE];
extern Sprite commonSprites[COMMON_SPRITE_SIZE];

extern unsigned int renderFrames;
// Map
Block map[MAP_SIZE][MAP_SIZE];
Item itemMap[MAP_SIZE][MAP_SIZE];
extern bool hasMap[MAP_SIZE][MAP_SIZE];
bool hasEnemy[MAP_SIZE][MAP_SIZE];
int spikeDamage = 1;
// Sprite
Snake* spriteSnake[SPRITES_MAX_NUM];
LinkList* bullets;

int gameLevel, stage;
int spritesCount, playersCount, flasksCount, herosCount, flasksSetting,
    herosSetting, spritesSetting, bossSetting;
// Win
int GAME_WIN_NUM;
int termCount;
bool willTerm;
int status;
// Drop rate
double GAME_LUCKY;
double GAME_DROPOUT_YELLOW_FLASKS;
double GAME_DROPOUT_WEAPONS;
double GAME_TRAP_RATE;
extern double AI_LOCK_LIMIT;
double GAME_MONSTERS_HP_ADJUST;
double GAME_MONSTERS_WEAPON_BUFF_ADJUST;
double GAME_MONSTERS_GEN_FACTOR;
void setLevel(int level) {
  gameLevel = level;
  spritesSetting = 25;
  bossSetting = 2;
  herosSetting = 8;
  flasksSetting = 6;
  GAME_LUCKY = 1;
  GAME_DROPOUT_YELLOW_FLASKS = 0.3;
  GAME_DROPOUT_WEAPONS = 0.7;
  GAME_TRAP_RATE = 0.005 * (level + 1);
  GAME_MONSTERS_HP_ADJUST = 1 + level * 0.8 + stage * level * 0.1;
  GAME_MONSTERS_GEN_FACTOR = 1 + level * 0.5 + stage * level * 0.05;
  GAME_MONSTERS_WEAPON_BUFF_ADJUST = 1 + level * 0.8 + stage * level * 0.02;
  AI_LOCK_LIMIT = MAX(1, 7 - level * 2 - stage / 2);
  GAME_WIN_NUM = 10 + level * 5 + stage * 3;
  if (level == 0) {
  } else if (level == 1) {
    GAME_DROPOUT_WEAPONS = 0.98;
    herosSetting = 5;
    flasksSetting = 4;
    spritesSetting = 28;
    bossSetting = 3;
  } else if (level == 2) {
    GAME_DROPOUT_WEAPONS = 0.98;
    GAME_DROPOUT_YELLOW_FLASKS = 0.3;
    spritesSetting = 28;
    herosSetting = 5;
    flasksSetting = 3;
    bossSetting = 5;
  }
  spritesSetting += stage / 2 * (level + 1);
  bossSetting += stage / 3;
}

Score** startGame(int localPlayers, int remotePlayers, bool localFirst) {
  Score** scores = malloc(sizeof(Score*) * localPlayers);
  for (int i = 0; i < localPlayers; i++) {
    scores[i] = createScore();
  }
  int status;
  stage = 0;
  do {
    initGame(localPlayers, remotePlayers, localFirst);
    setLevel(gameLevel);
    status = gameLoop();
    for (int i = 0; i < localPlayers; i++)
      addScore(scores[i], spriteSnake[i]->score);
    destroyGame(status);
    stage++;
  } while (status == 0);
  return scores;
}
void appendSpriteToSnake(
    Snake* snake, int sprite_id, int x, int y,
    Direction direcion) {  // x ,y, dir only matter when empty snake
  snake->num++;
  snake->score->got++;
  // at head
  LinkNode* node = malloc(sizeof(LinkNode));
  initLinkNode(node);

  // create a sprite
  Sprite* snakeHead = NULL;
  if (snake->sprites->head) {
    snakeHead = snake->sprites->head->element;
    x = snakeHead->x, y = snakeHead->y;
    int delta = (snakeHead->ani->origin->width * SCALE_FACTOR +
                 commonSprites[sprite_id].ani->origin->width * SCALE_FACTOR) /
                2;
    if (snakeHead->direction == LEFT)
      x -= delta;
    else if (snakeHead->direction == RIGHT)
      x += delta;
    else if (snakeHead->direction == UP)
      y -= delta;
    else
      y += delta;
  }
  Sprite* sprite = createSprite(&commonSprites[sprite_id], x, y);
  sprite->direction = direcion;
  if (direcion == LEFT) sprite->face = LEFT;
  if (snakeHead) {
    sprite->direction = snakeHead->direction;
    sprite->face = snakeHead->face;
    sprite->ani->currentFrame = snakeHead->ani->currentFrame;
  }
  // insert the sprite
  node->element = sprite;
  pushLinkNodeAtHead(snake->sprites, node);

  // push ani
  pushAnimationToRender(RENDER_LIST_SPRITE_ID, sprite->ani);

  // apply buff
  if (snake->buffs[BUFF_DEFFENCE])
    shieldSprite(sprite, snake->buffs[BUFF_DEFFENCE]);
}
void initPlayer(int playerType) {
  spritesCount++;
  Snake* p = spriteSnake[playersCount] =
      createSnake(MOVE_STEP, playersCount, playerType);
  appendSpriteToSnake(p, SPRITE_KNIGHT, SCREEN_WIDTH / 2,
                      SCREEN_HEIGHT / 2 + playersCount * 2 * UNIT, RIGHT);
  playersCount++;
}
void generateHeroItem(int x, int y) {
  int heroId = randInt(SPRITE_KNIGHT, SPRITE_LIZARD);
  Animation* ani = malloc(sizeof(Animation));
  itemMap[x][y] = (Item){ITEM_HERO, heroId, 0, ani};
  copyAnimation(commonSprites[heroId].ani, ani);
  x *= UNIT, y *= UNIT;
  // TODO:Dangerous
  ani->origin--;
  ani->x = x + UNIT / 2, ani->y = y + UNIT - 3;
  ani->at = AT_BOTTOM_CENTER;
  pushAnimationToRender(RENDER_LIST_SPRITE_ID, ani);
}
void generateItem(int x, int y, ItemType type) {
  int textureId = RES_FLASK_BIG_RED, id = 0, belong = SPRITE_KNIGHT;
  if (type == ITEM_HP_MEDCINE)
    textureId = RES_FLASK_BIG_RED;
  else if (type == ITEM_HP_EXTRA_MEDCINE)
    textureId = RES_FLASK_BIG_YELLOW;
  else if (type == ITEM_WEAPON) {
    int kind = randInt(0, 5);
    if (kind == 0) {
      textureId = RES_ICE_SWORD;
      id = WEAPON_ICE_SWORD;
      belong = SPRITE_KNIGHT;
    } else if (kind == 1) {
      textureId = RES_HOLY_SWORD;
      id = WEAPON_HOLY_SWORD;
      belong = SPRITE_KNIGHT;
    } else if (kind == 2) {
      textureId = RES_THUNDER_STAFF;
      id = WEAPON_THUNDER_STAFF;
      belong = SPRITE_WIZZARD;
    } else if (kind == 3) {
      textureId = RES_PURPLE_STAFF;
      id = WEAPON_PURPLE_STAFF;
      belong = SPRITE_WIZZARD;
    } else if (kind == 4) {
      textureId = RES_GRASS_SWORD;
      id = WEAPON_SOLID_CLAW;
      belong = SPRITE_LIZARD;
    } else if (kind == 5) {
      textureId = RES_POWERFUL_BOW;
      id = WEAPON_POWERFUL_BOW;
      belong = SPRITE_ELF;
    }
  }
  Animation* ani = createAndPushAnimation(
      &animationsList[RENDER_LIST_MAP_ITEMS_ID], &textures[textureId], NULL,
      LOOP_INFI, 3, x * UNIT, y * UNIT, SDL_FLIP_NONE, 0, AT_BOTTOM_LEFT);
  itemMap[x][y] = (Item){type, id, belong, ani};
}
void takeHpMedcine(Snake* snake, int delta, bool extra) {
  for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
    Sprite* sprite = p->element;
    if (sprite->hp == sprite->totalHp && !extra) continue;
    int addHp = (double)delta * sprite->totalHp / 100;
    if (!extra) addHp = MAX(0, MIN(sprite->totalHp - sprite->hp, addHp));
    sprite->hp += addHp;
    Animation* ani = createAndPushAnimation(
        &animationsList[RENDER_LIST_EFFECT_ID], &textures[RES_HP_MED], NULL,
        LOOP_ONCE, SPRITE_ANIMATION_DURATION, sprite->x, sprite->y,
        SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
    bindAnimationToSprite(ani, sprite, false);
  }
}
bool takeWeapon(Snake* snake, Item* weaponItem) {
  Weapon* weapon = &weapons[weaponItem->id];
  bool taken = false;
  for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
    Sprite* sprite = p->element;
    if (sprite->ani->origin == commonSprites[weaponItem->belong].ani->origin &&
        sprite->weapon == commonSprites[weaponItem->belong].weapon) {
      sprite->weapon = weapon;
      Animation* ani = createAndPushAnimation(
          &animationsList[RENDER_LIST_EFFECT_ID], weaponItem->ani->origin, NULL,
          LOOP_INFI, 3, sprite->x, sprite->y, SDL_FLIP_NONE, 0,
          AT_BOTTOM_CENTER);
      bindAnimationToSprite(ani, sprite, true);
      sprite->hp += GAME_HP_MEDICINE_EXTRA_DELTA / 100.0 * sprite->totalHp * 5;
      ani = createAndPushAnimation(&animationsList[RENDER_LIST_EFFECT_ID],
                                   &textures[RES_HP_MED], NULL, LOOP_ONCE,
                                   SPRITE_ANIMATION_DURATION, 0, 0,
                                   SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
      bindAnimationToSprite(ani, sprite, true);
      taken = true;
      break;
    }
  }
  return taken;
}
void dropItemNearSprite(Sprite* sprite, ItemType itemType) {
  for (int dx = -1; dx <= 1; dx++)
    for (int dy = -1; dy <= 1; dy++) {
      int x = sprite->x / UNIT + dx, y = sprite->y / UNIT + dy;
      if (inr(x, 0, n - 1) && inr(y, 0, m - 1) && hasMap[x][y] &&
          itemMap[x][y].type == ITEM_NONE) {
        generateItem(x, y, itemType);
      }
      return;
    }
}
void generateHeroItemAllMap() {
  int x, y;
  do {
    x = randInt(1, n - 2), y = randInt(1, m - 2);
  } while (!hasMap[x][y] || map[x][y].bp != BLOCK_FLOOR ||
           itemMap[x][y].type != ITEM_NONE ||
           !hasMap[x - 1][y] + !hasMap[x + 1][y] + !hasMap[x][y + 1] +
                   !hasMap[x][y - 1] >=
               1);
  generateHeroItem(x, y);
}
void clearItemMap() {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < m; j++) {
      itemMap[i][j].type = ITEM_NONE;
    }
}
void initItemMap(int hCount, int fCount) {
  int x, y;
  while (hCount--) {
    generateHeroItemAllMap();
    herosCount++;
  }
  while (fCount--) {
    do {
      x = randInt(0, n - 1), y = randInt(0, m - 1);
    } while (!hasMap[x][y] || map[x][y].bp != BLOCK_FLOOR ||
             itemMap[x][y].type != ITEM_NONE);
    generateItem(x, y, ITEM_HP_MEDCINE);
    flasksCount++;
  }
}
int generateEnemy(int x, int y, int minLen, int maxLen, int minId, int maxId,
                  int step) {
  Snake* snake = spriteSnake[spritesCount++] =
      createSnake(step, GAME_MONSTERS_TEAM, COMPUTER);
  hasEnemy[x][y] = 1;
  bool vertical = randInt(0, 1);
  int len = 1;
  if (vertical) {
    while (inr(y + len, 0, m - 1) && hasMap[x][y + len] &&
           map[x][y + len].bp == BLOCK_FLOOR &&
           itemMap[x][y + len].type == ITEM_NONE && !hasEnemy[x][y + len])
      len++;
  } else {
    while (inr(x + len, 0, n - 1) && hasMap[x + len][y] &&
           map[x + len][y].bp == BLOCK_FLOOR &&
           itemMap[x + len][y].type == ITEM_NONE && !hasEnemy[x + len][y])
      len++;
  }
  minLen = MIN(minLen, len);
  maxLen = MIN(maxLen, len);
  len = randInt(minLen, maxLen);
  for (int i = 0; i < len; i++) {
    int xx = x, yy = y;
    if (vertical)
      yy += i;
    else
      xx += i;
    hasEnemy[xx][yy] = 1;
    xx *= UNIT, yy *= UNIT;
    yy += UNIT;
    xx += UNIT / 2;
    int spriteId = randInt(minId, maxId);
    appendSpriteToSnake(snake, spriteId, xx, yy, vertical ? DOWN : RIGHT);
  }
  return len;
}
Point getAvaliablePos() {
  int x, y;
  do {
    x = randInt(0, n - 1), y = randInt(0, m - 1);
  } while (!hasMap[x][y] || map[x][y].bp != BLOCK_FLOOR ||
           itemMap[x][y].type != ITEM_NONE || hasEnemy[x][y] ||
           !hasMap[x - 1][y] + !hasMap[x + 1][y] + !hasMap[x][y + 1] +
                   !hasMap[x][y - 1] >=
               1);
  return (Point){x, y};
}
void initEnemies(int enemiesCount) {
  memset(hasEnemy, 0, sizeof hasEnemy);
  for (int i = -2; i <= 2; i++)
    for (int j = -2; j <= 2; j++) hasEnemy[n / 2 + i][m / 2 + j] = 1;
  for (int i = 0; i < enemiesCount;) {
    double random = randDouble() * GAME_MONSTERS_GEN_FACTOR;
    Point pos = getAvaliablePos();
    int x = pos.x, y = pos.y;
    int minLen = 2, maxLen = 4, step = 1;
    int startId = SPRITE_TINY_ZOMBIE, endId = SPRITE_TINY_ZOMBIE;
    // double random = i * GAME_MONSTERS_GEN_FACTOR / enemiesCount;
    if (random < 0.3) {
      startId = SPRITE_TINY_ZOMBIE;
      endId = SPRITE_SKELET;
    } else if (random < 0.4) {
      startId = SPRITE_WOGOL;
      endId = SPRITE_CHROT;
      step = 2;
    } else if (random < 0.5) {
      startId = SPRITE_ZOMBIE;
      endId = SPRITE_ICE_ZOMBIE;
    } else if (random < 0.8) {
      startId = SPRITE_MUDDY;
      endId = SPRITE_SWAMPY;
    } else {
      startId = SPRITE_MASKED_ORC;
      endId = SPRITE_NECROMANCER;
    }
    i += generateEnemy(x, y, minLen, maxLen, startId, endId, step);
  }
  for (int bossCount = 0; bossCount < bossSetting; bossCount++) {
    Point pos = getAvaliablePos();
    generateEnemy(pos.x, pos.y, 1, 1, SPRITE_BIG_ZOMBIE, SPRITE_BIG_DEMON, 1);
  }
}

/*
 * Put buff animation on snake
 */

void freezeSnake(Snake* snake, int duration) {
  if (snake->buffs[BUFF_FROZEN]) return;
  if (!snake->buffs[BUFF_DEFFENCE]) snake->buffs[BUFF_FROZEN] += duration;
  Effect* effect = NULL;
  if (snake->buffs[BUFF_DEFFENCE]) {
    effect = malloc(sizeof(Effect));
    copyEffect(&effects[EFFECT_VANISH30], effect);
    duration = 30;
  }
  for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
    Sprite* sprite = p->element;
    Animation* ani = createAndPushAnimation(
        &animationsList[RENDER_LIST_EFFECT_ID], &textures[RES_ICE], effect,
        LOOP_ONCE, duration, sprite->x, sprite->y, SDL_FLIP_NONE, 0,
        AT_BOTTOM_CENTER);
    ani->scaled = false;
    if (snake->buffs[BUFF_DEFFENCE]) continue;
    bindAnimationToSprite(ani, sprite, true);
  }
}

void slowDownSnake(Snake* snake, int duration) {
  if (snake->buffs[BUFF_SLOWDOWN]) return;
  if (!snake->buffs[BUFF_DEFFENCE]) snake->buffs[BUFF_SLOWDOWN] += duration;
  Effect* effect = NULL;
  if (snake->buffs[BUFF_DEFFENCE]) {
    effect = malloc(sizeof(Effect));
    copyEffect(&effects[EFFECT_VANISH30], effect);
    duration = 30;
  }
  for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
    Sprite* sprite = p->element;
    Animation* ani = createAndPushAnimation(
        &animationsList[RENDER_LIST_EFFECT_ID], &textures[RES_SOLIDFX], effect,
        LOOP_LIFESPAN, 40, sprite->x, sprite->y, SDL_FLIP_NONE, 0,
        AT_BOTTOM_CENTER);
    ani->lifeSpan = duration;
    ani->scaled = false;
    if (snake->buffs[BUFF_DEFFENCE]) continue;
    bindAnimationToSprite(ani, sprite, true);
  }
}

void shieldSprite(Sprite* sprite, int duration) {
  Animation* ani = createAndPushAnimation(
      &animationsList[RENDER_LIST_EFFECT_ID], &textures[RES_HOLY_SHIELD], NULL,
      LOOP_LIFESPAN, 40, sprite->x, sprite->y, SDL_FLIP_NONE, 0,
      AT_BOTTOM_CENTER);
  bindAnimationToSprite(ani, sprite, true);
  ani->lifeSpan = duration;
}

void shieldSnake(Snake* snake, int duration) {
  if (snake->buffs[BUFF_DEFFENCE]) return;
  snake->buffs[BUFF_DEFFENCE] += duration;
  for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
    shieldSprite(p->element, duration);
  }
}

void attackUpSprite(Sprite* sprite, int duration) {
  Animation* ani = createAndPushAnimation(
      &animationsList[RENDER_LIST_EFFECT_ID], &textures[RES_ATTACK_UP], NULL,
      LOOP_LIFESPAN, SPRITE_ANIMATION_DURATION, sprite->x, sprite->y,
      SDL_FLIP_NONE, 0, AT_BOTTOM_CENTER);
  bindAnimationToSprite(ani, sprite, true);
  ani->lifeSpan = duration;
}

void attackUpSnkae(Snake* snake, int duration) {
  if (snake->buffs[BUFF_ATTACK]) return;
  snake->buffs[BUFF_ATTACK] += duration;
  for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
    attackUpSprite(p->element, duration);
  }
}

/*
  Initialize and deinitialize game and snake
*/

void initGame(int localPlayers, int remotePlayers, bool localFirst) {
  randomBgm();
  status = 0;
  termCount = willTerm = 0;
  spritesCount = playersCount = flasksCount = herosCount = 0;
  initRenderer();
  initCountDownBar();

  // create default hero at (w/2, h/2) (as well push ani)
  for (int i = 0; i < localPlayers + remotePlayers; i++) {
    int playerType = LOCAL;
    if (localFirst)
      playerType = i < localPlayers ? LOCAL : REMOTE;
    else
      playerType = i < remotePlayers ? REMOTE : LOCAL;
    initPlayer(playerType);
    shieldSnake(spriteSnake[i], 300);
  }
  initInfo();
  // create map
  initRandomMap(0.7, 7, GAME_TRAP_RATE);

  clearItemMap();
  // create enemies
  initEnemies(spritesSetting);
  pushMapToRender();
  bullets = createLinkList();
}

void destroyGame(int status) {
  while (spritesCount) {
    destroySnake(spriteSnake[--spritesCount]);
    spriteSnake[spritesCount] = NULL;
  }
  for (int i = 0; i < ANIMATION_LINK_LIST_NUM; i++)
    destroyAnimationsByLinkList(&animationsList[i]);

  for (LinkNode* p = bullets->head; p; p = p->nxt) {
    destroyBullet(p->element);
    p->element = NULL;
  }

  destroyLinkList(bullets);
  bullets = NULL;

  blackout();
  char* msg;
  if (status == 0)
    msg = "Stage Clear";
  else
    msg = "Game Over";
  extern SDL_Color WHITE;
  Text* text = createText(msg, WHITE);
  renderCenteredText(text, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 2);
  destroyText(text);
  SDL_RenderPresent(renderer);
  sleep(RENDER_GAMEOVER_DURATION);
  clearRenderer();
}

void destroySnake(Snake* snake) {
  if (bullets)
    for (LinkNode* p = bullets->head; p; p = p->nxt) {
      Bullet* bullet = p->element;
      if (bullet->owner == snake) bullet->owner = NULL;
    }
  for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
    Sprite* sprite = p->element;
    free(sprite);
    p->element = NULL;
  }
  destroyLinkList(snake->sprites);
  snake->sprites = NULL;
  destroyScore(snake->score);
  snake->score = NULL;
  free(snake);
}

/*
  Helper function to determine whehter a snake is a player
*/
inline bool isPlayer(Snake* snake) {
  for (int i = 0; i < playersCount; i++)
    if (snake == spriteSnake[i]) return true;
  return false;
}

/*
  Verdict if a sprite crushes on other objects
*/
bool crushVerdict(Sprite* sprite, bool loose, bool useAnimationBox) {
  int x = sprite->x, y = sprite->y;
  SDL_Rect block, box = useAnimationBox ? getSpriteAnimationBox(sprite)
                                        : getSpriteFeetBox(sprite);

  // If the sprite is out of the map, then consider it as crushed
  if (inr(x / UNIT, 0, n - 1) && inr(y / UNIT, 0, m - 1))
    ;
  else
    return true;
  // Loop over the cells nearby the sprite to know better if it falls out of map
  for (int dx = -1; dx <= 1; dx++)
    for (int dy = -1; dy <= 1; dy++) {
      int xx = x / UNIT + dx, yy = y / UNIT + dy;
      if (inr(xx, 0, n - 1) && inr(yy, 0, m - 1)) {
        block = getMapRect(xx, yy);
        if (RectRectCross(&box, &block) && !hasMap[xx][yy]) {
          return true;
        }
      }
    }

  // If it has crushed on other sprites
  for (int i = 0; i < spritesCount; i++) {
    bool self = false;
    for (LinkNode* p = spriteSnake[i]->sprites->head; p; p = p->nxt) {
      Sprite* other = p->element;
      if (other != sprite) {
        SDL_Rect otherBox = useAnimationBox ? getSpriteAnimationBox(other)
                                            : getSpriteFeetBox(other);
        if (RectRectCross(&box, &otherBox)) {
          if ((self && loose) || (p->pre && p->pre->element == sprite))
            ;
          else
            return true;
        }
      } else
        self = true;
    }
  }
  return false;
}

void dropItem(Sprite* sprite) {
  double random = randDouble() * sprite->dropRate * GAME_LUCKY;
#ifdef DBG
// printf("%lf\n", random);
#endif
  if (random < GAME_DROPOUT_YELLOW_FLASKS)
    dropItemNearSprite(sprite, ITEM_HP_EXTRA_MEDCINE);
  else if (random > GAME_DROPOUT_WEAPONS)
    dropItemNearSprite(sprite, ITEM_WEAPON);
}

void invokeWeaponBuff(Snake* src, Weapon* weapon, Snake* dest, int damage) {
  double random;
  for (int i = BUFF_BEGIN; i < BUFF_END; i++) {
    random = randDouble();
    if (src && src->team == GAME_MONSTERS_TEAM)
      random *= GAME_MONSTERS_WEAPON_BUFF_ADJUST;
    if (random < weapon->effects[i].chance) switch (i) {
        case BUFF_FROZEN:
          freezeSnake(dest, weapon->effects[i].duration);
          break;
        case BUFF_SLOWDOWN:
          slowDownSnake(dest, weapon->effects[i].duration);
          break;
        case BUFF_DEFFENCE:
          if (src) shieldSnake(src, weapon->effects[i].duration);
          break;
        case BUFF_ATTACK:
          if (src) attackUpSnkae(src, weapon->effects[i].duration);
          break;
        default:
          break;
      }
  }
}

void dealDamage(Snake* src, Snake* dest, Sprite* target, int damage) {
  double calcDamage = damage;
  if (dest->buffs[BUFF_FROZEN]) calcDamage *= GAME_FROZEN_DAMAGE_K;
  if (src && src != spriteSnake[GAME_MONSTERS_TEAM]) {
    if (src->buffs[BUFF_ATTACK]) calcDamage *= GAME_BUFF_ATTACK_K;
  }
  if (dest != spriteSnake[GAME_MONSTERS_TEAM]) {
    if (dest->buffs[BUFF_DEFFENCE]) calcDamage /= GAME_BUFF_DEFENSE_K;
  }
  target->hp -= calcDamage;
  if (src) {
    src->score->damage += calcDamage;
    if (target->hp <= 0) src->score->killed++;
  }
  dest->score->stand += damage;
}

bool makeSnakeCross(Snake* snake) {
  if (!snake->sprites->head) return false;
  // Trap and Item ( everything related to block ) verdict
  for (int i = 0; i < SCREEN_WIDTH / UNIT; i++)
    for (int j = 0; j < SCREEN_HEIGHT / UNIT; j++)
      if (hasMap[i][j]) {
        SDL_Rect block = {i * UNIT, j * UNIT, UNIT, UNIT};
        if (map[i][j].bp == BLOCK_TRAP && map[i][j].enable) {
          for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
            Sprite* sprite = p->element;
            SDL_Rect spriteRect = getSpriteFeetBox(sprite);
            if (RectRectCross(&spriteRect, &block)) {
              dealDamage(NULL, snake, sprite, spikeDamage);
            }
          }
        }
        if (isPlayer(snake)) {
          SDL_Rect headBox = getSpriteFeetBox(snake->sprites->head->element);
          if (itemMap[i][j].type != ITEM_NONE) {
            if (RectRectCross(&headBox, &block)) {
              bool taken = true;
              Animation* ani = itemMap[i][j].ani;
              if (itemMap[i][j].type == ITEM_HERO) {
                playAudio(AUDIO_COIN);
                appendSpriteToSnake(snake, itemMap[i][j].id, 0, 0, RIGHT);
                herosCount--;
                removeAnimationFromLinkList(
                    &animationsList[RENDER_LIST_SPRITE_ID], ani);
              } else if (itemMap[i][j].type == ITEM_HP_MEDCINE ||
                         itemMap[i][j].type == ITEM_HP_EXTRA_MEDCINE) {
                playAudio(AUDIO_MED);
                takeHpMedcine(snake, GAME_HP_MEDICINE_DELTA,
                              itemMap[i][j].type == ITEM_HP_EXTRA_MEDCINE);
                flasksCount -= itemMap[i][j].type == ITEM_HP_MEDCINE;

                removeAnimationFromLinkList(
                    &animationsList[RENDER_LIST_MAP_ITEMS_ID], ani);
              } else if (itemMap[i][j].type == ITEM_WEAPON) {
                taken = takeWeapon(snake, &itemMap[i][j]);
                if (taken) {
                  playAudio(AUDIO_MED);
                  removeAnimationFromLinkList(
                      &animationsList[RENDER_LIST_MAP_ITEMS_ID], ani);
                }
              }
              if (taken) itemMap[i][j].type = ITEM_NONE;
            }
          }
        }
      }
  // Death verdict, create death ani
  for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
    Sprite* sprite = p->element;
    if (sprite->hp <= 0) {
      playAudio(AUDIO_HUMAN_DEATH);
      Texture* death = sprite->ani->origin;
      if (isPlayer(snake)) death++;
      dropItem(sprite);
      createAndPushAnimation(
          &animationsList[RENDER_LIST_DEATH_ID], &textures[RES_SKULL], NULL,
          LOOP_INFI, 1,
          sprite->x + randInt(-MAP_SKULL_SPILL_RANGE, MAP_SKULL_SPILL_RANGE),
          sprite->y + randInt(-MAP_SKULL_SPILL_RANGE, MAP_SKULL_SPILL_RANGE),
          sprite->face == LEFT ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL, 0,
          AT_BOTTOM_CENTER);
      createAndPushAnimation(
          &animationsList[RENDER_LIST_DEATH_ID], death, &effects[EFFECT_DEATH],
          LOOP_ONCE, SPRITE_ANIMATION_DURATION, sprite->x, sprite->y,
          sprite->face == RIGHT ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL, 0,
          AT_BOTTOM_CENTER);
      /* TOO BLOODY
  createAndPushAnimation(
      &animationsList[RENDER_LIST_MAP_SPECIAL_ID], &textures[randInt(RES_BLOOD1,
  RES_BLOOD4)],NULL , LOOP_INFI, SPRITE_ANIMATION_DURATION, sprite->x +
  randInt(-MAP_BLOOD_SPILL_RANGE, MAP_BLOOD_SPILL_RANGE), sprite->y +
  randInt(-MAP_BLOOD_SPILL_RANGE, MAP_BLOOD_SPILL_RANGE), sprite->face == RIGHT
  ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL, 0, AT_BOTTOM_CENTER);
      */
      clearBindInAnimationsList(sprite, RENDER_LIST_EFFECT_ID);
      clearBindInAnimationsList(sprite, RENDER_LIST_SPRITE_ID);
      removeAnimationFromLinkList(&animationsList[RENDER_LIST_SPRITE_ID],
                                  sprite->ani);
      sprite->ani = NULL;
      snake->num--;
    }
  }
  // Update position
  for (LinkNode *p = snake->sprites->head, *nxt; p; p = nxt) {
    Sprite* sprite = p->element;
    nxt = p->nxt;
    if (sprite->hp <= 0) {
      for (LinkNode* q = snake->sprites->tail; q != p; q = q->pre) {
        Sprite *prevSprite = q->pre->element, *sprite = q->element;
        sprite->direction = prevSprite->direction;
        sprite->face = prevSprite->face;
        sprite->posBuffer = prevSprite->posBuffer;
        sprite->x = prevSprite->x;
        sprite->y = prevSprite->y;
      }
      removeLinkNode(snake->sprites, p);
      free(sprite);
    }
  }
  if (!snake->sprites->head) return false;
  Sprite* snakeHead = snake->sprites->head->element;
  bool die = crushVerdict(snakeHead, !isPlayer(snake), false);
  if (die) {
    for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
      Sprite* sprite = p->element;
      sprite->hp = 0;
    }
  }

  return die;
}
bool makeBulletCross(Bullet* bullet) {
  Weapon* weapon = bullet->parent;
  bool hit = false;
  int width = MIN(bullet->ani->origin->width, bullet->ani->origin->height) *
              (bullet->ani->scaled ? SCALE_FACTOR : 1) * 0.8;
  SDL_Rect bulletBox = {bullet->x - width / 2, bullet->y - width / 2, width,
                        width};
  if (!hasMap[bullet->x / UNIT][bullet->y / UNIT]) {
    Animation* ani = malloc(sizeof(Animation));
    copyAnimation(weapon->deathAni, ani);
    ani->x = bullet->x, ani->y = bullet->y;
    pushAnimationToRender(RENDER_LIST_EFFECT_ID, ani);

    hit = true;
  }
  if (!hit)
    for (int i = 0; i < spritesCount; i++)
      if (bullet->team != spriteSnake[i]->team) {
        for (LinkNode* p = spriteSnake[i]->sprites->head; p; p = p->nxt) {
          Sprite* target = p->element;
          SDL_Rect box = getSpriteBoundBox(target);
          if (RectRectCross(&box, &bulletBox)) {
            Animation* ani = malloc(sizeof(Animation));
            copyAnimation(weapon->deathAni, ani);
            ani->x = bullet->x, ani->y = bullet->y;
            pushAnimationToRender(RENDER_LIST_EFFECT_ID, ani);
            hit = true;
            if (weapon->wp == WEAPON_GUN_POINT ||
                weapon->wp == WEAPON_GUN_POINT_MULTI) {
              dealDamage(bullet->owner, spriteSnake[i], target, weapon->damage);

              invokeWeaponBuff(bullet->owner, weapon, spriteSnake[i],
                               weapon->damage);
              return hit;
            }
            break;
          }
        }
      }
  if (hit) {
    playAudio(weapon->deathAudio);
    for (int i = 0; i < spritesCount; i++)
      if (bullet->team != spriteSnake[i]->team) {
        for (LinkNode* p = spriteSnake[i]->sprites->head; p; p = p->nxt) {
          Sprite* target = p->element;
          if (distance((Point){target->x, target->y},
                       (Point){bullet->x, bullet->y}) <= weapon->effectRange) {
            dealDamage(bullet->owner, spriteSnake[i], target, weapon->damage);
            invokeWeaponBuff(bullet->owner, weapon, spriteSnake[i],
                             weapon->damage);
          }
        }
      }
  }
  return hit;
}
void makeCross() {
  for (int i = 0; i < spritesCount; i++) {
    makeSnakeCross(spriteSnake[i]);
  }
  for (LinkNode *p = bullets->head, *nxt; p; p = nxt) {
    nxt = p->nxt;
    if (makeBulletCross(p->element)) {
      Bullet* bullet = p->element;
      removeAnimationFromLinkList(&animationsList[RENDER_LIST_EFFECT_ID],
                                  bullet->ani);
      removeLinkNode(bullets, p);
    }
  }
}
void moveSprite(Sprite* sprite, int step) {
  Direction dir = sprite->direction;
  if (dir == LEFT)
    sprite->x -= step;
  else if (dir == RIGHT)
    sprite->x += step;
  else if (dir == UP)
    sprite->y -= step;
  else if (dir == DOWN)
    sprite->y += step;
}
void moveSnake(Snake* snake) {
  if (snake->buffs[BUFF_FROZEN]) return;
  int step = snake->moveStep;
  if (snake->buffs[BUFF_SLOWDOWN]) step = MAX(step / 2, 1);
  for (LinkNode* p = snake->sprites->head; p; p = p->nxt) {
    Sprite* sprite = p->element;
    for (int i = 0; i < step; i++) {
      PositionBuffer* b = &sprite->posBuffer;
      PositionBufferSlot firstSlot = b->buffer[0];
      while (b->size && sprite->x == firstSlot.x && sprite->y == firstSlot.y) {
        changeSpriteDirection(p, firstSlot.direction);
        b->size--;
        for (int i = 0; i < b->size; i++) b->buffer[i] = b->buffer[i + 1];

        firstSlot = b->buffer[0];
      }
      moveSprite(sprite, 1);
    }
  }
}
void updateMap() {
  int maskedTime = renderFrames % SPIKE_TIME_MASK;
  for (int i = 0; i < SCREEN_WIDTH / UNIT; i++)
    for (int j = 0; j < SCREEN_HEIGHT / UNIT; j++) {
      if (hasMap[i][j] && map[i][j].bp == BLOCK_TRAP) {
        if (!maskedTime)
          createAndPushAnimation(&animationsList[RENDER_LIST_MAP_SPECIAL_ID],
                                 &textures[RES_FLOOR_SPIKE_OUT_ANI], NULL,
                                 LOOP_ONCE, SPIKE_ANI_DURATION, i * UNIT,
                                 j * UNIT, SDL_FLIP_NONE, 0, AT_TOP_LEFT);
        else if (maskedTime == SPIKE_ANI_DURATION - 1) {
          map[i][j].enable = true;
          map[i][j].ani->origin = &textures[RES_FLOOR_SPIKE_ENABLED];
        } else if (maskedTime == SPIKE_ANI_DURATION + SPIKE_OUT_INTERVAL - 1) {
          createAndPushAnimation(&animationsList[RENDER_LIST_MAP_SPECIAL_ID],
                                 &textures[RES_FLOOR_SPIKE_IN_ANI], NULL,
                                 LOOP_ONCE, SPIKE_ANI_DURATION, i * UNIT,
                                 j * UNIT, SDL_FLIP_NONE, 0, AT_TOP_LEFT);
          map[i][j].enable = false;
          map[i][j].ani->origin = &textures[RES_FLOOR_SPIKE_DISABLED];
        }
      }
    }
}
void updateBuffDuration() {
  for (int i = 0; i < spritesCount; i++) {
    Snake* snake = spriteSnake[i];
    for (int j = BUFF_BEGIN; j < BUFF_END; j++)
      if (snake->buffs[j] > 0) snake->buffs[j]--;
  }
}
void makeSpriteAttack(Sprite* sprite, Snake* snake) {
  Weapon* weapon = sprite->weapon;
  if (renderFrames - sprite->lastAttack < weapon->gap) {
    return;
  }
  bool attacked = false;
  for (int i = 0; i < spritesCount; i++)
    if (snake->team != spriteSnake[i]->team) {
      for (LinkNode* p = spriteSnake[i]->sprites->head; p; p = p->nxt) {
        Sprite* target = p->element;
        if (distance((Point){sprite->x, sprite->y},
                     (Point){target->x, target->y}) > weapon->shootRange)
          continue;
        double rad = atan2(target->y - sprite->y, target->x - sprite->x);
        if (weapon->wp == WEAPON_SWORD_POINT ||
            weapon->wp == WEAPON_SWORD_RANGE) {
          Animation* ani = malloc(sizeof(Animation));
          copyAnimation(weapon->deathAni, ani);
          // ani->x = target->x, ani->y = target->y;
          bindAnimationToSprite(ani, target, false);
          if (ani->angle != -1) ani->angle = rad * 180 / PI;
          pushAnimationToRender(RENDER_LIST_EFFECT_ID, ani);
          dealDamage(snake, spriteSnake[i], target, weapon->damage);
          invokeWeaponBuff(snake, weapon, spriteSnake[i], weapon->damage);
          attacked = true;
          if (weapon->wp == WEAPON_SWORD_POINT) goto ATTACK_END;
        } else {
          Bullet* bullet = createBullet(snake, weapon, sprite->x, sprite->y,
                                        rad, snake->team, weapon->flyAni);
          pushLinkNode(bullets, createLinkNode(bullet));
          pushAnimationToRender(RENDER_LIST_EFFECT_ID, bullet->ani);
          attacked = true;
          if (weapon->wp != WEAPON_GUN_POINT_MULTI) goto ATTACK_END;
        }
      }
    }
ATTACK_END:
  if (attacked) {
    if (weapon->birthAni) {
      Animation* ani = malloc(sizeof(Animation));
      copyAnimation(weapon->birthAni, ani);
      bindAnimationToSprite(ani, sprite, true);
      ani->at = AT_BOTTOM_CENTER;
      pushAnimationToRender(RENDER_LIST_EFFECT_ID, ani);
    }
    if (weapon->wp == WEAPON_SWORD_POINT || weapon->wp == WEAPON_SWORD_RANGE)
      playAudio(weapon->deathAudio);
    else
      playAudio(weapon->birthAudio);
    sprite->lastAttack = renderFrames;
  }
}
void makeSnakeAttack(Snake* snake) {
  if (snake->buffs[BUFF_FROZEN]) return;
  for (LinkNode* p = snake->sprites->head; p; p = p->nxt)
    makeSpriteAttack(p->element, snake);
}
bool isWin() {
  if (playersCount != 1) return false;
  return spriteSnake[0]->num >= GAME_WIN_NUM;
}

typedef enum { STAGE_CLEAR, GAME_OVER } GameStatus;
void setTerm(GameStatus s) {
  stopBgm();
  if (s == 0)
    playAudio(AUDIO_WIN);
  else
    playAudio(AUDIO_LOSE);
  status = s;
  willTerm = true;
  termCount = RENDER_TERM_COUNT;
}
void pauseGame() {
  pauseSound();
  playAudio(AUDIO_BUTTON1);
  dim();
  const char msg[] = "Paused";
  extern SDL_Color WHITE;
  Text* text = createText(msg, WHITE);
  renderCenteredText(text, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 1);
  SDL_RenderPresent(renderer);
  destroyText(text);
  SDL_Event e;
  for (bool quit = 0; !quit;) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN) {
        quit = true;
        break;
      }
    }
  }
  resumeSound();
  playAudio(AUDIO_BUTTON1);
}

int arrowsToDirection(int keyValue) {
  switch (keyValue) {
    case SDLK_LEFT:
      return LEFT;
      break;
    case SDLK_RIGHT:
      return RIGHT;
      break;
    case SDLK_UP:
      return UP;
      break;
    case SDLK_DOWN:
      return DOWN;
      break;
  }
  return -1;
}

int wasdToDirection(int keyValue) {
  switch (keyValue) {
    case SDLK_a:
      return LEFT;
      break;
    case SDLK_d:
      return RIGHT;
      break;
    case SDLK_w:
      return UP;
      break;
    case SDLK_s:
      return DOWN;
      break;
  }
  return -1;
}

bool handleLocalKeypress() {
  static SDL_Event e;
  bool quit = false;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      quit = true;
      setTerm(GAME_OVER);
    } else if (e.type == SDL_KEYDOWN) {
      int keyValue = e.key.keysym.sym;
      if (keyValue == SDLK_ESCAPE) pauseGame();
      for (int id = 0; id <= 1 && id < playersCount; id++) {
        Snake* player = spriteSnake[id];
        if (player->playerType == LOCAL) {
          if (!player->buffs[BUFF_FROZEN] && player->sprites->head != NULL) {
            int direction = id == 0 ? arrowsToDirection(keyValue)
                                    : wasdToDirection(keyValue);
            if (direction >= 0) {
              sendPlayerMovePacket(id, direction);
              changeSpriteDirection(player->sprites->head, direction);
            }
          }
        }
      }
    }
  }
  return quit;
}

void handleLanKeypress() {
  static LanPacket packet;
  int status = recvLanPacket(&packet);
  if (!status) return;  // nop
  unsigned type = packet.type;
  if (type == HEADER_PLAYERMOVE) {
    PlayerMovePacket* playerMovePacket = (PlayerMovePacket*)(&packet);
    Snake* player = spriteSnake[playerMovePacket->playerId];
    int direction = playerMovePacket->direction;
    fprintf(stderr, "recv: player move, %d, %d\n", playerMovePacket->playerId,
            direction);
    if (player->sprites->head)
      changeSpriteDirection(player->sprites->head, direction);
  } else if (type == HEADER_GAMEOVER) {
    fprintf(stderr, "recv: game over, %d\n", -1);
    setTerm(GAME_OVER);
  }
}

int gameLoop() {
  // int posx = 0, posy = SCREEN_HEIGHT / 2;
  // Game loop
  for (bool quit = 0; !quit;) {
    quit = handleLocalKeypress();
    if (quit) sendGameOverPacket(3);
    if (lanClientSocket != NULL) handleLanKeypress();

    updateMap();

    for (int i = 0; i < spritesCount; i++) {
      if (!spriteSnake[i]->sprites->head)
        continue;  // some snakes killed by before but not clean up yet
      if (i >= playersCount && renderFrames % AI_DECIDE_RATE == 0)
        AiInput(spriteSnake[i]);
      moveSnake(spriteSnake[i]);
      makeSnakeAttack(spriteSnake[i]);
    }
    for (LinkNode* p = bullets->head; p; p = p->nxt) moveBullet(p->element);
    if (renderFrames % GAME_MAP_RELOAD_PERIOD == 0)
      initItemMap(herosSetting - herosCount, flasksSetting - flasksCount);
    for (int i = 0; i < spritesCount; i++) {
      updateAnimationOfSnake(spriteSnake[i]);
      if (spriteSnake[i]->buffs[BUFF_FROZEN])
        for (LinkNode* p = spriteSnake[i]->sprites->head; p; p = p->nxt) {
          Sprite* sprite = p->element;
          sprite->ani->currentFrame--;
        }
    }
    makeCross();
    render();
    updateBuffDuration();
    for (int i = playersCount; i < spritesCount; i++) {
      if (!spriteSnake[i]->num) {
        destroySnake(spriteSnake[i]);
        spriteSnake[i] = NULL;
        for (int j = i; j + 1 < spritesCount; j++)
          spriteSnake[j] = spriteSnake[j + 1];
        spriteSnake[spritesCount--] = NULL;
      }
    }
    if (willTerm) {
      termCount--;
      if (!termCount) break;
    } else {
      int alivePlayer = -1;
      for (int i = 0; i < playersCount; i++) {
        if (!spriteSnake[i]->sprites->head) {
          setTerm(GAME_OVER);
          sendGameOverPacket(alivePlayer);
          break;
        } else
          alivePlayer = i;
      }
      if (isWin()) {
        setTerm(STAGE_CLEAR);
      }
    }
  }
  return status;
}
