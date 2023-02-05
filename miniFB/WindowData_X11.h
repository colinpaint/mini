#pragma once
#include "MiniFB_enums.h"
#include <stdint.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

typedef struct {
  Window              window;
  Display             *display;
  int                 screen;
  GC                  gc;
  GLXContext          context;
  uint32_t            text_id;
  struct mfb_timer   *timer;
  } SWindowData_X11;
