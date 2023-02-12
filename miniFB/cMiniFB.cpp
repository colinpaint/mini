// cMiniFB.cpp
//{{{  includes
#include "cMiniFB.h"
#include <vector>

#ifdef _WIN32
  #include <gl/gl.h>
#else
  #include <GL/gl.h>
  #include <GL/glx.h>
#endif

#include "../common/cLog.h"

using namespace std;
//}}}
#define RGBA 0x1908  // [ Core in gl 1.0, gles1 1.0, gles2 2.0, glsc2 2.0 ]

namespace {
  #ifdef _WIN32
    //{{{
    bool setup_pixel_format (HDC hDC) {

      PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR), // size
                                   1,                             // version
                                   PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER, // support double-buffering
                                   PFD_TYPE_RGBA,                 // color type
                                   24,                            // preferred color depth
                                   0, 0, 0, 0, 0, 0,              // color and shift bits (ignored)
                                   0,                             // no alpha buffer
                                   0,                             // alpha bits (ignored)
                                   0,                             // no accumulation buffer
                                   0, 0, 0, 0,                    // accum bits (ignored)
                                   24,                            // depth buffer
                                   8,                             // no stencil buffer
                                   0,                             // no auxiliary buffers
                                   PFD_MAIN_PLANE,                // main layer
                                   0,                             // reserved
                                   0, 0, 0,                       // no layer, visible, damage masks
                                   };

      int pixelFormat = ChoosePixelFormat (hDC, &pfd);
      if (!pixelFormat) {
        MessageBox (WindowFromDC (hDC), "ChoosePixelFormat failed.", "Error", MB_ICONERROR | MB_OK);
        return false;
        }

      if (!SetPixelFormat (hDC, pixelFormat, &pfd)) {
        MessageBox (WindowFromDC (hDC), "SetPixelFormat failed.", "Error", MB_ICONERROR | MB_OK);
        return false;
        }

      return true;
      }
    //}}}
    typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
    PFNWGLSWAPINTERVALEXTPROC SwapIntervalEXT = 0;
    typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC)(void);
    PFNWGLGETSWAPINTERVALEXTPROC GetSwapIntervalEXT = 0;
    //{{{
    bool CheckGLExtension (const char* name) {

      static const char* extensions = 0x0;

      if (extensions == 0x0) {
        #ifdef _WIN32
          // TODO: This is deprecated on OpenGL 3+.
          // Use glGetIntegerv (GL_NUM_EXTENSIONS, &n) and glGetStringi (GL_EXTENSIONS, index)
          extensions = (const char*)glGetString (GL_EXTENSIONS);
        #else
          Display* display = glXGetCurrentDisplay();
          extensions = glXQueryExtensionsString (display, DefaultScreen(display));
        #endif
        }

      if (extensions != 0x0) {
        const char* start = extensions;
        const char* end, *where;
        while(1) {
          where = strstr (start, name);
          if (where == 0x0)
            return false;

          end = where + strlen(name);
          if (where == start || *(where - 1) == ' ')
            if (*end == ' ' || *end == 0)
              break;

          start = end;
          }
        }

      return true;
      }
    //}}}
  #else
    //{{{
    bool setup_pixel_format (cMiniFB* miniFB) {

      GLint glxAttribs[] = { GLX_RGBA,
                             GLX_DOUBLEBUFFER,
                             GLX_DEPTH_SIZE,     24,
                             GLX_STENCIL_SIZE,   8,
                             GLX_RED_SIZE,       8,
                             GLX_GREEN_SIZE,     8,
                             GLX_BLUE_SIZE,      8,
                             GLX_DEPTH_SIZE,     24,
                             GLX_STENCIL_SIZE,   8,
                             GLX_SAMPLE_BUFFERS, 0,
                             GLX_SAMPLES,        0,
                             None };

      XVisualInfo* visualInfo = glXChooseVisual (miniFB->display, miniFB->screen, glxAttribs);
      if (!visualInfo) {
        cLog::log (LOGERROR, "Could not create correct visual window");
        XCloseDisplay (miniFB->display);
        return false;
        }

      miniFB->context = glXCreateContext (miniFB->display, visualInfo, NULL, GL_TRUE);

      return true;
      }
    //}}}
    //typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display*,GLXDrawable,int);
    //PFNGLXSWAPINTERVALEXTPROC SwapIntervalEXT = 0x0;
  #endif
  }

