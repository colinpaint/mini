// miniFB.h - main interface to miniFB
#pragma once
#include <functional>
#include "miniFBenums.h"

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
#else
  #include <X11/Xlib.h>
  #include <GL/glx.h>
#endif

struct sMiniFBtimer;
class sInfo;
typedef void(*infoFuncType)(sInfo* info);
typedef bool(*closeFuncType)(sInfo* info);
//{{{
class sInfo {
public:
  eUpdateState update (void* buffer);
  eUpdateState updateEx (void* buffer, unsigned width, unsigned height);
  eUpdateState updateEvents();
  void close();

  bool waitSync();
  // gets
  void* getUserData () { return userData; }

  bool isWindowActive() { return isActive; }
  unsigned getWindowWidth()  { return window_width; }
  unsigned getWindowHeight() { return window_height; }

  int getPointerX () { return pointerPosX; }
  int getPointerY () { return pointerPosY; }
  int getPointerPressure() { return pointerPressure; }
  int64_t getPointerTimestamp() { return pointerTimestamp; }

  float getPointerWheelX() { return pointerWheelX; }
  float getPointerWheelY() { return pointerWheelY; }

  const uint8_t* getPointerButtonBuffer() { return pointerButtonStatus; }
  const uint8_t* getKeyBuffer()  { return keyStatus; }

  void getMonitorScale (float* scale_x, float* scale_y);

  // sets
  void setUserData (void* user_data);
  bool setViewport (unsigned offset_x, unsigned offset_y, unsigned width, unsigned height);
  bool setViewportBestFit (unsigned old_width, unsigned old_height);

  // static
  static const char* getKeyName (eKey key);

  // vars
  void* userData;

  infoFuncType  activeFunc;
  infoFuncType  resizeFunc;
  closeFuncType closeFunc;
  infoFuncType  keyFunc;
  infoFuncType  charFunc;
  infoFuncType  buttonFunc;
  infoFuncType  moveFunc;
  infoFuncType  wheelFunc;
  infoFuncType  enterFunc;

  uint32_t window_width;
  uint32_t window_height;
  uint32_t windowScaledWidth;
  uint32_t windowScaledHeight;

  uint32_t dst_offset_x;
  uint32_t dst_offset_y;
  uint32_t dst_width;
  uint32_t dst_height;

  float    factor_x;
  float    factor_y;
  float    factor_width;
  float    factor_height;

  void*    draw_buffer;
  uint32_t bufferWidth;
  uint32_t bufferHeight;
  uint32_t bufferStride;

  uint32_t isPressed;
  bool     isActive;
  bool     isInitialized;
  bool     isDown;
  bool     pointerInside;
  bool     closed;

  uint32_t codepoint;
  eKey     keyCode;
  uint8_t  keyStatus[512];
  uint32_t modifierKeys;

  int32_t  pointerTimestamp;
  uint8_t  pointerButtonStatus[8];
  int32_t  pointerPosX;
  int32_t  pointerPosY;
  int32_t  pointerPressure;
  int32_t  pointerTiltX;
  int32_t  pointerTiltY;
  float    pointerWheelX;
  float    pointerWheelY;

  sMiniFBtimer* timer;
  uint32_t textureId;

  #ifdef _WIN32
    HWND     window;
    WNDCLASS wc;
    HDC      hdc;
    HGLRC    hGLRC;
  #else
    Window   window;
    Display* display;
    int      screen;
    GC       gc;
    GLXContext context;
  #endif
  };
//}}}

sInfo* open (const char* title, unsigned width, unsigned height);
sInfo* openEx (const char* title, unsigned width, unsigned height, unsigned flags);


// fps
void setTargetFps (uint32_t fps);
unsigned getTargetFps();

// timer
struct sMiniFBtimer;
struct sMiniFBtimer* timerCreate();
void timerDestroy (struct sMiniFBtimer* tmr);
void  timerReset (struct sMiniFBtimer* tmr);
double timerNow (struct sMiniFBtimer* tmr);
double timerDelta (struct sMiniFBtimer* tmr);
double timerGetFrequency();
double timerGetResolution();

//{{{  callbacks
void setActiveCallback (sInfo* info, infoFuncType callback);
void setResizeCallback (sInfo* info, infoFuncType callback);
void setCloseCallback  (sInfo* info, closeFuncType callback);
void setKeyCallback    (sInfo* info, infoFuncType callback);
void setCharCallback   (sInfo* info, infoFuncType callback);
void setButtonCallback (sInfo* info, infoFuncType callback);
void setMoveCallback   (sInfo* info, infoFuncType callback);
void setWheelCallback  (sInfo* info, infoFuncType callback);
void setEnterCallback  (sInfo* info, infoFuncType callback);

// lambda callbacks
void setActiveCallback (std::function <void (sInfo*)> func, sInfo* info);
void setResizeCallback (std::function <void (sInfo*)> func, sInfo* info);
void setCloseCallback  (std::function <bool (sInfo*)> func, sInfo* info);
void setKeyCallback    (std::function <void (sInfo*)> func, sInfo* info);
void setCharCallback   (std::function <void (sInfo*)> func, sInfo* info);
void setButtonCallback (std::function <void (sInfo*)> func, sInfo* info);
void setMoveCallback   (std::function <void (sInfo*)> func, sInfo* info);
void setWheelCallback  (std::function <void (sInfo*)> func, sInfo* info);
void setEnterCallback  (std::function <void (sInfo*)> func, sInfo* info);

//{{{
class cStub {
  cStub() {}

  // friends
  friend void setActiveCallback (std::function <void (sInfo*)> func, sInfo* info);
  friend void setResizeCallback (std::function <void (sInfo*)> func, sInfo* info);
  friend void setCloseCallback  (std::function <bool (sInfo*)> func, sInfo* info);
  friend void setKeyCallback    (std::function <void (sInfo*)> func, sInfo* info);
  friend void setCharCallback   (std::function <void (sInfo*)> func, sInfo* info);
  friend void setButtonCallback (std::function <void (sInfo*)> func, sInfo* info);
  friend void setMoveCallback   (std::function <void (sInfo*)> func, sInfo* info);
  friend void setWheelCallback  (std::function <void (sInfo*)> func, sInfo* info);
  friend void setEnterCallback  (std::function <void (sInfo*)> func, sInfo* info);

  // statics
  static cStub* getInstance (sInfo* info);

  static void activeStub (sInfo* info);
  static void resizeStub (sInfo* info);
  static bool closeStub  (sInfo* info);
  static void keyStub    (sInfo* info);
  static void charStub   (sInfo* info);
  static void buttonStub (sInfo* info);
  static void moveStub   (sInfo* info);
  static void wheelStub  (sInfo* info);
  static void enterStub  (sInfo* info);

  // vars
  sInfo* m_info = nullptr;

  std::function <void (sInfo* info)> mActiveFunc;
  std::function <void (sInfo* info)> mResizeFunc;
  std::function <bool (sInfo* info)> mCloseFunc;
  std::function <void (sInfo* info)> mKeyFunc;
  std::function <void (sInfo* info)> mCharFunc;
  std::function <void (sInfo* info)> mButtonFunc;
  std::function <void (sInfo* info)> mMoveFunc;
  std::function <void (sInfo* info)> mWheelFunc;
  std::function <void (sInfo* info)> mEnterFunc;
  };
//}}}
//}}}
