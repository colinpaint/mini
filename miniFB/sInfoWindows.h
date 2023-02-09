// sInfoWindows - specific is opaque pointer
#pragma once
#include <windows.h>

struct sInfoWindows {
  HWND           window;
  WNDCLASS       wc;
  HDC            hdc;
  HGLRC          hGLRC;
  };