//{{{
eUpdateState cMiniFB::update (void *buffer) {
  return updateEx (buffer, bufferWidth, bufferHeight);
  }
//}}}
//{{{
void cMiniFB::close() {
  closed = true;
  }
//}}}

// gets
//{{{
const char* cMiniFB::getKeyName (eKey key) {

  switch (key) {
    case KB_KEY_SPACE: return "Space";
    case KB_KEY_APOSTROPHE: return "Apostrophe";
    case KB_KEY_COMMA: return "Comma";
    case KB_KEY_MINUS: return "Minus";
    case KB_KEY_PERIOD: return "Period";
    case KB_KEY_SLASH: return "Slash";

    //{{{  numbers
    case KB_KEY_0: return "0";
    case KB_KEY_1: return "1";
    case KB_KEY_2: return "2";
    case KB_KEY_3: return "3";
    case KB_KEY_4: return "4";
    case KB_KEY_5: return "5";
    case KB_KEY_6: return "6";
    case KB_KEY_7: return "7";
    case KB_KEY_8: return "8";
    case KB_KEY_9: return "9";
    //}}}

    case KB_KEY_SEMICOLON: return "Semicolon";
    case KB_KEY_EQUAL: return "Equal";

    //{{{  alpha
    case KB_KEY_A: return "A";
    case KB_KEY_B: return "B";
    case KB_KEY_C: return "C";
    case KB_KEY_D: return "D";
    case KB_KEY_E: return "E";
    case KB_KEY_F: return "F";
    case KB_KEY_G: return "G";
    case KB_KEY_H: return "H";
    case KB_KEY_I: return "I";
    case KB_KEY_J: return "J";
    case KB_KEY_K: return "K";
    case KB_KEY_L: return "L";
    case KB_KEY_M: return "M";
    case KB_KEY_N: return "N";
    case KB_KEY_O: return "O";
    case KB_KEY_P: return "P";
    case KB_KEY_Q: return "Q";
    case KB_KEY_R: return "R";
    case KB_KEY_S: return "S";
    case KB_KEY_T: return "T";
    case KB_KEY_U: return "U";
    case KB_KEY_V: return "V";
    case KB_KEY_W: return "W";
    case KB_KEY_X: return "X";
    case KB_KEY_Y: return "Y";
    case KB_KEY_Z: return "Z";
    //}}}
    //{{{  punctuation
    case KB_KEY_LEFT_BRACKET: return "Left_Bracket";
    case KB_KEY_BACKSLASH: return "Backslash";
    case KB_KEY_RIGHT_BRACKET: return "Right_Bracket";
    case KB_KEY_GRAVE_ACCENT: return "Grave_Accent";
    case KB_KEY_WORLD_1: return "World_1";
    case KB_KEY_WORLD_2: return "World_2";
    case KB_KEY_ESCAPE: return "Escape";
    case KB_KEY_ENTER: return "Enter";
    case KB_KEY_TAB: return "Tab";
    case KB_KEY_BACKSPACE: return "Backspace";
    case KB_KEY_INSERT: return "Insert";
    case KB_KEY_DELETE: return "Delete";
    case KB_KEY_RIGHT: return "Right";
    case KB_KEY_LEFT: return "Left";
    case KB_KEY_DOWN: return "Down";
    case KB_KEY_UP: return "Up";
    case KB_KEY_PAGE_UP: return "Page_Up";
    case KB_KEY_PAGE_DOWN: return "Page_Down";
    case KB_KEY_HOME: return "Home";
    case KB_KEY_END: return "End";
    case KB_KEY_CAPS_LOCK: return "Caps_Lock";
    case KB_KEY_SCROLL_LOCK: return "Scroll_Lock";
    case KB_KEY_NUM_LOCK: return "Num_Lock";
    case KB_KEY_PRINT_SCREEN: return "Print_Screen";
    case KB_KEY_PAUSE: return "Pause";
    //}}}
    //{{{  function
    case KB_KEY_F1: return "F1";
    case KB_KEY_F2: return "F2";
    case KB_KEY_F3: return "F3";
    case KB_KEY_F4: return "F4";
    case KB_KEY_F5: return "F5";
    case KB_KEY_F6: return "F6";
    case KB_KEY_F7: return "F7";
    case KB_KEY_F8: return "F8";
    case KB_KEY_F9: return "F9";
    case KB_KEY_F10: return "F10";
    case KB_KEY_F11: return "F11";
    case KB_KEY_F12: return "F12";
    case KB_KEY_F13: return "F13";
    case KB_KEY_F14: return "F14";
    case KB_KEY_F15: return "F15";
    case KB_KEY_F16: return "F16";
    case KB_KEY_F17: return "F17";
    case KB_KEY_F18: return "F18";
    case KB_KEY_F19: return "F19";
    case KB_KEY_F20: return "F20";
    case KB_KEY_F21: return "F21";
    case KB_KEY_F22: return "F22";
    case KB_KEY_F23: return "F23";
    case KB_KEY_F24: return "F24";
    case KB_KEY_F25: return "F25";
    //}}}
    //{{{  numpad
    case KB_KEY_KP_0: return "KP_0";
    case KB_KEY_KP_1: return "KP_1";
    case KB_KEY_KP_2: return "KP_2";
    case KB_KEY_KP_3: return "KP_3";
    case KB_KEY_KP_4: return "KP_4";
    case KB_KEY_KP_5: return "KP_5";
    case KB_KEY_KP_6: return "KP_6";
    case KB_KEY_KP_7: return "KP_7";
    case KB_KEY_KP_8: return "KP_8";
    case KB_KEY_KP_9: return "KP_9";
    case KB_KEY_KP_DECIMAL: return "KP_Decimal";
    case KB_KEY_KP_DIVIDE: return "KP_Divide";
    case KB_KEY_KP_MULTIPLY: return "KP_Multiply";
    case KB_KEY_KP_SUBTRACT: return "KP_Subtract";
    case KB_KEY_KP_ADD: return "KP_Add";
    case KB_KEY_KP_ENTER: return "KP_Enter";
    case KB_KEY_KP_EQUAL: return "KP_Equal";
    //}}}

    case KB_KEY_LEFT_SHIFT: return "Left_Shift";
    case KB_KEY_LEFT_CONTROL: return "Left_Control";
    case KB_KEY_LEFT_ALT: return "Left_Alt";
    case KB_KEY_LEFT_SUPER: return "Left_Super";

    case KB_KEY_RIGHT_CONTROL: return "Right_Control";
    case KB_KEY_RIGHT_SHIFT: return "Right_Shift";
    case KB_KEY_RIGHT_ALT: return "Right_Alt";
    case KB_KEY_RIGHT_SUPER: return "Right_Super";

    case KB_KEY_MENU: return "Menu";

    case KB_KEY_UNKNOWN: return "Unknown";
    }

  return "Unknown";
  }
