// miniFBx11.cpp
//{{{  includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

// I cannot find a way to get dpi under VirtualBox
//#include <X11/Xresource.h>
//#include <X11/extensions/Xrandr.h>
#include <xkbcommon/xkbcommon.h>
#include <X11/extensions/XInput.h>

#include "miniFB.h"

#include "../common/cLog.h"
//}}}

extern void stretchImage (uint32_t* srcImage, uint32_t srcX, uint32_t srcY,
                          uint32_t srcWidth, uint32_t srcHeight, uint32_t srcPitch,
                          uint32_t* dstImage, uint32_t dstX, uint32_t dstY,
                          uint32_t dstWidth, uint32_t dstHeight, uint32_t dstPitch);

extern double gTimeForFrame;
extern bool gUseHardwareSync;
extern short int gKeycodes[512];

namespace {
  Atom gDeleteWindowAtom;
  XDevice* gDevice = nullptr;
  uint32_t gMotionType = 0;
  uint32_t gNumEventClasses = 0;
  XEventClass gEventClasses[16];
  int32_t gRangeX = 0;
  int32_t gRangeY = 0;
  int32_t gMaxPressure = 0;

  //{{{
  int translateKeyCodeB (int keySym) {

    switch (keySym) {
      case XK_KP_0:           return KB_KEY_KP_0;
      case XK_KP_1:           return KB_KEY_KP_1;
      case XK_KP_2:           return KB_KEY_KP_2;
      case XK_KP_3:           return KB_KEY_KP_3;
      case XK_KP_4:           return KB_KEY_KP_4;
      case XK_KP_5:           return KB_KEY_KP_5;
      case XK_KP_6:           return KB_KEY_KP_6;
      case XK_KP_7:           return KB_KEY_KP_7;
      case XK_KP_8:           return KB_KEY_KP_8;
      case XK_KP_9:           return KB_KEY_KP_9;
      case XK_KP_Separator:
      case XK_KP_Decimal:     return KB_KEY_KP_DECIMAL;
      case XK_KP_Equal:       return KB_KEY_KP_EQUAL;
      case XK_KP_Enter:       return KB_KEY_KP_ENTER;
      }

    return KB_KEY_UNKNOWN;
    }
  //}}}
  //{{{
  int translateKeyCodeA (int keySym) {

    switch (keySym) {
      //{{{  control keys
      case XK_Escape:         return KB_KEY_ESCAPE;
      case XK_Tab:            return KB_KEY_TAB;
      case XK_Shift_L:        return KB_KEY_LEFT_SHIFT;
      case XK_Shift_R:        return KB_KEY_RIGHT_SHIFT;
      case XK_Control_L:      return KB_KEY_LEFT_CONTROL;
      case XK_Control_R:      return KB_KEY_RIGHT_CONTROL;

      case XK_Meta_L:
      case XK_Alt_L:          return KB_KEY_LEFT_ALT;
      case XK_Mode_switch:      // Mapped to Alt_R on many keyboards
      case XK_ISO_Level3_Shift: // AltGr on at least some machines
      case XK_Meta_R:
      case XK_Alt_R:          return KB_KEY_RIGHT_ALT;

      case XK_Super_L:        return KB_KEY_LEFT_SUPER;
      case XK_Super_R:        return KB_KEY_RIGHT_SUPER;

      case XK_Menu:           return KB_KEY_MENU;

      case XK_Num_Lock:       return KB_KEY_NUM_LOCK;
      case XK_Caps_Lock:      return KB_KEY_CAPS_LOCK;

      case XK_Print:          return KB_KEY_PRINT_SCREEN;
      case XK_Scroll_Lock:    return KB_KEY_SCROLL_LOCK;
      case XK_Pause:          return KB_KEY_PAUSE;

      case XK_Delete:         return KB_KEY_DELETE;
      case XK_BackSpace:      return KB_KEY_BACKSPACE;
      case XK_Return:         return KB_KEY_ENTER;

      case XK_Home:           return KB_KEY_HOME;
      case XK_End:            return KB_KEY_END;

      case XK_Page_Up:        return KB_KEY_PAGE_UP;
      case XK_Page_Down:      return KB_KEY_PAGE_DOWN;

      case XK_Insert:         return KB_KEY_INSERT;
      case XK_Left:           return KB_KEY_LEFT;
      case XK_Right:          return KB_KEY_RIGHT;
      case XK_Down:           return KB_KEY_DOWN;
      case XK_Up:             return KB_KEY_UP;
      //}}}
      //{{{  function keys
      case XK_F1:             return KB_KEY_F1;
      case XK_F2:             return KB_KEY_F2;
      case XK_F3:             return KB_KEY_F3;
      case XK_F4:             return KB_KEY_F4;
      case XK_F5:             return KB_KEY_F5;
      case XK_F6:             return KB_KEY_F6;
      case XK_F7:             return KB_KEY_F7;
      case XK_F8:             return KB_KEY_F8;
      case XK_F9:             return KB_KEY_F9;

      case XK_F10:            return KB_KEY_F10;
      case XK_F11:            return KB_KEY_F11;
      case XK_F12:            return KB_KEY_F12;
      case XK_F13:            return KB_KEY_F13;
      case XK_F14:            return KB_KEY_F14;
      case XK_F15:            return KB_KEY_F15;
      case XK_F16:            return KB_KEY_F16;
      case XK_F17:            return KB_KEY_F17;
      case XK_F18:            return KB_KEY_F18;
      case XK_F19:            return KB_KEY_F19;

      case XK_F20:            return KB_KEY_F20;
      case XK_F21:            return KB_KEY_F21;
      case XK_F22:            return KB_KEY_F22;
      case XK_F23:            return KB_KEY_F23;
      case XK_F24:            return KB_KEY_F24;
      case XK_F25:            return KB_KEY_F25;
      //}}}
      //{{{  Numeric keypad
      case XK_KP_Divide:      return KB_KEY_KP_DIVIDE;
      case XK_KP_Multiply:    return KB_KEY_KP_MULTIPLY;
      case XK_KP_Subtract:    return KB_KEY_KP_SUBTRACT;
      case XK_KP_Add:         return KB_KEY_KP_ADD;
      //}}}
      //{{{  These should have been detected in secondary keysym test above!
      case XK_KP_Insert:      return KB_KEY_KP_0;
      case XK_KP_End:         return KB_KEY_KP_1;
      case XK_KP_Down:        return KB_KEY_KP_2;
      case XK_KP_Page_Down:   return KB_KEY_KP_3;
      case XK_KP_Left:        return KB_KEY_KP_4;
      case XK_KP_Right:       return KB_KEY_KP_6;
      case XK_KP_Home:        return KB_KEY_KP_7;
      case XK_KP_Up:          return KB_KEY_KP_8;
      case XK_KP_Page_Up:     return KB_KEY_KP_9;
      case XK_KP_Delete:      return KB_KEY_KP_DECIMAL;
      case XK_KP_Equal:       return KB_KEY_KP_EQUAL;
      case XK_KP_Enter:       return KB_KEY_KP_ENTER;
      //}}}
      //{{{  Last resort: Check for printable keys (should not happen if the XK extension is available)
      // This will give a layout dependent mapping (which is wrong, and we may miss some keys
      // especially on non-US keyboards), but it's better than nothing...
      case XK_a:              return KB_KEY_A;
      case XK_b:              return KB_KEY_B;
      case XK_c:              return KB_KEY_C;
      case XK_d:              return KB_KEY_D;
      case XK_e:              return KB_KEY_E;
      case XK_f:              return KB_KEY_F;
      case XK_g:              return KB_KEY_G;
      case XK_h:              return KB_KEY_H;
      case XK_i:              return KB_KEY_I;
      case XK_j:              return KB_KEY_J;
      case XK_k:              return KB_KEY_K;
      case XK_l:              return KB_KEY_L;
      case XK_m:              return KB_KEY_M;
      case XK_n:              return KB_KEY_N;
      case XK_o:              return KB_KEY_O;
      case XK_p:              return KB_KEY_P;
      case XK_q:              return KB_KEY_Q;
      case XK_r:              return KB_KEY_R;
      case XK_s:              return KB_KEY_S;
      case XK_t:              return KB_KEY_T;
      case XK_u:              return KB_KEY_U;
      case XK_v:              return KB_KEY_V;
      case XK_w:              return KB_KEY_W;
      case XK_x:              return KB_KEY_X;
      case XK_y:              return KB_KEY_Y;
      case XK_z:              return KB_KEY_Z;

      case XK_1:              return KB_KEY_1;
      case XK_2:              return KB_KEY_2;
      case XK_3:              return KB_KEY_3;
      case XK_4:              return KB_KEY_4;
      case XK_5:              return KB_KEY_5;
      case XK_6:              return KB_KEY_6;
      case XK_7:              return KB_KEY_7;
      case XK_8:              return KB_KEY_8;
      case XK_9:              return KB_KEY_9;
      case XK_0:              return KB_KEY_0;

      case XK_space:          return KB_KEY_SPACE;
      case XK_minus:          return KB_KEY_MINUS;
      case XK_equal:          return KB_KEY_EQUAL;
      case XK_bracketleft:    return KB_KEY_LEFT_BRACKET;
      case XK_bracketright:   return KB_KEY_RIGHT_BRACKET;
      case XK_backslash:      return KB_KEY_BACKSLASH;
      case XK_semicolon:      return KB_KEY_SEMICOLON;
      case XK_apostrophe:     return KB_KEY_APOSTROPHE;
      case XK_grave:          return KB_KEY_GRAVE_ACCENT;
      case XK_comma:          return KB_KEY_COMMA;
      case XK_period:         return KB_KEY_PERIOD;
      case XK_slash:          return KB_KEY_SLASH;
      case XK_less:           return KB_KEY_WORLD_1; // At least in some layouts...
      //}}}
      default: break;
      }

    return KB_KEY_UNKNOWN;
    }
  //}}}

