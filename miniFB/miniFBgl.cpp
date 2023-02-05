// MiniFB_GL.cpp
//{{{  includes
#include "miniFBgl.h"
#include "miniFBinternal.h"

#if defined(_WIN32) || defined(WIN32)
  #include "windowDataWin.h"
  #include <gl/gl.h>
#else
  #include "windowDataX11.h"
  #include <GL/gl.h>
  #include <GL/glx.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/cLog.h"

using namespace std;
//}}}

extern bool g_use_hardware_sync;
extern double g_time_for_frame;

#if defined(RGB)
  #undef RGB
#endif

#define TEXTURE0    0x84C0  // [ Core in gl 1.3, gles1 1.0, gles2 2.0, glsc2 2.0, Provided by GL_ARB_multitexture (gl) ]
#define RGB         0x1907  // [ Core in gl 1.0, gles1 1.0, gles2 2.0, glsc2 2.0 ]
#define RGBA        0x1908  // [ Core in gl 1.0, gles1 1.0, gles2 2.0, glsc2 2.0 ]
#define BGR         0x80E0  // [ Core in gl 1.2 ]
#define BGRA        0x80E1  // [ Core in gl 1.2, Provided by GL_ARB_vertex_array_bgra (gl|glcore) ]

namespace {
  #if defined(_WIN32) || defined(WIN32)
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
    PFNWGLSWAPINTERVALEXTPROC SwapIntervalEXT = 0x0;

    typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC)(void);
    PFNWGLGETSWAPINTERVALEXTPROC GetSwapIntervalEXT = 0x0;

  #else
    //{{{
    bool setup_pixel_format (SWindowData_X11* windowData_x11) {

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

      XVisualInfo* visualInfo = glXChooseVisual (windowData_x11->display, windowData_x11->screen, glxAttribs);
      if (!visualInfo) {
        cLog::log (LOGERROR, "Could not create correct visual window");
        XCloseDisplay (windowData_x11->display);
        return false;
        }

      windowData_x11->context = glXCreateContext (windowData_x11->display, visualInfo, NULL, GL_TRUE);

      return true;
      }
    //}}}

    typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display*,GLXDrawable,int);
    PFNGLXSWAPINTERVALEXTPROC SwapIntervalEXT = 0x0;
  #endif

  //{{{
  bool CheckGLExtension (const char* name) {

    static const char* extensions = 0x0;

    if (extensions == 0x0) {
      #if defined(_WIN32) || defined(WIN32)
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
  }

//{{{
void set_target_fps_aux() {

  // Assuming the monitor refresh rate is 60 hz
  int interval = (int)((60.0 * g_time_for_frame) + 0.5);

  #if defined(_WIN32) || defined(WIN32)
    if (SwapIntervalEXT) {
      bool success = SwapIntervalEXT (interval);
      cLog::log (LOGINFO, fmt::format ("windows setting swapInterval to {}", interval));

      if (GetSwapIntervalEXT) {
        int currentInterval = GetSwapIntervalEXT();
        if (interval != currentInterval)
          cLog::log (LOGERROR, fmt::format ("Cannot set target swap interval - Current swap interval is {}", currentInterval));
        }
      else if (!success)
        cLog::log (LOGERROR, fmt::format ("Cannot set target swap interval"));

      g_use_hardware_sync = true;
      }

  #else
    #define kGLX_SWAP_INTERVAL_EXT     0x20F1
    #define kGLX_MAX_SWAP_INTERVAL_EXT 0x20F2

    if (SwapIntervalEXT != 0x0) {
      Display* display = glXGetCurrentDisplay();
      GLXDrawable drawable = glXGetCurrentDrawable();
      unsigned int currentInterval, maxInterval;

      SwapIntervalEXT (display, drawable, interval);

      glXQueryDrawable (display, drawable, kGLX_SWAP_INTERVAL_EXT, &currentInterval);
      if (interval != (int)currentInterval) {
        glXQueryDrawable (display, drawable, kGLX_MAX_SWAP_INTERVAL_EXT, &maxInterval);
        cLog::log (LOGERROR, fmt::format ("Cannot set target swap interval. Current swap interval is {} max:{}",
                                          currentInterval, maxInterval));
        }

      g_use_hardware_sync = true;
      }

  #endif
  }
//}}}

//{{{
void initGL (SWindowData* windowData) {

  #if defined(_WIN32) || defined(WIN32)
    SWindowData_Win* windowData_ex = (SWindowData_Win*)windowData->specific;
  #else
    SWindowData_X11* windowData_ex = (SWindowData_X11*)windowData->specific;
  #endif

  glViewport (0, 0, windowData->window_width, windowData->window_height);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  glOrtho (0, windowData->window_width, windowData->window_height, 0, 2048, -2048);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();

  glDisable (GL_DEPTH_TEST);
  glDisable (GL_STENCIL_TEST);

  glEnable (GL_TEXTURE_2D);

  glGenTextures (1, &windowData_ex->text_id);
  //glActiveTexture (TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, windowData_ex->text_id);
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
//{{{
void resizeGL (SWindowData* windowData) {

  if (windowData->is_initialized) {
    #if defined(_WIN32) || defined(WIN32)
      SWindowData_Win* windowData_ex = (SWindowData_Win*) windowData->specific;
      wglMakeCurrent (windowData_ex->hdc, windowData_ex->hGLRC);
    #else
      SWindowData_X11* windowData_ex = (SWindowData_X11*) windowData->specific;
      glXMakeCurrent (windowData_ex->display, windowData_ex->window, windowData_ex->context);
    #endif

    glViewport (0,0, windowData->window_width,windowData->window_height);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho (0, windowData->window_width, windowData->window_height, 0, 2048, -2048);

    glClear (GL_COLOR_BUFFER_BIT);
    }
  }
//}}}
//{{{
void redrawGL (SWindowData* windowData, const void* pixels) {

  #if defined(_WIN32) || defined(WIN32)
    SWindowData_Win* windowData_ex = (SWindowData_Win*)windowData->specific;
    wglMakeCurrent (windowData_ex->hdc, windowData_ex->hGLRC);
  #else
    SWindowData_X11* windowData_ex = (SWindowData_X11*)windowData->specific;
    glXMakeCurrent (windowData_ex->display, windowData_ex->window, windowData_ex->context);
  #endif

  GLenum format = RGBA;

  // clear
  //glClear (GL_COLOR_BUFFER_BIT);
  glBindTexture (GL_TEXTURE_2D, windowData_ex->text_id);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA,
                windowData->buffer_width, windowData->buffer_height,
                0, format, GL_UNSIGNED_BYTE, pixels);
  //glTexSubImage2D (GL_TEXTURE_2D, 0,
  //                 0, 0, windowData->buffer_width, windowData->buffer_height,
  //                 format, GL_UNSIGNED_BYTE, pixels);

  // draw single texture
  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY);

  // vertices
  float x = (float)windowData->dst_offset_x;
  float y = (float)windowData->dst_offset_y;
  float w = (float)windowData->dst_offset_x + windowData->dst_width;
  float h = (float)windowData->dst_offset_y + windowData->dst_height;
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
  #if defined(_WIN32) || defined(WIN32)
    SwapBuffers (windowData_ex->hdc);
  #else
    glXSwapBuffers (windowData_ex->display, windowData_ex->window);
  #endif
  }
