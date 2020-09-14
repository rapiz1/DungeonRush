#ifndef SNAKE_GAME_H_
#define SNAKE_GAME_H_
#include "player.h"
#include "sprite.h"
#include "types.h"

// Control
#define SPRITES_MAX_NUM 1024
#define KEYPRESS_DELTA 17
#define MAX_PALYERS_NUM 2

// Game
// Spike
#define SPIKE_TIME_MASK 600
#define SPIKE_OUT_INTERVAL 120
#define SPIKE_ANI_DURATION 20
#define SPRITE_EFFECT_DELTA 20
// Bounder Box
#define BIG_SPRITE_EFFECT_DELTA 25
#define SPRITE_EFFECT_VERTICAL_DELTA 6
#define SPRITE_EFFECT_FEET 12
#define GAME_BULLET_EFFECTIVE 16
// Team
#define GAME_MONSTERS_TEAM 9
// Buff
#define GAME_HP_MEDICINE_DELTA 55
#define GAME_HP_MEDICINE_EXTRA_DELTA 33
#define GAME_MAP_RELOAD_PERIOD 120
#define GAME_BUFF_ATTACK_K 2.5
#define GAME_BUFF_DEFENSE_K 2
#define GAME_FROZEN_DAMAGE_K 0.1
// Drop Rate
// Win

void pushMapToRender();
Score** startGame(int localPlayers, int remotePlayers, bool localFirst);
void initGame(int localPlayers, int remotePlayers, bool localFirst);
void destroyGame(int);
int gameLoop();
void updateAnimationOfSprite(Sprite* self);
void updateAnimationOfBlock(Block* self);
// this shuold only be used to create once or permant animation
Animation* createAndPushAnimation(LinkList* list, Texture* texture,
                                  const Effect* effect, LoopType lp,
                                  int duration, int x, int y,
                                  SDL_RendererFlip flip, double angle, At at);
bool isPlayer(Snake* snake);
bool crushVerdict(Sprite* sprite, bool loose, bool useAnimationBox);
void moveSprite(Sprite*, int);
void moveSnake(Snake* snake);
void shieldSprite(Sprite* sprite, int duration);
void appendSpriteToSnake(Snake* snake, int sprite_id, int x, int y,
                         Direction direcion);
void setLevel(int level);
#endif