  //{{{
  void initKeycodes (cInfo* info) {

    // clear keys
    for (size_t i = 0; i < sizeof(gKeycodes) / sizeof(gKeycodes[0]); ++i)
      gKeycodes[i] = KB_KEY_UNKNOWN;

    // valid key code range is  [8,255], according to the Xlib manual
    for (size_t i = 8; i <= 255; ++i) {
      // try secondary keysym, for numeric keypad keys
      int keySym  = XkbKeycodeToKeysym (info->display, i, 0, 1);
      gKeycodes[i] = translateKeyCodeB (keySym);
      if (gKeycodes[i] == KB_KEY_UNKNOWN) {
        keySym = XkbKeycodeToKeysym (info->display, i, 0, 0);
        gKeycodes[i] = translateKeyCodeA (keySym);
        }
      }
    }
  //}}}
  //{{{
  int translateKey (int scancode) {

    if (scancode < 0 || scancode > 255)
      return KB_KEY_UNKNOWN;

    return gKeycodes[scancode];
    }
  //}}}
  //{{{
  int translateMod (int state) {

    int modifierKeys = 0;

    if (state & ShiftMask)
      modifierKeys |= KB_MOD_SHIFT;
    if (state & ControlMask)
      modifierKeys |= KB_MOD_CONTROL;
    if (state & Mod1Mask)
      modifierKeys |= KB_MOD_ALT;
    if (state & Mod4Mask)
      modifierKeys |= KB_MOD_SUPER;
    if (state & LockMask)
      modifierKeys |= KB_MOD_CAPS_LOCK;
    if (state & Mod2Mask)
      modifierKeys |= KB_MOD_NUM_LOCK;

    return modifierKeys;
    }
  //}}}
  //{{{
  int translateModEx (int key, int state, int is_pressed) {

    int modifierKeys = translateMod (state);

    switch (key) {
      case KB_KEY_LEFT_SHIFT:
      //{{{
      case KB_KEY_RIGHT_SHIFT:
        if (is_pressed)
          modifierKeys |= KB_MOD_SHIFT;
        else
          modifierKeys &= ~KB_MOD_SHIFT;
        break;
      //}}}

      case KB_KEY_LEFT_CONTROL:
      //{{{
      case KB_KEY_RIGHT_CONTROL:
        if (is_pressed)
          modifierKeys |= KB_MOD_CONTROL;
        else
          modifierKeys &= ~KB_MOD_CONTROL;
        break;
      //}}}

      case KB_KEY_LEFT_ALT:
      //{{{
      case KB_KEY_RIGHT_ALT:
        if (is_pressed)
          modifierKeys |= KB_MOD_ALT;
        else
          modifierKeys &= ~KB_MOD_ALT;
        break;
      //}}}

      case KB_KEY_LEFT_SUPER:
      //{{{
      case KB_KEY_RIGHT_SUPER:
        if (is_pressed)
          modifierKeys |= KB_MOD_SUPER;
        else
          modifierKeys &= ~KB_MOD_SUPER;
        break;
      //}}}
      }

    return modifierKeys;
    }
  //}}}

