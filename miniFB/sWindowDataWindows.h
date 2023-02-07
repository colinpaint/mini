#pragma once
#include "miniFBenums.h"
#include <windows.h>

struct sWindowDataWindows {
  HWND              window;
  WNDCLASS          wc;
  HDC               hdc;
  HGLRC             hGLRC;
  uint32_t          text_id;
  struct sMiniFBtimer* timer;
  bool              mouse_inside;
  };
