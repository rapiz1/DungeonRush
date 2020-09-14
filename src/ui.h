#ifndef SNAKE_UI_H_
#define SNAKE_UI_H_
#include <SDL.h>
#include "types.h"
#define UI_MAIN_GAP 40
#define UI_MAIN_GAP_ALT 22
int chooseOptions(int optsNum, Text** options);
void baseUi(int,int);
void mainUi();
void rankListUi(int,Score**);
void localRankListUi();
#endif
