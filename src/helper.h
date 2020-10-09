#ifndef SNAKE_HELPER_H
#define SNAKE_HELPER_H

#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>

#include "sprite.h"
#include "types.h"

#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x > y ? y : x)
#define PI 3.1415926535
#define HELPER_RECT_CROSS_LIMIT 8
int randInt(int l, int r);
// return random double in [0, 1]
double randDouble();
bool inr(int x, int l, int r);
bool IntervalCross(int, int, int, int);
bool RectRectCross(SDL_Rect*, SDL_Rect*);
bool RectCirCross(SDL_Rect*, int, int, int);
SDL_Rect getSpriteBoundBox(Sprite* sprite);
SDL_Rect getSpriteFeetBox(Sprite* sprite);
SDL_Rect getMapRect(int x, int y);
SDL_Rect getSpriteAnimationBox(Sprite* sprite);
int RectRectCalc(SDL_Rect* a, SDL_Rect* b);
double distance(Point a, Point b);

#endif
