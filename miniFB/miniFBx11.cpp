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
#include "miniFBinternal.h"
#include "sWindowData.h"

#include "miniFBgl.h"
#include "sWindowDataX11.h"

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
  void initKeycodes (sWindowDataX11* windowDataX11) {

    // clear keys
    for (size_t i = 0; i < sizeof(gKeycodes) / sizeof(gKeycodes[0]); ++i)
      gKeycodes[i] = KB_KEY_UNKNOWN;

    // valid key code range is  [8,255], according to the Xlib manual
    for (size_t i = 8; i <= 255; ++i) {
      // try secondary keysym, for numeric keypad keys
      int keySym  = XkbKeycodeToKeysym (windowDataX11->display, i, 0, 1);
      gKeycodes[i] = translateKeyCodeB (keySym);
      if (gKeycodes[i] == KB_KEY_UNKNOWN) {
        keySym = XkbKeycodeToKeysym (windowDataX11->display, i, 0, 0);
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

    int mod_keys = 0;

    if (state & ShiftMask)
      mod_keys |= KB_MOD_SHIFT;
    if (state & ControlMask)
      mod_keys |= KB_MOD_CONTROL;
    if (state & Mod1Mask)
      mod_keys |= KB_MOD_ALT;
    if (state & Mod4Mask)
      mod_keys |= KB_MOD_SUPER;
    if (state & LockMask)
      mod_keys |= KB_MOD_CAPS_LOCK;
    if (state & Mod2Mask)
      mod_keys |= KB_MOD_NUM_LOCK;

    return mod_keys;
    }
  //}}}
  //{{{
  int translateModEx (int key, int state, int is_pressed) {

    int mod_keys = translateMod (state);

    switch (key) {
      case KB_KEY_LEFT_SHIFT:
      //{{{
      case KB_KEY_RIGHT_SHIFT:
        if (is_pressed)
          mod_keys |= KB_MOD_SHIFT;
        else
          mod_keys &= ~KB_MOD_SHIFT;
        break;
      //}}}

      case KB_KEY_LEFT_CONTROL:
      //{{{
      case KB_KEY_RIGHT_CONTROL:
        if (is_pressed)
          mod_keys |= KB_MOD_CONTROL;
        else
          mod_keys &= ~KB_MOD_CONTROL;
        break;
      //}}}

      case KB_KEY_LEFT_ALT:
      //{{{
      case KB_KEY_RIGHT_ALT:
        if (is_pressed)
          mod_keys |= KB_MOD_ALT;
        else
         mod_keys &= ~KB_MOD_ALT;
        break;
      //}}}

      case KB_KEY_LEFT_SUPER:
      //{{{
      case KB_KEY_RIGHT_SUPER:
        if (is_pressed)
          mod_keys |= KB_MOD_SUPER;
        else
          mod_keys &= ~KB_MOD_SUPER;
        break;
      //}}}
      }

    return mod_keys;
    }
  //}}}

  //{{{
  void processEvent (sWindowData* windowData, XEvent* event) {

    switch (event->type) {
      case KeyPress:
      //{{{
      case KeyRelease:
        {
        mfb_key key_code = (mfb_key)translateKey (event->xkey.keycode);
        int is_pressed = (event->type == KeyPress);
        windowData->mod_keys = translateModEx (key_code, event->xkey.state, is_pressed);

        windowData->key_status[key_code] = is_pressed;
        kCall (key_func, key_code, (eKeyModifier)windowData->mod_keys, is_pressed);

        if (event->type == KeyPress) {
          KeySym keysym;
          XLookupString (&event->xkey, NULL, 0, &keysym, NULL);
          if ((keysym >= 0x0020 && keysym <= 0x007e) ||
              (keysym >= 0x00a0 && keysym <= 0x00ff)) {
            kCall (char_func, keysym);
            }
          else if ((keysym & 0xff000000) == 0x01000000)
            keysym = keysym & 0x00ffffff;

          kCall (char_func, keysym);
          // TODO: Investigate a bit more the xkbcommon api
          // This does not seem to be working properly
          // unsigned int codepoint = xkb_state_key_get_utf32(state, keysym);
          // if (codepoint != 0)
          //    kCall(char_input_func, codepoint);
          }
        }

        break;
      //}}}

      case ButtonPress:
      //{{{
      case ButtonRelease:
        {
        ePointerButton button = (ePointerButton)event->xbutton.button;
        int is_pressed = (event->type == ButtonPress);
        windowData->mod_keys = translateMod (event->xkey.state);

        // Swap pointer right and middle for parity with other platforms:
        // https://github.com/emoon/minifb/issues/65
        switch (button) {
          case Button2:
            button = (ePointerButton)Button3;
            break;
          case Button3:
            button = (ePointerButton)Button2;
            break;
          default:;
          }

        switch (button) {
          case Button1:
          case Button2:
          case Button3:
            windowData->pointerButtonStatus[button & 0x07] = is_pressed;
            kCall (pointer_button_func, button, (eKeyModifier) windowData->mod_keys, is_pressed);
            break;

          case Button4:
            windowData->pointerWheelY = 1.0f;
            kCall (pointer_wheel_func, (eKeyModifier) windowData->mod_keys, 0.0f, windowData->pointerWheelY);
            break;

          case Button5:
            windowData->pointerWheelY = -1.0f;
            kCall (pointer_wheel_func, (eKeyModifier) windowData->mod_keys, 0.0f, windowData->pointerWheelY);
            break;

          case 6:
            windowData->pointerWheelX = 1.0f;
            kCall (pointer_wheel_func, (eKeyModifier) windowData->mod_keys, windowData->pointerWheelX, 0.0f);
            break;

          case 7:
            windowData->pointerWheelX = -1.0f;
            kCall (pointer_wheel_func, (eKeyModifier) windowData->mod_keys, windowData->pointerWheelX, 0.0f);
            break;

          default:
            windowData->pointerButtonStatus[(button - 4) & 0x07] = is_pressed;
            kCall (pointer_button_func, (ePointerButton) (button - 4), (eKeyModifier) windowData->mod_keys, is_pressed);
            break;
          }
        }

        break;
      //}}}

      //{{{
      case MotionNotify:
        windowData->pointerPosX = event->xmotion.x;
        windowData->pointerPosY = event->xmotion.y;
        kCall (pointer_move_func, windowData->pointerPosX, windowData->pointerPosY,
                                  windowData->pointerButtonStatus[Button1] * 1024, 0);
        break;
      //}}}

      //{{{
      case ConfigureNotify:
        windowData->window_width  = event->xconfigure.width;
        windowData->window_height = event->xconfigure.height;

        resizeDst (windowData, event->xconfigure.width, event->xconfigure.height);
        resizeGL (windowData);

        kCall (resize_func, windowData->window_width, windowData->window_height);

        break;
      //}}}

      //{{{
      case EnterNotify:
        kCall (pointerEnterFunc, true);
        break;
      //}}}
      //{{{
      case LeaveNotify:
        kCall (pointerEnterFunc, false);
        break;
      //}}}

      //{{{
      case FocusIn:
        windowData->is_active = true;
        kCall (active_func, true);

        break;
      //}}}
      //{{{
      case FocusOut:
        windowData->is_active = false;
        kCall (active_func, false);

        break;
      //}}}

      //{{{
      case DestroyNotify:
        windowData->close = true;
        return;
      //}}}
      //{{{
      case ClientMessage:
        if ((Atom)event->xclient.data.l[0] == gDeleteWindowAtom) {
          if (windowData) {
            bool destroy = false;
            if (!windowData->close_func || windowData->close_func ((sMiniWindow*)windowData))
              destroy = true;
            if (destroy) {
              windowData->close = true;
              return;
              }
            }
          }

        break;
      //}}}

      default:
        if (event->type == (int)gMotionType) {
          XDeviceMotionEvent* motionEvent = (XDeviceMotionEvent*)(event);
          //int posX = motionEvent->x;
          //int posY = motionEvent->y;
          int posX = motionEvent->axis_data[0];
          int posY = motionEvent->axis_data[1];
          int pressure = motionEvent->axis_data[2];
          cLog::log (LOGINFO, fmt::format ("tablet motionEvent {},{} {}", posX, posY, pressure));
          }
        else
          cLog::log (LOGINFO, fmt::format ("unused event {}", event->type));
        break;
      }
    }
  //}}}
  //{{{
  void processEvents (sWindowData* windowData) {

    sWindowDataX11* windowDataX11 = (sWindowDataX11*)windowData->specific;
    while (!windowData->close && XPending (windowDataX11->display)) {
      XEvent event;
      XNextEvent (windowDataX11->display, &event);
      processEvent (windowData, &event);
      }
    }
  //}}}
  //{{{
  void freeResources (sWindowData* windowData)  {

    if (gDevice) {
      sWindowDataX11* windowDataX11 = (sWindowDataX11*)windowData->specific;
      if (windowDataX11)
        XCloseDevice (windowDataX11->display, gDevice);
      }

    if (windowData) {
      destroyGLcontext (windowData);
      timerDestroy (windowData->timer);
      free (windowData->specific);
      free (windowData);
      }
    }
  //}}}
  }

// interface
//{{{
sMiniWindow* openEx (const char* title, unsigned width, unsigned height, unsigned flags) {

  sWindowData* windowData = (sWindowData*)calloc (1, sizeof(sWindowData));
  if (!windowData) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("failed to create window data"));
    return 0;
    }
    //}}}

  sWindowDataX11* windowDataX11 = (sWindowDataX11*)calloc (1, sizeof(sWindowDataX11));
  if (!windowDataX11) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("failed to create X11 data"));
    free (windowData);
    return 0;
    }
    //}}}
  windowData->specific = windowDataX11;

  windowDataX11->display = XOpenDisplay (0);
  if (!windowDataX11->display) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("failed to create display"));
    free (windowData);
    free (windowDataX11);
    return 0;
    }
    //}}}

  initKeycodes (windowDataX11);
  XAutoRepeatOff (windowDataX11->display);

  windowDataX11->screen = DefaultScreen (windowDataX11->display);
  Visual* visual = DefaultVisual (windowDataX11->display, windowDataX11->screen);
  //{{{  set format
  int formatCount;
  int convDepth = -1;
  XPixmapFormatValues* formats = XListPixmapFormats (windowDataX11->display, &formatCount);
  int depth = DefaultDepth (windowDataX11->display, windowDataX11->screen);
  Window defaultRootWindow = DefaultRootWindow (windowDataX11->display);
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
    XCloseDisplay (windowDataX11->display);
    return 0;
    }
    //}}}
  //}}}
  //{{{  set width, height
  int screenWidth = DisplayWidth (windowDataX11->display, windowDataX11->screen);
  int screenHeight = DisplayHeight (windowDataX11->display, windowDataX11->screen);

  XSetWindowAttributes windowAttributes;
  windowAttributes.border_pixel = BlackPixel (windowDataX11->display, windowDataX11->screen);
  windowAttributes.background_pixel = BlackPixel (windowDataX11->display, windowDataX11->screen);
  windowAttributes.backing_store = NotUseful;

  windowData->window_width  = width;
  windowData->window_height = height;
  windowData->buffer_width  = width;
  windowData->buffer_height = height;
  windowData->buffer_stride = width * 4;
  calcDstFactor (windowData, width, height);

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

  windowDataX11->window = XCreateWindow (windowDataX11->display, defaultRootWindow,
                                         posX, posY, windowWidth, windowHeight,
                                         0, depth, InputOutput, visual,
                                         CWBackPixel | CWBorderPixel | CWBackingStore,
                                         &windowAttributes);
  if (!windowDataX11->window) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("failed to create X11 window"));
    return 0;
    }
    //}}}

  XSelectInput (windowDataX11->display, windowDataX11->window,
                StructureNotifyMask | ExposureMask |
                FocusChangeMask |
                KeyPressMask | KeyReleaseMask |
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                EnterWindowMask | LeaveWindowMask);
  XStoreName (windowDataX11->display, windowDataX11->window, title);

  if (flags & WF_BORDERLESS) {
    //{{{  borderless
    struct StyleHints {
      unsigned long flags;
      unsigned long functions;
      unsigned long decorations;
      long          inputMode;
      unsigned long status;
      } sh = {2, 0, 0, 0, 0};

    Atom sh_p = XInternAtom (windowDataX11->display, "_MOTIF_WM_HINTS", True);
    XChangeProperty (windowDataX11->display, windowDataX11->window, sh_p, sh_p, 32,
                     PropModeReplace, (unsigned char*)&sh, 5);
    }
    //}}}
  if (flags & WF_ALWAYS_ON_TOP) {
    //{{{  always on top
    Atom sa_p = XInternAtom (windowDataX11->display, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty (windowDataX11->display, windowDataX11->window,
                     XInternAtom (windowDataX11->display, "_NET_WM_STATE", False), XA_ATOM, 32,
                     PropModeReplace, (unsigned char*)&sa_p, 1);
    }
    //}}}
  if (flags & WF_FULLSCREEN) {
    //{{{  full screen
    Atom sf_p = XInternAtom (windowDataX11->display, "_NET_WM_STATE_FULLSCREEN", True);
    XChangeProperty (windowDataX11->display, windowDataX11->window,
                     XInternAtom (windowDataX11->display, "_NET_WM_STATE", True), XA_ATOM, 32,
                     PropModeReplace, (unsigned char*)&sf_p, 1);
    }
    //}}}

  gDeleteWindowAtom = XInternAtom (windowDataX11->display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols (windowDataX11->display, windowDataX11->window, &gDeleteWindowAtom, 1);

  if (!createGLcontext (windowData)) {
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
  XSetWMNormalHints (windowDataX11->display, windowDataX11->window, &sizeHints);
  //}}}
  XClearWindow (windowDataX11->display, windowDataX11->window);
  XMapRaised (windowDataX11->display, windowDataX11->window);
  XFlush (windowDataX11->display);

  windowDataX11->gc = DefaultGC (windowDataX11->display, windowDataX11->screen);
  cLog::log (LOGINFO, "using X11 API");

  int32_t count;
  XDeviceInfoPtr devices = (XDeviceInfoPtr)XListInputDevices (windowDataX11->display, &count);
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
      gDevice = XOpenDevice (windowDataX11->display, devices[i].id);
      XAnyClassPtr classPtr = devices[i].inputclassinfo;
      for (int32_t j = 0; j < devices[i].num_classes; j++) {
        switch (classPtr->c_class) {
          case ValuatorClass: {
            XValuatorInfo* info = (XValuatorInfo*)classPtr;
            if (info->num_axes > 0) {
              // x
              int32_t minX = info->axes[0].min_value;
              gRangeX = info->axes[0].max_value;
              cLog::log (LOGINFO, fmt::format ("- stylus xRange {}:{}", minX, gRangeX));
              }

            if (info->num_axes > 1) {
              // y
              int32_t minY = info->axes[1].min_value;
              gRangeY = info->axes[1].max_value;
              cLog::log (LOGINFO, fmt::format ("- stylus yRange {}:{}", minY, gRangeY));
              }

            if (info->num_axes > 2) {
              // pressure
              int32_t minPressure = info->axes[2].min_value;
              gMaxPressure = info->axes[2].max_value;
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
    XSelectExtensionEvent (windowDataX11->display, windowDataX11->window, gEventClasses, gNumEventClasses);
    }
  XFreeDeviceList (devices);
  //}}}

  windowData->timer = timerCreate();
  setKeyCallback ((sMiniWindow*)windowData, keyDefault);

  windowData->is_initialized = true;
  return (sMiniWindow*)windowData;
  }
//}}}
//{{{
eUpdateState updateEx (sMiniWindow* window, void* buffer, unsigned width, unsigned height) {

  if (window == 0x0)
    return STATE_INVALID_WINDOW;

  sWindowData* windowData = (sWindowData*)window;
  if (windowData->close) {
    freeResources (windowData);
    return STATE_EXIT;
    }

  if (buffer == 0x0)
    return STATE_INVALID_BUFFER;

  if (windowData->buffer_width != width || windowData->buffer_height != height) {
    windowData->buffer_width  = width;
    windowData->buffer_stride = width * 4;
    windowData->buffer_height = height;
    }

  redrawGL (windowData, buffer);

  processEvents (windowData);

  return STATE_OK;
  }
//}}}
//{{{
eUpdateState updateEvents (sMiniWindow* window) {

  if (window == 0x0)
    return STATE_INVALID_WINDOW;

  sWindowData* windowData = (sWindowData*)window;
  if (windowData->close) {
    freeResources (windowData);
    return STATE_EXIT;
    }

  sWindowDataX11* windowDataX11 = (sWindowDataX11*)windowData->specific;
  XFlush (windowDataX11->display);

  processEvents (windowData);

  return STATE_OK;
  }
//}}}

//{{{
bool waitSync (sMiniWindow* window) {

  if (window == 0x0)
    return false;

  sWindowData* windowData = (sWindowData*)window;
  if (windowData->close) {
    freeResources (windowData);
    return false;
    }

  if (gUseHardwareSync)
    return true;

  sWindowDataX11* windowDataX11 = (sWindowDataX11*)windowData->specific;
  XFlush (windowDataX11->display);

  XEvent event;
  double current;
  uint32_t millis = 1;
  while(1) {
    current = timerNow (windowData->timer);
    if (current >= gTimeForFrame * 0.96) {
      timerReset (windowData->timer);
      return true;
      }
    else if (current >= gTimeForFrame * 0.8)
      millis = 0;

    usleep (millis * 1000);
    //sched_yield();

    if (millis == 1 && XEventsQueued (windowDataX11->display, QueuedAlready) > 0) {
      XNextEvent (windowDataX11->display, &event);
      processEvent (windowData, &event);

      if (windowData->close) {
        freeResources (windowData);
        return false;
        }
      }
    }

  return true;
  }
//}}}

//{{{
bool setViewport (sMiniWindow* window, unsigned offset_x, unsigned offset_y, unsigned width, unsigned height)  {

  sWindowData* windowData = (sWindowData*)window;

  if (offset_x + width > windowData->window_width)
    return false;
  if (offset_y + height > windowData->window_height)
    return false;

  windowData->dst_offset_x = offset_x;
  windowData->dst_offset_y = offset_y;
  windowData->dst_width = width;
  windowData->dst_height = height;
  calcDstFactor (windowData, windowData->window_width, windowData->window_height);

  return true;
  }
//}}}
//{{{
void getMonitorScale (sMiniWindow* window, float* scale_x, float* scale_y) {

  float x = 96.0;
  float y = 96.0;

  if (window != 0x0) {
    //sWindowData     *windowData     = (sWindowData *) window;
    //sWindowDataX11 *windowDataX11 = (sWindowDataX11 *) windowData->specific;
    // I cannot find a way to get dpi under VirtualBox
    // XrmGetResource "Xft.dpi", "Xft.Dpi"
    // XRRGetOutputInfo
    // DisplayWidthMM, DisplayHeightMM
    // All returning invalid values or 0
    }

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
