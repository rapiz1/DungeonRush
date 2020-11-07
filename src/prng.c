#include "prng.h"

// from https://stackoverflow.com/questions/15500621/c-c-algorithm-to-produce-same-pseudo-random-number-sequences-from-same-seed-on

static unsigned int nSeed = 5323;

void prngSrand(unsigned seed) {
  nSeed = seed;
}

unsigned int prngRand()
{
    // our initial starting seed is 5323

    // Take the current seed and generate a new value from it
    // Due to our use of large constants and overflow, it would be
    // very hard for someone to predict what the next number is
    // going to be from the previous one.
    nSeed = (8253729 * nSeed + 2396403); 

    // Take the seed and return a value between 0 and 32767
    return nSeed  % PRNG_MAX;
}
