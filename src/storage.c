#include "storage.h"

#include <string.h>

void readScore(FILE* f, Score* score) {
  fscanf(f, "%d %d %d %d\n", &score->damage, &score->stand, &score->killed,
         &score->got);
  calcScore(score);
}
void writeScore(FILE* f, Score* score) {
  fprintf(f, "%d %d %d %d\n", score->damage, score->stand, score->killed,
          score->got);
}
void destroyRanklist(int n, Score** scores) {
  for (int i = 0; i < n; i++) {
    destroyScore(scores[i]);
  }
}
Score** insertScoreToRanklist(Score* score, int* n, Score** scores) {
  for (int i = 0; i < *n; i++)
    if (scores[i]->rank < score->rank) {
      if (*n < STORAGE_RANKLIST_NUM) {
        scores = realloc(scores, sizeof(Score*) * (++*n));
        scores[*n - 1] = createScore();
      }
      else destroyScore(scores[*n - 1]);
      for (int j = *n - 1; j > i ; j--) scores[j] = scores[j - 1];
      scores[i] = createScore();
      memcpy(scores[i], score, sizeof(Score));
      break;
    }
  return scores;
}

void writeRanklist(const char* path, int n, Score** scores) {
  FILE* f = fopen(path, "w");
  if (f == NULL) {
    fprintf(stderr, "writeRanklist: Can not create file\n");
    return;
  } 
  fprintf(f, "%d\n", n);
  for (int i = 0; i < n; i++) writeScore(f, scores[i]);
  fclose(f);
}
Score** readRanklist(const char* path, int* n) {
  FILE* f = fopen(path, "r");
  if (!f) {
      *n = 1;
      Score** scores = malloc(sizeof(Score*) * (*n));
      scores[0] = createScore();
      memset(scores[0], 0, sizeof(Score));
      return scores;
  }
  fscanf(f, "%d", n);
  Score** scores = malloc(sizeof(Score*) * (*n));
  for (int i = 0; i < *n; i++) {
    scores[i] = createScore();
    readScore(f, scores[i]);
  }
  fclose(f);
  return scores;
}
void updateLocalRanklist(Score* score) {
  int n;
  Score** scores = readRanklist(STORAGE_PATH, &n);
  scores = insertScoreToRanklist(score, &n, scores);
  writeRanklist(STORAGE_PATH, n, scores);
}
