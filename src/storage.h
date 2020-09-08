#ifndef SNAKE_STORAGE_H_
#define SNAKE_STORAGE_H_
#include <stdio.h>

#include "types.h"

#define STORAGE_PATH "storage.dat"
#define STORAGE_RANKLIST_NUM 10

void updateLocalRanklist(Score*);
Score** insertScoreToRanklist(Score*, int*, Score**);
void destroyRanklist(int n, Score** scores);
void writeRanklist(const char*, int, Score**);
Score** readRanklist(const char* path, int* n);
#endif
