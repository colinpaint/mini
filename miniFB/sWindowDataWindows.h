// sWindowDataWindows - specific is opaque pointer
#pragma once
#include "miniFBenums.h"
#include <windows.h>

struct sWindowDataWindows {
  HWND           window;
  WNDCLASS       wc;
  HDC            hdc;
  HGLRC          hGLRC;
  bool           pointerInside;
  };