//}}}

// sets
void cMiniFB::setUserData (void* user_data) { userData = user_data; }
//{{{
bool cMiniFB::setViewportBestFit (unsigned old_width, unsigned old_height) {

  unsigned newWidth  = windowWidth;
  unsigned newHeight = windowHeight;

  float scale_x = newWidth  / (float) old_width;
  float scale_y = newHeight / (float) old_height;
  if (scale_x >= scale_y)
    scale_x = scale_y;
  else
    scale_y = scale_x;

  unsigned finalWidth  = (unsigned)((old_width  * scale_x) + 0.5f);
  unsigned finalHeight = (unsigned)((old_height * scale_y) + 0.5f);

  unsigned offset_x = (newWidth  - finalWidth)  >> 1;
  unsigned offset_y = (newHeight - finalHeight) >> 1;

  getMonitorScale (&scale_x, &scale_y);
  return setViewport ((unsigned)(offset_x / scale_x), (unsigned)(offset_y / scale_y),
                      (unsigned)(finalWidth / scale_x), (unsigned)(finalHeight / scale_y));
  }
//}}}

// set C style callbacks
void cMiniFB::setActiveCallback (void(*callback)(cMiniFB* miniFB)) { activeFunc = callback; }
void cMiniFB::setResizeCallback (void(*callback)(cMiniFB* miniFB)) { resizeFunc = callback; }
void cMiniFB::setCloseCallback  (bool(*callback)(cMiniFB* miniFB)) { closeFunc = callback; }
void cMiniFB::setKeyCallback    (void(*callback)(cMiniFB* miniFB)) { keyFunc = callback; }
void cMiniFB::setCharCallback   (void(*callback)(cMiniFB* miniFB)) { charFunc = callback; }
void cMiniFB::setButtonCallback (void(*callback)(cMiniFB* miniFB)) { buttonFunc = callback; }
void cMiniFB::setMoveCallback   (void(*callback)(cMiniFB* miniFB)) { moveFunc = callback; }
void cMiniFB::setWheelCallback  (void(*callback)(cMiniFB* miniFB)) { wheelFunc = callback; }
void cMiniFB::setEnterCallback  (void(*callback)(cMiniFB* miniFB)) { enterFunc = callback; }

