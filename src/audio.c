#include "audio.h"

#include "helper.h"
#include "res.h"

extern const int bgmNums;
extern Mix_Music *bgms[AUDIO_BGM_SIZE];
extern Mix_Chunk *sounds[AUDIO_SOUND_SIZE];

int nowBgmId = -1;
void playBgm(int id) {
  if (nowBgmId == id) return;
  if (nowBgmId == -1)
    Mix_PlayMusic(bgms[id], -1);
  else
    Mix_FadeInMusic(bgms[id], -1, BGM_FADE_DURATION);
  nowBgmId = id;
}
void stopBgm() {
  Mix_FadeOutMusic(BGM_FADE_DURATION);
  nowBgmId = -1;
}
void randomBgm() { playBgm(randInt(1, bgmNums - 1)); }
void playAudio(int id) {
  if (id >= 0) Mix_PlayChannel(-1, sounds[id], 0);
}
void pauseSound() {
  Mix_Pause(-1);
  Mix_PauseMusic();
}
void resumeSound() {
  Mix_Resume(-1);
  Mix_ResumeMusic();
}
