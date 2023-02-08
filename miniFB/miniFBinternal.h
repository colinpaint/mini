#pragma once
#include "miniFB.h"
#include "sWindowData.h"

#define kCall(func, ...)  \
  if (windowData && windowData->func) \
    windowData->func ((sMiniWindow*)windowData, __VA_ARGS__);

//{{{
struct sMiniFBtimer {
  int64_t start_time;
  int64_t delta_counter;
  uint64_t time;
  };
//}}}

extern short int gKeycodes[512];
void keyDefault (sMiniWindow* window, mfb_key key, eKeyModifier mod, bool isPressed);

void calcDstFactor (sWindowData* windowData, uint32_t width, uint32_t height);
void resizeDst (sWindowData* windowData, uint32_t width, uint32_t height);

void setTargetFpsAux();