// set function style callbacks
//{{{
void cMiniFB::setActiveFunc (function <void (cMiniFB*)> func) {

  using namespace placeholders;

  cCallbackStub::getInstance (this)->mActiveFunc = bind (func, _1);
  setActiveCallback (cCallbackStub::activeStub);
  }
//}}}
//{{{
void cMiniFB::setResizeFunc (function <void (cMiniFB*)> func) {

  using namespace placeholders;

  cCallbackStub::getInstance (this)->mResizeFunc = bind(func, _1);
  setResizeCallback (cCallbackStub::resizeStub);
  }
//}}}
//{{{
void cMiniFB::setCloseFunc  (function <bool (cMiniFB*)> func) {

  using namespace placeholders;

  cCallbackStub::getInstance (this)->mCloseFunc = bind(func, _1);
  setCloseCallback (cCallbackStub::closeStub);
  }
//}}}
//{{{
void cMiniFB::setKeyFunc    (function <void (cMiniFB*)> func) {

  using namespace placeholders;

  cCallbackStub::getInstance (this)->mKeyFunc = bind (func, _1);
  setKeyCallback (cCallbackStub::keyStub);
  }
//}}}
//{{{
void cMiniFB::setCharFunc   (function <void (cMiniFB*)> func) {

  using namespace placeholders;

  cCallbackStub::getInstance (this)->mCharFunc = bind (func, _1);
  setCharCallback (cCallbackStub::charStub);
  }
//}}}
//{{{
void cMiniFB::setButtonFunc (function <void (cMiniFB*)> func) {

  using namespace placeholders;

  cCallbackStub::getInstance (this)->mButtonFunc = bind (func, _1);
  setButtonCallback (cCallbackStub::buttonStub);
  }
//}}}
//{{{
void cMiniFB::setMoveFunc   (function <void (cMiniFB*)> func) {

  using namespace placeholders;

  cCallbackStub::getInstance (this)->mMoveFunc = bind (func, _1);
  setMoveCallback (cCallbackStub::moveStub);
  }
//}}}
//{{{
void cMiniFB::setWheelFunc  (function <void (cMiniFB*)> func) {

  using namespace placeholders;

  cCallbackStub::getInstance (this)->mWheelFunc = bind (func, _1);
  setWheelCallback (cCallbackStub::wheelStub);
  }
//}}}
//{{{
void cMiniFB::setEnterFunc  (function <void (cMiniFB*)> func) {

  using namespace placeholders;

  cCallbackStub::getInstance (this)->mEnterFunc = bind (func, _1);
  setEnterCallback (cCallbackStub::enterStub);
  }
//}}}

