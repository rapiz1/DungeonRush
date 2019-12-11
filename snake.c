#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "types.h"
#include "res.h"
#include "render.h"
#include "game.h"
#include "ui.h"

#ifdef DBG
#include <assert.h>
#endif

int main(int argc, char** args) {
  srand(time(NULL));
  // Start up SDL and create window
  if (!init()) {
    printf("Failed to initialize!\n");
  } else {
    // Load media
    if (!loadMedia()) {
      printf("Failed to load media!\n");
    } else {
      mainUi();
    }
  }
  cleanup();
}