#include "weapon.h"

#include "render.h"
#include "res.h"
#include "types.h"

#include <stdio.h>

extern Texture textures[RES_HALO_EXPLOSION2];
extern Mix_Chunk* sounds[AUDIO_SOUND_SIZE];

Weapon weapons[WEAPONS_SIZE];
void initWeapon(Weapon* self, int birthTextureId, int deathTextureId,
                int flyTextureId) {
  Animation* birthAni = NULL;
  Animation* deathAni = NULL;
  Animation* flyAni = NULL;
  if (birthTextureId != -1) {
    birthAni = createAnimation(&textures[birthTextureId], NULL, LOOP_ONCE,
                               SPRITE_ANIMATION_DURATION, 0, 0, SDL_FLIP_NONE,
                               0, AT_CENTER);
  }
  if (deathTextureId != -1) {
    deathAni = createAnimation(&textures[deathTextureId], NULL, LOOP_ONCE,
                               SPRITE_ANIMATION_DURATION, 0, 0, SDL_FLIP_NONE,
                               0, AT_BOTTOM_CENTER);
  }
  if (flyTextureId != -1) {
    flyAni = createAnimation(&textures[flyTextureId], NULL, LOOP_INFI,
                             SPRITE_ANIMATION_DURATION, 0, 0, SDL_FLIP_NONE, 0,
                             AT_CENTER);
  }
  *self = (Weapon){
      WEAPON_SWORD_POINT, 32 * 2, 40, 10, 60, 6, birthAni, deathAni, flyAni, -1,
      AUDIO_CLAW_HIT};
}
void initWeapons() {
  Weapon* now;

  initWeapon(now = &weapons[WEAPON_SWORD], -1, RES_SwordFx, -1);
  now->damage = 30;
  now->shootRange = 32 * 3;
  now->deathAni->scaled = false;
  now->deathAni->angle = -1;
  now->deathAudio = AUDIO_SWORD_HIT;

  initWeapon(now = &weapons[WEAPON_MONSTER_CLAW], -1, RES_CLAWFX2, -1);
  now->wp = WEAPON_SWORD_RANGE;
  now->shootRange = 32 * 3 + 16;
  now->damage = 24;
  now->deathAni->angle = -1;
  now->deathAni->at = AT_CENTER;
  now->deathAudio = AUDIO_CLAW_HIT_HEAVY;

  initWeapon(now = &weapons[WEAPON_FIREBALL], RES_Shine, RES_HALO_EXPLOSION1,
             RES_FIREBALL);
  now->wp = WEAPON_GUN_RANGE;
  now->damage = 45;
  now->effectRange = 50;
  now->shootRange = 256;
  now->gap = 180;
  now->deathAni->angle = -1;
  now->deathAni->at = AT_CENTER;
  now->birthAni->duration = 24;
  now->birthAudio = AUDIO_SHOOT;
  now->deathAudio = AUDIO_FIREBALL_EXP;

  initWeapon(now = &weapons[WEAPON_THUNDER], RES_BLOOD_BOUND, RES_Thunder, -1);
  now->wp = WEAPON_SWORD_RANGE;
  now->damage = 80;
  now->shootRange = 128;
  now->gap = 120;
  now->deathAni->angle = -1;
  now->deathAni->scaled = false;
  now->deathAudio = AUDIO_THUNDER;

  initWeapon(now = &weapons[WEAPON_THUNDER_STAFF], -1, RES_THUNDER_YELLOW, -1);
  now->wp = WEAPON_SWORD_RANGE;
  now->damage = 50;
  now->shootRange = 128;
  now->gap = 120;
  now->deathAni->angle = -1;
  now->deathAni->scaled = false;
  now->deathAudio = AUDIO_THUNDER;

  initWeapon(now = &weapons[WEAPON_ARROW], -1, RES_HALO_EXPLOSION2, RES_ARROW);
  now->wp = WEAPON_GUN_POINT;
  now->gap = 40;
  now->damage = 10;
  now->shootRange = 200;
  now->bulletSpeed = 10;
  now->deathAni->angle = -1;
  now->deathAni->at = AT_CENTER;
  now->flyAni->scaled = false;
  now->birthAudio = AUDIO_LIGHT_SHOOT;
  now->deathAudio = AUDIO_ARROW_HIT;

  initWeapon(now = &weapons[WEAPON_POWERFUL_BOW], -1, RES_HALO_EXPLOSION2,
             RES_ARROW);
  now->wp = WEAPON_GUN_POINT;
  now->gap = 60;
  now->damage = 25;
  now->shootRange = 320;
  now->bulletSpeed = 7;
  now->deathAni->angle = -1;
  now->deathAni->at = AT_CENTER;
  now->birthAudio = AUDIO_LIGHT_SHOOT;
  now->deathAudio = AUDIO_ARROW_HIT;
  now->effects[BUFF_ATTACK] = (WeaponBuff){0.5, 240};

  initWeapon(now = &weapons[WEAPON_MONSTER_CLAW2], -1, RES_CLAWFX, -1);

  initWeapon(now = &weapons[WEAPON_THROW_AXE], -1, RES_CROSS_HIT, RES_AXE);
  now->wp = WEAPON_GUN_POINT;
  now->damage = 12;
  now->shootRange = 160;
  now->bulletSpeed = 10;
  now->flyAni->duration = 24;
  now->flyAni->angle = -1;
  now->flyAni->scaled = false;
  now->deathAni->scaled = false;
  now->deathAni->at = AT_CENTER;
  now->birthAudio = AUDIO_LIGHT_SHOOT;
  now->deathAudio = AUDIO_ARROW_HIT;

  initWeapon(now = &weapons[WEAPON_MANY_AXES], -1, RES_CROSS_HIT, RES_AXE);
  now->wp = WEAPON_GUN_POINT_MULTI;
  now->shootRange = 180;
  now->gap = 70;
  now->effectRange = 50;
  now->damage = 50;
  now->bulletSpeed = 4;
  now->flyAni->duration = 24;
  now->flyAni->angle = -1;
  now->deathAni->at = AT_CENTER;
  now->birthAudio = AUDIO_LIGHT_SHOOT;
  now->deathAudio = AUDIO_ARROW_HIT;

  initWeapon(now = &weapons[WEAPON_SOLID], -1, RES_SOLIDFX, -1);
  now->deathAni->scaled = false;
  now->deathAni->angle = -1;
  now->effects[BUFF_SLOWDOWN] = (WeaponBuff){0.3, 180};

  initWeapon(now = &weapons[WEAPON_SOLID_GREEN], -1, RES_SOLID_GREENFX, -1);
  now->shootRange = 96;
  now->deathAni->scaled = false;
  now->deathAni->angle = -1;
  now->effects[BUFF_SLOWDOWN] = (WeaponBuff){0.3, 180};

  initWeapon(now = &weapons[WEAPON_SOLID_CLAW], -1, RES_SOLID_GREENFX, -1);
  now->wp = WEAPON_SWORD_RANGE;
  now->shootRange = 32 * 3 + 16;
  now->damage = 35;
  now->deathAni->scaled = false;
  now->deathAni->angle = -1;
  now->deathAudio = AUDIO_CLAW_HIT_HEAVY;
  now->effects[BUFF_SLOWDOWN] = (WeaponBuff){0.7, 60};

  initWeapon(now = &weapons[WEAPON_ICEPICK], -1, RES_ICESHATTER, RES_ICEPICK);
  now->wp = WEAPON_GUN_RANGE;
  now->damage = 30;
  now->effectRange = 50;
  now->shootRange = 256;
  now->gap = 180;
  now->bulletSpeed = 8;
  now->deathAni->angle = -1;
  now->flyAni->scaled = false;
  now->deathAni->at = AT_CENTER;
  now->effects[BUFF_FROZEN] = (WeaponBuff){0.2, 60};
  now->birthAudio = AUDIO_ICE_SHOOT;

  initWeapon(now = &weapons[WEAPON_PURPLE_BALL], -1, RES_PURPLE_EXP,
             RES_PURPLE_BALL);
  now->wp = WEAPON_GUN_RANGE;
  now->damage = 20;
  now->effectRange = 50;
  now->shootRange = 256;
  now->gap = 100;
  now->bulletSpeed = 6;
  now->deathAni->angle = -1;
  now->deathAni->scaled = false;
  now->flyAni->scaled = false;
  now->deathAni->at = AT_CENTER;
  now->birthAudio = AUDIO_ICE_SHOOT;
  now->deathAudio = AUDIO_ARROW_HIT;

  initWeapon(now = &weapons[WEAPON_PURPLE_STAFF], -1, RES_PURPLE_EXP,
             RES_PURPLE_BALL);
  now->wp = WEAPON_GUN_POINT_MULTI;
  now->damage = 45;
  now->effectRange = 50;
  now->shootRange = 256;
  now->gap = 100;
  now->bulletSpeed = 7;
  now->deathAni->angle = -1;
  now->deathAni->scaled = false;
  now->flyAni->scaled = false;
  now->deathAni->at = AT_CENTER;
  now->birthAudio = AUDIO_ICE_SHOOT;
  now->deathAudio = AUDIO_ARROW_HIT;

  initWeapon(now = &weapons[WEAPON_HOLY_SWORD], -1, RES_GOLDEN_CROSS_HIT, -1);
  now->wp = WEAPON_SWORD_RANGE;
  now->damage = 30;
  now->shootRange = 32 * 4;
  now->effects[BUFF_DEFFENCE] = (WeaponBuff){0.6, 180};

  initWeapon(now = &weapons[WEAPON_ICE_SWORD], -1, RES_ICESHATTER, -1);
  now->wp = WEAPON_SWORD_RANGE;
  now->shootRange = 32 * 3 + 16;
  now->damage = 80;
  now->gap = 30;
  now->deathAni->angle = -1;
  now->deathAni->at = AT_CENTER;
  now->effects[BUFF_FROZEN] = (WeaponBuff){0.6, 80};
  now->deathAudio = AUDIO_SWORD_HIT;

#ifdef DBG
  puts("weapon done");
#endif
}