//}}}

//{{{
bool createGLcontext (SWindowData* windowData) {

  #if defined(_WIN32) || defined(WIN32)
    SWindowData_Win* windowData_win = (SWindowData_Win*)windowData->specific;
    if (setup_pixel_format (windowData_win->hdc) == false)
      return false;

    windowData_win->hGLRC = wglCreateContext (windowData_win->hdc);
    wglMakeCurrent (windowData_win->hdc, windowData_win->hGLRC);

    cLog::log (LOGINFO, (const char*)glGetString (GL_VENDOR));
    cLog::log (LOGINFO, (const char*)glGetString (GL_RENDERER));
    cLog::log (LOGINFO, (const char*)glGetString (GL_VERSION));
    initGL (windowData);

    // get extensions
    SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress ("wglSwapIntervalEXT");
    GetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress ("wglGetSwapIntervalEXT");
    set_target_fps_aux();

  #else
    SWindowData_X11* windowData_x11 = (SWindowData_X11*) windowData->specific;

    GLint majorGLX = 0;
    GLint minorGLX = 0;
    glXQueryVersion (windowData_x11->display, &majorGLX, &minorGLX);
    if ((majorGLX <= 1) && (minorGLX < 2)) {
      cLog::log (LOGERROR, "GLX 1.2 or greater is required");
      XCloseDisplay (windowData_x11->display);
      return false;
      }
    else
      cLog::log (LOGINFO, fmt::format ("GLX version:{}.{}", majorGLX, minorGLX));

    if (setup_pixel_format (windowData_x11) == false)
      return false;

    glXMakeCurrent (windowData_x11->display, windowData_x11->window, windowData_x11->context);

    cLog::log (LOGINFO, (const char*)glGetString (GL_VENDOR));
    cLog::log (LOGINFO, (const char*)glGetString (GL_RENDERER));
    cLog::log (LOGINFO, (const char*)glGetString (GL_VERSION));
    cLog::log (LOGINFO, (const char*)glGetString (GL_SHADING_LANGUAGE_VERSION));
    initGL (windowData);

    // get extensions
    if (CheckGLExtension ("GLX_EXT_swap_control")) {
      SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress ((const GLubyte*)"glXSwapIntervalEXT");
      set_target_fps_aux();
      }

  #endif

  return true;
  }
//}}}
//{{{
void destroyGLcontext (SWindowData* windowData) {

  #if defined(_WIN32) || defined(WIN32)
    SWindowData_Win* windowData_win = (SWindowData_Win*)windowData->specific;
    if (windowData_win->hGLRC) {
      wglMakeCurrent (NULL, NULL);
      wglDeleteContext (windowData_win->hGLRC);
      windowData_win->hGLRC = 0;
      }

  #else
    SWindowData_X11* windowData_x11 = (SWindowData_X11*)windowData->specific;
    glXDestroyContext (windowData_x11->display, windowData_x11->context);
  #endif
  }
//}}}