// utils
//{{{
void cMiniFB::resizeDst (uint32_t width, uint32_t height) {

  dstOffsetX = (uint32_t) (width  * factorX);
  dstOffsetY = (uint32_t) (height * factorY);
  dstWidth    = (uint32_t) (width  * factorWidth);
  dstHeight   = (uint32_t) (height * factorHeight);
  }
//}}}
//{{{
void cMiniFB::calcDstFactor (uint32_t width, uint32_t height) {

  if (dstWidth == 0)
    dstWidth = width;

  factorX     = (float)dstOffsetX / (float)width;
  factorWidth = (float)dstWidth    / (float)width;

  if (dstHeight == 0)
    dstHeight = height;

  factorY      = (float)dstOffsetY / (float)height;
  factorHeight = (float)dstHeight   / (float)height;
  }
//}}}

// openGL
//{{{
void cMiniFB::resizeGL() {

  if (isInitialized) {
    #ifdef _WIN32
      wglMakeCurrent (hdc, hGLRC);
    #else
      glXMakeCurrent (display, window, context);
    #endif

    glViewport (0,0, windowWidth,windowHeight);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho (0, windowWidth, windowHeight, 0, 2048, -2048);

    glClear (GL_COLOR_BUFFER_BIT);
    }
  }
//}}}
//{{{
void cMiniFB::redrawGL (const void* pixels) {

  #ifdef _WIN32
    wglMakeCurrent (hdc, hGLRC);
  #else
    glXMakeCurrent (display, window, context);
  #endif

  GLenum format = RGBA;

  // clear
  //glClear (GL_COLOR_BUFFER_BIT);
  glBindTexture (GL_TEXTURE_2D, textureId);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA,
                bufferWidth, bufferHeight,
                0, format, GL_UNSIGNED_BYTE, pixels);
  //glTexSubImage2D (GL_TEXTURE_2D, 0,
  //                 0, 0, buffer_width, buffer_height,
  //                 format, GL_UNSIGNED_BYTE, pixels);

  // draw single texture
  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY);

  // vertices
  float x = (float)dstOffsetX;
  float y = (float)dstOffsetY;
  float w = (float)dstOffsetX + dstWidth;
  float h = (float)dstOffsetY + dstHeight;
  float vertices[] = { x, y, 0, 0,
                       w, y, 1, 0,
                       x, h, 0, 1,
                       w, h, 1, 1 };
  glVertexPointer (2, GL_FLOAT, 4 * sizeof(float), vertices);
  glTexCoordPointer (2, GL_FLOAT, 4 * sizeof(float), vertices + 2);
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

  glDisableClientState (GL_TEXTURE_COORD_ARRAY);
  glDisableClientState (GL_VERTEX_ARRAY);
  glBindTexture (GL_TEXTURE_2D, 0);

  // swap buffer
  #ifdef _WIN32
    SwapBuffers (hdc);
  #else
    glXSwapBuffers (display, window);
  #endif
  }
//}}}
//{{{
bool cMiniFB::createGLcontext() {

  #ifdef _WIN32
    if (setup_pixel_format (hdc) == false)
      return false;

    hGLRC = wglCreateContext (hdc);
    wglMakeCurrent (hdc, hGLRC);

    cLog::log (LOGINFO, (const char*)glGetString (GL_VENDOR));
    cLog::log (LOGINFO, (const char*)glGetString (GL_RENDERER));
    cLog::log (LOGINFO, (const char*)glGetString (GL_VERSION));
    initGL();

    // get extensions
    SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress ("wglSwapIntervalEXT");
    GetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress ("wglGetSwapIntervalEXT");

  #else
    GLint majorGLX = 0;
    GLint minorGLX = 0;
    glXQueryVersion (display, &majorGLX, &minorGLX);
    if ((majorGLX <= 1) && (minorGLX < 2)) {
      cLog::log (LOGERROR, "GLX 1.2 or greater is required");
      XCloseDisplay (display);
      return false;
      }
    else
      cLog::log (LOGINFO, fmt::format ("GLX version:{}.{}", majorGLX, minorGLX));

    if (!setup_pixel_format (this))
      return false;

    glXMakeCurrent (display, window, context);

    cLog::log (LOGINFO, (const char*)glGetString (GL_VENDOR));
    cLog::log (LOGINFO, (const char*)glGetString (GL_RENDERER));
    cLog::log (LOGINFO, (const char*)glGetString (GL_VERSION));
    cLog::log (LOGINFO, (const char*)glGetString (GL_SHADING_LANGUAGE_VERSION));
    initGL();
  #endif

  return true;
  }
