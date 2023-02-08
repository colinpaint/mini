// sWindowData - sMiniWindow is opaque pointer
#pragma once
#include "miniFBenums.h"
#include <functional>

struct sWindowData {
  void* specific;
  void* user_data;

  activeFuncType         activeFunc;
  resizeFuncType         resizeFunc;
  closeFuncType          closeFunc;
  keyFuncType            keyFunc;
  charFuncType           charFunc;
  pointerButtonFuncType  pointer_buttonFunc;
  pointerMoveFuncType    pointer_moveFunc;
  pointerWheelFuncType   pointer_wheelFunc;
  pointerEnterFuncType   pointerEnterFunc;

  uint32_t window_width;
  uint32_t window_height;

  uint32_t dst_offset_x;
  uint32_t dst_offset_y;
  uint32_t dst_width;
  uint32_t dst_height;

  float    factor_x;
  float    factor_y;
  float    factor_width;
  float    factor_height;

  void*    draw_buffer;
  uint32_t bufferWidth;
  uint32_t bufferHeight;
  uint32_t bufferStride;

  int32_t  pointerPosX;
  int32_t  pointerPosY;
  int32_t  pointerPressure;
  int32_t  timestamp;

  float    pointerWheelX;
  float    pointerWheelY;
  uint8_t  pointerButtonStatus[8];

  bool     isActive;
  bool     isInitialized;
  bool     closed;

  uint8_t  keyStatus[512];
  uint32_t modifierKeys;

  uint32_t textureId;
  sMiniFBtimer* timer;
  };
