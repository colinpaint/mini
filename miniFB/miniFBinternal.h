#pragma once
#include "miniFB.h"
#include "sInfo.h"

#define kCall(func, ...)  \
  if (info && info->func) \
    info->func ((sOpaqueInfo*)info, __VA_ARGS__);

//{{{
struct sMiniFBtimer {
  int64_t start_time;
  int64_t delta_counter;
  uint64_t time;
  };
//}}}

extern short int gKeycodes[512];
void keyDefault (sOpaqueInfo* window, eKey key, eKeyModifier mod, bool isPressed);

void calcDstFactor (sInfo* info, uint32_t width, uint32_t height);
void resizeDst (sInfo* info, uint32_t width, uint32_t height);

void setTargetFpsAux();