//}}}
//{{{
void cMiniFB::destroyGLcontext() {

  #ifdef _WIN32
    if (hGLRC) {
      wglMakeCurrent (NULL, NULL);
      wglDeleteContext (hGLRC);
      hGLRC = 0;
      }
  #else
    glXDestroyContext (display, context);
  #endif
  }
//}}}

// private
//{{{
void cMiniFB::initGL() {

  glViewport (0, 0, windowWidth, windowHeight);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  glOrtho (0, windowWidth, windowHeight, 0, 2048, -2048);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();

  glDisable (GL_DEPTH_TEST);
  glDisable (GL_STENCIL_TEST);

  glEnable (GL_TEXTURE_2D);

  glGenTextures (1, &textureId);
  glBindTexture (GL_TEXTURE_2D, textureId);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY);

  glDisableClientState (GL_TEXTURE_COORD_ARRAY);
  glDisableClientState (GL_VERTEX_ARRAY);
  glBindTexture (GL_TEXTURE_2D, 0);
  }
//}}}

// cCallbackStub
//{{{
cCallbackStub* cCallbackStub::getInstance (cMiniFB* miniFB) {

  //{{{
  struct stub_vector {
    vector<cCallbackStub*> instances;

    stub_vector() = default;
    //{{{
    ~stub_vector() {
      for (cCallbackStub* instance : instances)
        delete instance;
      }
    //}}}

    cCallbackStub* Get (cMiniFB *miniFB) {
      for (cCallbackStub *instance : instances) {
        if( instance->mMiniFB == miniFB) {
          return instance;
          }
        }
      instances.push_back (new cCallbackStub);
      instances.back()->mMiniFB = miniFB;
      return instances.back();
      }
    };
  //}}}
  static stub_vector gInstances;

  return gInstances.Get (miniFB);
  }
//}}}
//{{{
void cCallbackStub::activeStub (cMiniFB* miniFB) {
  cCallbackStub::getInstance (miniFB)->mActiveFunc (miniFB);
  }
//}}}
//{{{
void cCallbackStub::resizeStub (cMiniFB* miniFB) {
  cCallbackStub::getInstance (miniFB)->mResizeFunc (miniFB);
  }
//}}}
//{{{
bool cCallbackStub::closeStub  (cMiniFB* miniFB) {
  return cCallbackStub::getInstance (miniFB)->mCloseFunc (miniFB);
  }
//}}}
//{{{
void cCallbackStub::keyStub    (cMiniFB* miniFB) {
  cCallbackStub::getInstance (miniFB)->mKeyFunc(miniFB);
  }
//}}}
//{{{
void cCallbackStub::charStub   (cMiniFB* miniFB) {
  cCallbackStub::getInstance (miniFB)->mCharFunc(miniFB);
  }
//}}}
//{{{
void cCallbackStub::buttonStub (cMiniFB* miniFB) {
  cCallbackStub::getInstance (miniFB)->mButtonFunc(miniFB);
  }
//}}}
//{{{
void cCallbackStub::moveStub   (cMiniFB* miniFB) {
  cCallbackStub::getInstance (miniFB)->mMoveFunc(miniFB);
  }
//}}}
//{{{
void cCallbackStub::wheelStub  (cMiniFB* miniFB) {
  cCallbackStub::getInstance (miniFB)->mWheelFunc(miniFB);
  }
//}}}
//{{{
void cCallbackStub::enterStub  (cMiniFB* miniFB) {
  cCallbackStub::getInstance (miniFB)->mEnterFunc(miniFB);
  }
//}}}
