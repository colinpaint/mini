#pragma once
#include "miniFBenums.h"
#include <stdint.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

struct sWindowDataX11 {
  Window      window;
  Display*    display;
  int         screen;
  GC          gc;
  GLXContext  context;
  uint32_t    text_id;
  struct sMiniFBtimer* timer;
  } ;
