#ifndef SNAKE_TYPES_
#define SNAKE_TYPES_
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

#define TEXT_LEN 1024
#define POSITION_BUFFER_SIZE 256
#define BUFF_BEGIN 0
#define BUFF_FROZEN 0
#define BUFF_SLOWDOWN 1
#define BUFF_DEFFENCE 2
#define BUFF_ATTACK 3
#define BUFF_END 4
// Renderer Types
typedef enum { LEFT, RIGHT, UP, DOWN } Direction;
typedef enum { AT_TOP_LEFT, AT_BOTTOM_LEFT, AT_BOTTOM_CENTER, AT_CENTER} At;
typedef enum {
  LOOP_ONCE,
  LOOP_INFI,
  LOOP_LIFESPAN
} LoopType;

typedef struct {
  SDL_Texture* origin;
  int height, width, frames;
  // SDL_Rect* crops;
  SDL_Rect* crops;
} Texture;
typedef struct {
  char text[TEXT_LEN];
  int height, width;
  SDL_Texture* origin;
  SDL_Color color;
} Text;
void setText(Text* self, const char* str);
typedef struct {
  int duration, currentFrame, length;
  SDL_Color* keys;
  SDL_BlendMode mode;
} Effect;
typedef struct {
  LoopType lp;
  Texture* origin;
  Effect* effect;
  int duration, currentFrame;
  int x, y;
  double angle;
  SDL_RendererFlip flip;
  At at;
  void* bind;
  bool scaled;
  bool strongBind;
  int lifeSpan;
} Animation;

void initTexture(Texture* self, SDL_Texture* origin, int width, int height,
                 int frames);
void destroyTexture(Texture* self);
bool initText(Text* self, const char* str, SDL_Color color);
Text* createText(const char* str, SDL_Color color);
void destroyText(Text* self);
void initEffect(Effect* self,  int duration, int length, SDL_BlendMode mode);
void destroyEffect(Effect* self);
void copyEffect(const Effect* src, Effect* dest);
void initAnimation(Animation* self, Texture* origin, const Effect* effect, LoopType lp, int duration, int x, int y, SDL_RendererFlip flip, double angle, At at);
Animation* createAnimation(Texture* origin, const Effect* effect, LoopType lp, int duration, int x, int y, SDL_RendererFlip flip, double angle, At at);
void destroyAnimation(Animation* self);
void copyAnimation(Animation* src, Animation* dest);

// ADT
struct _LinkNode {
  void* element;
  struct _LinkNode *pre, *nxt;
};
typedef struct _LinkNode LinkNode;
typedef struct {
  LinkNode *head, *tail;
} LinkList;
LinkNode* createLinkNode(void* element);
LinkList* createLinkList();
void initLinkList(LinkList* self);
void initLinkNode(LinkNode* self);
void destroyLinkList(LinkList* self);
void destroyAnimationsByLinkList(LinkList* list);
void removeAnimationFromLinkList(LinkList* list, Animation* ani);
void pushLinkNode(LinkList* self, LinkNode* node);
void pushLinkNodeAtHead(LinkList* self, LinkNode* node);
void removeLinkNode(LinkList* self, LinkNode* node);

// Game Logic Types
typedef enum {
  WEAPON_SWORD_POINT,
  WEAPON_SWORD_RANGE,
  WEAPON_GUN_RANGE,
  WEAPON_GUN_POINT,
  WEAPON_GUN_POINT_MULTI,
} WeaponType;
typedef struct {
  double chance;
  int duration;
} WeaponBuff;
typedef struct {
  WeaponType wp;
  int shootRange, effectRange, damage, gap, bulletSpeed;
  Animation *birthAni, *deathAni, *flyAni;
  int birthAudio, deathAudio;
  WeaponBuff effects[BUFF_END];
} Weapon;
typedef struct {int x, y; } Point;
typedef struct {
  int x, y;
  Direction direction;
} PositionBuffer;
typedef struct {
  int x, y, hp, totoalHp;
  Weapon* weapon;
  Animation* ani;
  Direction face;
  Direction direction;
  PositionBuffer buffer[POSITION_BUFFER_SIZE];
  int bufferSize;
  int lastAttack;
  double dropRate;
} Sprite;
void changeSpriteDirection(LinkNode*, Direction);
typedef struct {
  int damage, stand, killed, got;
  double rank;
} Score;
Score* createScore();
void calcScore(Score*);
void addScore(Score*, Score*);
void destroyScore(Score*);
typedef struct {
  LinkList* sprites;
  int moveStep, team, num, buffs[BUFF_END];
  Score* score;
} Snake;
void destroySnake(Snake*);
typedef struct {
  Weapon* parent;
  int x, y, team;
  Snake* owner;
  double rad;
  Animation* ani;
} Bullet;
Bullet* createBullet(Snake* owner,Weapon* parent, int x, int y, double rad, int team, Animation* ani);
void destroyBullet(Bullet* bullet);
void moveBullet(Bullet* bullet);

typedef enum {
  BLOCK_TRAP,
  BLOCK_WALL,
  BLOCK_FLOOR,
  BLOCK_EXIT
} BlockType;
typedef struct {
  BlockType bp;
  int x, y, bid;
  bool enable;
  Animation* ani;
} Block;
typedef enum {
  ITEM_NONE,
  ITEM_HERO,
  ITEM_HP_MEDCINE,
  ITEM_HP_EXTRA_MEDCINE,
  ITEM_WEAPON
} ItemType;
typedef struct {
  ItemType type;
  int id, belong;
  Animation* ani;
} Item;
#endif