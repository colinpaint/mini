// sWindowDataX11 - specific is opaque pointer
#pragma once
#include <X11/Xlib.h>
#include <GL/glx.h>

struct sInfoX11 {
  Window     window;
  Display*   display;
  int        screen;
  GC         gc;
  GLXContext context;
  };