  //{{{
  void processEvent (cInfo* info, XEvent* event) {

    switch (event->type) {
      case KeyPress:
      //{{{
      case KeyRelease:
        info->keyCode = (eKey)translateKey (event->xkey.keycode);
        info->isDown = (event->type == KeyPress);
        info->modifierKeys = translateModEx (info->keyCode, event->xkey.state, info->isDown);
        info->keyStatus[info->keyCode] = info->isDown;
        if (info->keyFunc)
          info->keyFunc (info);

        if (info->isDown) {
          KeySym keysym;
          XLookupString (&event->xkey, NULL, 0, &keysym, NULL);
          if ((keysym >= 0x0020 && keysym <= 0x007e) ||
              (keysym >= 0x00a0 && keysym <= 0x00ff)) {
            info->codepoint = keysym;
            if (info->charFunc)
              info->charFunc (info);
            }
          else if ((keysym & 0xff000000) == 0x01000000)
            keysym = keysym & 0x00ffffff;

          info->codepoint = keysym;
          if (info->charFunc)
            info->charFunc (info);
          // This does not seem to be working properly
          // unsigned int codepoint = xkb_state_key_get_utf32(state, keysym);
          // if (codepoint != 0)
          //    kCall(char_inputFunc, codepoint);
          }

        break;
      //}}}

      case ButtonPress:
      //{{{
      case ButtonRelease:
        {
        info->isDown = (event->type == ButtonPress);
        info->modifierKeys = translateMod (event->xkey.state);

        // swap 2 & 3 ?
        ePointerButton button = (ePointerButton)event->xbutton.button;
        switch (button) {
          case Button1:
          case Button3:
            info->pointerButtonStatus[button & 0x07] = info->isDown;
            if (info->buttonFunc)
              info->buttonFunc (info);
            break;

          // wheel
          case Button4:
            info->pointerWheelY = 1.0f;
            if (info->wheelFunc)
              info->wheelFunc (info);
            break;

          case Button5:
            info->pointerWheelY = -1.0f;
            if (info->wheelFunc)
              info->wheelFunc (info);
            break;

          case 6:
            info->pointerWheelX = 1.0f;
            if (info->wheelFunc)
              info->wheelFunc (info);
            break;

          case 7:
            info->pointerWheelX = -1.0f;
            if (info->wheelFunc)
              info->wheelFunc (info);
            break;

          default:
            break;
          }
        }

        break;
      //}}}

      //{{{
      case MotionNotify: {
        XDeviceMotionEvent* motionEvent = (XDeviceMotionEvent*)(event);
        cLog::log (LOGINFO, fmt::format ("motionNotify {},{} {}",
                                         event->xmotion.x, event->xmotion.y, motionEvent->serial));
        info->pointerTimestamp = 0;
        info->pointerPosX = event->xmotion.x;
        info->pointerPosY = event->xmotion.y;
        info->pointerPressure = info->pointerButtonStatus[Button1] * 1024;
        if (info->moveFunc)
          info->moveFunc (info);
        }
        break;
      //}}}

      //{{{
      case ConfigureNotify:
        info->window_width  = event->xconfigure.width;
        info->window_height = event->xconfigure.height;
        info->windowScaledWidth  = info->window_width;
        info->windowScaledHeight = info->window_height;

        info->resizeDst (event->xconfigure.width, event->xconfigure.height);
        resizeGL (info);

        if (info->resizeFunc)
          info->resizeFunc (info);

        break;
      //}}}

      //{{{
      case EnterNotify:
        info->pointerInside = true;
        if (info->activeFunc)
          info->activeFunc (info);

        break;
      //}}}
      //{{{
      case LeaveNotify:
        info->pointerInside = false;
        if (info->activeFunc)
          info->activeFunc (info);

        break;
      //}}}

      //{{{
      case FocusIn:
        info->isActive = true;
        if (info->activeFunc)
          info->activeFunc (info);

        break;
      //}}}
      //{{{
      case FocusOut:
        info->isActive = false;
        if (info->activeFunc)
          info->activeFunc (info);

        break;
      //}}}

      //{{{
      case DestroyNotify:
        info->closed = true;
        return;
      //}}}
      //{{{
      case ClientMessage:
        if ((Atom)event->xclient.data.l[0] == gDeleteWindowAtom) {
          if (info) {
            bool destroy = false;
            if (!info->closeFunc || info->closeFunc (info))
              destroy = true;
            if (destroy) {
              info->closed = true;
              return;
              }
            }
          }

        break;
      //}}}

      default:
        if (event->type == (int)gMotionType) {
          XDeviceMotionEvent* motionEvent = (XDeviceMotionEvent*)(event);
          int posX = motionEvent->x;
          int posY = motionEvent->y;
          //int posX = motionEvent->axis_data[0];
          //int posY = motionEvent->axis_data[1];
          int pressure = motionEvent->axis_data[2];
          cLog::log (LOGINFO, fmt::format ("tablet motionEvent {},{} {} {}",
                                           posX, posY, pressure, motionEvent->serial, event->type));
          }
        else
          cLog::log (LOGINFO, fmt::format ("unused event {}", event->type));
        break;
      }
    }
  //}}}
  //{{{
  void processEvents (cInfo* info) {

    while (!info->closed && XPending (info->display)) {
      XEvent event;
      XNextEvent (info->display, &event);
      processEvent (info, &event);
      }
    }
  //}}}
  //{{{
  void freeResources (cInfo* info)  {

    if (gDevice)
      XCloseDevice (info->display, gDevice);

    if (info) {
      destroyGLcontext (info);
      free (info);
      }
    }
  //}}}
  }

