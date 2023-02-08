// sWindowData - sMiniWindow is opaque pointer to this
#pragma once
#include "miniFBenums.h"
#include <functional>

struct sWindowData {
  void* specific;
  void* user_data;

  mfb_active_func         active_func;
  mfb_resize_func         resize_func;
  mfb_close_func          close_func;
  mfb_key_func            key_func;
  mfb_char_func           char_func;
  mfb_pointer_button_func pointer_button_func;
  mfb_pointer_move_func   pointer_move_func;
  mfb_pointer_wheel_func  pointer_wheel_func;
  pointerEnterFuncType    pointerEnterFunc;

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
  uint32_t buffer_width;
  uint32_t buffer_height;
  uint32_t buffer_stride;

  int32_t  pointerPosX;
  int32_t  pointerPosY;
  int32_t  pointerPressure;
  int32_t  timestamp;

  float    pointerWheelX;
  float    pointerWheelY;
  uint8_t  pointerButtonStatus[8];

  bool     is_active;
  bool     is_initialized;

  bool     close;

  uint8_t  key_status[512];
  uint32_t mod_keys;

  uint32_t    text_id;
  sMiniFBtimer* timer;
  };
