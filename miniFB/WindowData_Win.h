#pragma once
#include "MiniFB_enums.h"
#include <windows.h>

typedef struct {
  HWND              window;
  WNDCLASS          wc;
  HDC               hdc;
  HGLRC             hGLRC;
  uint32_t          text_id;
  struct mfb_timer* timer;
  bool              mouse_inside;
  } SWindowData_Win;
