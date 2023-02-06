#pragma once
#include "miniFB.h"
#include "windowData.h"

#define kCall(func, ...)  \
  if (windowData && windowData->func) \
    windowData->func ((struct sMiniFBwindow*)windowData, __VA_ARGS__);

//{{{
struct sMiniFBtimer {
  int64_t start_time;
  int64_t delta_counter;
  uint64_t time;
  };
//}}}

extern short int g_keycodes[512];
void keyboardDefault (struct sMiniFBwindow* window, mfb_key key, mfb_key_mod mod, bool isPressed);

void calcDstFactor (sWindowData* windowData, uint32_t width, uint32_t height);
void resizeDst (sWindowData* windowData, uint32_t width, uint32_t height);

void setTargetFpsAux();