// interface
//{{{
cInfo* openEx (const char* title, unsigned width, unsigned height, unsigned flags) {

  cInfo* info = new cInfo();
  if (!info) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("failed to create window data"));
    return 0;
    }
    //}}}

  info->display = XOpenDisplay (0);
  if (!info->display) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("failed to create display"));
    delete (info);
    return 0;
    }
    //}}}

  initKeycodes (info);
  XAutoRepeatOff (info->display);

  info->screen = DefaultScreen (info->display);
  Visual* visual = DefaultVisual (info->display, info->screen);
  //{{{  set format
  int formatCount;
  int convDepth = -1;
  XPixmapFormatValues* formats = XListPixmapFormats (info->display, &formatCount);
  int depth = DefaultDepth (info->display, info->screen);
  Window defaultRootWindow = DefaultRootWindow (info->display);
  for (int i = 0; i < formatCount; ++i) {
    if (depth == formats[i].depth) {
      convDepth = formats[i].bits_per_pixel;
      break;
      }
    }
  XFree (formats);

  // We only support 32-bit right now
  if (convDepth != 32) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("failed to create 32 bir depth"));
    XCloseDisplay (info->display);
    return 0;
    }
    //}}}
  //}}}
  //{{{  set width, height
  int screenWidth = DisplayWidth (info->display, info->screen);
  int screenHeight = DisplayHeight (info->display, info->screen);

  XSetWindowAttributes windowAttributes;
  windowAttributes.border_pixel = BlackPixel (info->display, info->screen);
  windowAttributes.background_pixel = BlackPixel (info->display,info-> screen);
  windowAttributes.backing_store = NotUseful;

  info->window_width  = width;
  info->window_height = height;
  info->bufferWidth  = width;
  info->bufferHeight = height;
  info->bufferStride = width * 4;
  info->calcDstFactor (width, height);

  int posX, posY;
  int windowWidth, windowHeight;
  if (flags & WF_FULLSCREEN_DESKTOP) {
    //{{{  full screen desktop
    posX = 0;
    posY = 0;

    windowWidth  = screenWidth;
    windowHeight = screenHeight;
    }
    //}}}
  else {
    //{{{  window
    posX = (screenWidth  - width)  / 2;
    posY = (screenHeight - height) / 2;

    windowWidth  = width;
    windowHeight = height;
    }
    //}}}
  //}}}

  info->window = XCreateWindow (info->display, defaultRootWindow,
                                         posX, posY, windowWidth, windowHeight,
                                         0, depth, InputOutput, visual,
                                         CWBackPixel | CWBorderPixel | CWBackingStore,
                                         &windowAttributes);
  if (!info->window) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("failed to create X11 window"));
    return 0;
    }
    //}}}

  XSelectInput (info->display, info->window,
                StructureNotifyMask | ExposureMask |
                FocusChangeMask |
                KeyPressMask | KeyReleaseMask |
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                EnterWindowMask | LeaveWindowMask);
  XStoreName (info->display, info->window, title);

  if (flags & WF_BORDERLESS) {
    //{{{  borderless
    struct StyleHints {
      unsigned long flags;
      unsigned long functions;
      unsigned long decorations;
      long          inputMode;
      unsigned long status;
      } sh = {2, 0, 0, 0, 0};

    Atom sh_p = XInternAtom (info->display, "_MOTIF_WM_HINTS", True);
    XChangeProperty (info->display, info->window, sh_p, sh_p, 32,
                     PropModeReplace, (unsigned char*)&sh, 5);
    }
    //}}}
  if (flags & WF_ALWAYS_ON_TOP) {
    //{{{  always on top
    Atom sa_p = XInternAtom (info->display, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty (info->display, info->window,
                     XInternAtom (info->display, "_NET_WM_STATE", False), XA_ATOM, 32,
                     PropModeReplace, (unsigned char*)&sa_p, 1);
    }
    //}}}
  if (flags & WF_FULLSCREEN) {
    //{{{  full screen
    Atom sf_p = XInternAtom (info->display, "_NET_WM_STATE_FULLSCREEN", True);
    XChangeProperty (info->display, info->window,
                     XInternAtom (info->display, "_NET_WM_STATE", True), XA_ATOM, 32,
                     PropModeReplace, (unsigned char*)&sf_p, 1);
    }
    //}}}

  gDeleteWindowAtom = XInternAtom (info->display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols (info->display, info->window, &gDeleteWindowAtom, 1);

  if (!createGLcontext (info)) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("fialed to create GL context"));
    return 0;
    }
    //}}}

  //{{{  set sizeHints
  XSizeHints sizeHints;
  sizeHints.flags = PPosition | PMinSize | PMaxSize;
  sizeHints.x = 0;
  sizeHints.y = 0;
  sizeHints.min_width  = width;
  sizeHints.min_height = height;
  if (flags & WF_RESIZABLE) {
    //{{{  resizable
    sizeHints.max_width  = screenWidth;
    sizeHints.max_height = screenHeight;
    }
    //}}}
  else {
    //{{{  not resizable
    sizeHints.max_width  = width;
    sizeHints.max_height = height;
    }
    //}}}
  XSetWMNormalHints (info->display, info->window, &sizeHints);
  //}}}
  XClearWindow (info->display, info->window);
  XMapRaised (info->display, info->window);
  XFlush (info->display);

  info->gc = DefaultGC (info->display, info->screen);
  cLog::log (LOGINFO, "using X11 API");

  int32_t count;
  XDeviceInfoPtr devices = (XDeviceInfoPtr)XListInputDevices (info->display, &count);
  if (!devices) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("X11 - no input device list"));
    return 0;
    }
    //}}}
  //{{{  search device list for "stylus"
  cLog::log (LOGINFO, fmt::format ("X11 input devices"));

  for (int32_t i = 0; i < count; i++) {
    cLog::log (LOGINFO, fmt::format ("- device:{} name:{} id:{}", i, devices[i].name, devices[i].id));
    if (strstr (devices[i].name, "stylus")) { // "eraser"
      gDevice = XOpenDevice (info->display, devices[i].id);
      XAnyClassPtr classPtr = devices[i].inputclassinfo;
      for (int32_t j = 0; j < devices[i].num_classes; j++) {
        switch (classPtr->c_class) {
          case ValuatorClass: {
            XValuatorInfo* valuatorInfo = (XValuatorInfo*)classPtr;
            if (valuatorInfo->num_axes > 0) {
              // x
              int32_t minX = valuatorInfo->axes[0].min_value;
              gRangeX = valuatorInfo->axes[0].max_value;
              cLog::log (LOGINFO, fmt::format ("- stylus xRange {}:{}", minX, gRangeX));
              }

            if (valuatorInfo->num_axes > 1) {
              // y
              int32_t minY = valuatorInfo->axes[1].min_value;
              gRangeY = valuatorInfo->axes[1].max_value;
              cLog::log (LOGINFO, fmt::format ("- stylus yRange {}:{}", minY, gRangeY));
              }

            if (valuatorInfo->num_axes > 2) {
              // pressure
              int32_t minPressure = valuatorInfo->axes[2].min_value;
              gMaxPressure = valuatorInfo->axes[2].max_value;
              cLog::log (LOGINFO, fmt::format ("- stylus pressureRange {}:{}", minPressure, gMaxPressure));
              }

            XEventClass eventClass;
            DeviceMotionNotify (gDevice, gMotionType, eventClass);
            if (eventClass) {
              gEventClasses[gNumEventClasses] = eventClass;
              gNumEventClasses++;
              }
            }
            break;

          default:
            cLog::log (LOGINFO, fmt::format ("- unused class:{}", classPtr->c_class));
            break;
          }
        classPtr = (XAnyClassPtr)((uint8_t*)classPtr + classPtr->length);
        }
      }
    XSelectExtensionEvent (info->display, info->window, gEventClasses, gNumEventClasses);
    }
  XFreeDeviceList (devices);
  //}}}

  info->isInitialized = true;
  return info;
  }
//}}}

//{{{
eUpdateState cInfo::updateEx (void* buffer, unsigned width, unsigned height) {

  if (closed) {
    freeResources (this);
    return STATE_EXIT;
    }

  if (!buffer)
    return STATE_INVALID_BUFFER;

  if (bufferWidth != width || bufferHeight != height) {
    bufferWidth  = width;
    bufferStride = width * 4;
    bufferHeight = height;
    }

  redrawGL (this, buffer);

  processEvents (this);

  return STATE_OK;
  }
//}}}
//{{{
eUpdateState cInfo::updateEvents() {

  if (closed) {
    freeResources (this);
    return STATE_EXIT;
    }

  XFlush (display);

  processEvents (this);

  return STATE_OK;
  }
//}}}

//{{{
void cInfo::getMonitorScale (float* scale_x, float* scale_y) {

  float x = 96.0;
  float y = 96.0;

  if (scale_x) {
    *scale_x = x / 96.0f;
    if (*scale_x == 0)
      *scale_x = 1.0f;
    }

  if (scale_y) {
    *scale_y = y / 96.0f;
    if (*scale_y == 0)
      *scale_y = 1.0f;
    }
  }
//}}}
//{{{
bool cInfo::setViewport (unsigned offset_x, unsigned offset_y, unsigned width, unsigned height)  {

  if (offset_x + width > window_width)
    return false;
  if (offset_y + height > window_height)
    return false;

  dst_offset_x = offset_x;
  dst_offset_y = offset_y;
  dst_width = width;
  dst_height = height;
  calcDstFactor (window_width, window_height);

  return true;
  }
//}}}
