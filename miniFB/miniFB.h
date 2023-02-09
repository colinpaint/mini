// miniFB.h - main interface to miniFB
#pragma once
#include "miniFBenums.h"
#include <functional>

#define MFB_RGB(r,g,b) (((uint32_t)r) << 16) | (((uint32_t)g) << 8) | ((uint32_t)b)

// opaque window pointer
struct sWindow;

sWindow* open (const char* title, unsigned width, unsigned height);
sWindow* openEx (const char* title, unsigned width, unsigned height, unsigned flags);

eUpdateState update (sWindow* window, void* buffer);
eUpdateState updateEx (sWindow* window, void* buffer, unsigned width, unsigned height);
eUpdateState updateEvents (sWindow* window);

void close (sWindow* window);

// gets
bool isWindowActive (sWindow* window);
unsigned getWindowWidth (sWindow* window);
unsigned getWindowHeight (sWindow* window);

int getPointerX (sWindow* window);          // Last Pointer pos X
int getPointerY (sWindow* window);          // Last Pointer pos Y
int getPointerPressure (sWindow* window);   // Last Pointer pressure
int64_t getPointerTimestamp (sWindow* window);  // Last Pointer timestamp

float getPointerWheelX (sWindow* window); // Pointer wheel X as a sum. When you call this function it resets.
float getPointerWheelY (sWindow* window); // Pointer wheel Y as a sum. When you call this function it resets.

const uint8_t* getPointerButtonBuffer (sWindow* window); // One byte for every button. Press (1), Release 0. (up to 8 buttons)
const uint8_t* getKeyBuffer (sWindow* window);          // One byte for every key. Press (1), Release 0.
const char* getKeyName (eKey key);

void getMonitorScale (sWindow* window, float* scale_x, float* scale_y);

// sets
void setUserData (sWindow* window, void* user_data);
void* getUserData (sWindow* window);

bool setViewport (sWindow* window, unsigned offset_x, unsigned offset_y, unsigned width, unsigned height);
bool setViewportBestFit (sWindow* window, unsigned old_width, unsigned old_height);

// fps
void setTargetFps (uint32_t fps);
unsigned getTargetFps();

bool waitSync (sWindow* window);

// timer
struct sMiniFBtimer;
struct sMiniFBtimer* timerCreate();
void timerDestroy (struct sMiniFBtimer* tmr);
void  timerReset (struct sMiniFBtimer* tmr);
double timerNow (struct sMiniFBtimer* tmr);
double timerDelta (struct sMiniFBtimer* tmr);
double timerGetFrequency();
double timerGetResolution();

// func callbacks
typedef void(*activeFuncType)(sWindow* window, bool isActive);
typedef void(*resizeFuncType)(sWindow* window, int width, int height);
typedef bool(*closeFuncType)(sWindow* window);

typedef void(*keyFuncType)(sWindow* window, eKey key, eKeyModifier mod, bool isPressed);
typedef void(*charFuncType)(sWindow* window, unsigned int code);

typedef void(*pointerButtonFuncType)(sWindow* window, ePointerButton button, eKeyModifier mod, bool isPressed);
typedef void(*pointerMoveFuncType)(sWindow* window, int x, int y, int pressure, int timestamp);
typedef void(*pointerWheelFuncType)(sWindow* window, eKeyModifier mod, float deltaX, float deltaY);
typedef void(*pointerEnterFuncType)(sWindow* window, bool enter);

void setActiveCallback (sWindow* window, activeFuncType callback);
void setResizeCallback (sWindow* window, resizeFuncType callback);
void setCloseCallback (sWindow* window, closeFuncType callback);

void setKeyCallback (sWindow* window, keyFuncType callback);
void setCharCallback (sWindow* window, charFuncType callback);

void setPointerButtonCallback (sWindow* window, pointerButtonFuncType callback);
void setPointerMoveCallback (sWindow* window, pointerMoveFuncType callback);
void setPointerWheelCallback (sWindow* window, pointerWheelFuncType callback);
void setPointerEnterCallback (sWindow* window, pointerEnterFuncType callback);

// lambda callbacks
void setActiveCallback (std::function <void (sWindow*, bool)> func, sWindow* window);
void setResizeCallback (std::function <void (sWindow*, int, int)> func, sWindow* window);
void setCloseCallback (std::function <bool (sWindow*)> func, sWindow* window);

void setKeyCallback (std::function <void (sWindow*, eKey, eKeyModifier, bool)> func, sWindow* window);
void setCharCallback (std::function <void (sWindow*, unsigned int)> func, sWindow* window);

void setPointerButtonCallback (std::function <void (sWindow*, ePointerButton, eKeyModifier, bool)> func, sWindow* window);
void setPointerMoveCallback (std::function <void (sWindow*, int, int, int, int)>func, sWindow* window);
void setPointerWheelCallback (std::function <void (sWindow*, eKeyModifier, float, float)> func, sWindow* window);
void setPointerEnterCallback (std::function <void (sWindow*, bool)> func, sWindow* window);

// templates
template <class T> void setActiveCallback (sWindow* window, T* obj,
                                           void (T::*method)(sWindow*, bool));
template <class T> void setResizeCallback (sWindow* window, T* obj,
                                           void (T::*method)(sWindow*, int, int));

template <class T> void setKeyCallback (sWindow* window, T* obj,
                                        void (T::*method)(sWindow*, eKey, eKeyModifier, bool));
template <class T> void setCharCallback (sWindow* window, T* obj,
                                         void (T::*method)(sWindow*, unsigned int));

template <class T> void setPointerButtonCallback (sWindow* window, T* obj,
                                                  void (T::*method)(sWindow*, ePointerButton, eKeyModifier, bool));
template <class T> void setPointerMoveCallback (sWindow* window, T* obj,
                                              void (T::*method)(sWindow*, int, int, int, int));
template <class T> void setPointerWheelCallback (sWindow* window, T* obj,
                                               void (T::*method)(sWindow*, eKeyModifier, float, float));
template <class T> void setPointerEnterCallback (sWindow* window, T* obj,
                                                 void (T::*method)(sWindow*, bool));

//{{{
class cStub {
  cStub() : m_window(0x0) {}

  // friends
  friend void setActiveCallback (std::function <void (sWindow* window, bool)> func, sWindow* window);
  friend void setResizeCallback (std::function <void (sWindow*, int, int)> func, sWindow* window);
  friend void setCloseCallback (std::function <bool (sWindow*)> func, sWindow* window);

  friend void setKeyCallback (std::function <void (sWindow*, eKey, eKeyModifier, bool)> func,
                              sWindow* window);
  friend void setCharCallback (std::function <void (sWindow*, unsigned int)> func,
                               sWindow* window);

  friend void setPointerButtonCallback (std::function <void (sWindow*, ePointerButton, eKeyModifier, bool)> func,
                                      sWindow* window);
  friend void setPointerMoveCallback (std::function <void (sWindow*, int, int, int, int)> func,
                                    sWindow* window);
  friend void setPointerWheelCallback (std::function <void (sWindow*, eKeyModifier, float, float)> func,
                                     sWindow* window);
  friend void setPointerEnterCallback (std::function <void (sWindow*, bool)> func,
                                       sWindow* window);

  // templates
  template <class T> friend void setActiveCallback (sWindow* window, T* obj,
                                                    void (T::*method)(sWindow*, bool));
  template <class T> friend void setResizeCallback (sWindow* window, T* obj,
                                                    void (T::*method)(sWindow*, int, int));
  template <class T> friend void setCloseCallback (sWindow* window, T* obj,
                                                   bool (T::*method)(sWindow* ));

  template <class T> friend void setKeyCallback (sWindow* window, T* obj,
                                                 void (T::*method)(sWindow*, eKey, eKeyModifier, bool));
  template <class T> friend void setCharCallback (sWindow* window, T* obj,
                                                  void (T::*method)(sWindow*, unsigned int));

  template <class T> friend void setPointerButtonCallback (sWindow* window, T* obj,
                                                         void (T::*method)(sWindow*, ePointerButton, eKeyModifier, bool));
  template <class T> friend void setPointerMoveCallback (sWindow* window, T* obj,
                                                       void (T::*method)(sWindow*, int, int, int, int));
  template <class T> friend void setPointerWheelCallback (sWindow* window, T* obj,
                                                        void (T::*method)(sWindow*, eKeyModifier, float, float));
  template <class T> friend void setPointerEnterCallback (sWindow* wndow, T* obj,
                                                          void (T::*method)(sWindow*, bool));

  // statics
  static cStub* GetInstance (sWindow* window);

  static void activeStub (sWindow* window, bool isActive);
  static void resizeStub (sWindow* window, int width, int height);
  static bool closeStub (sWindow* window);

  static void keyStub (sWindow* window, eKey key, eKeyModifier mod, bool isPressed);
  static void charStub (sWindow* window, unsigned int);

  static void pointerButtonStub (sWindow* window, ePointerButton button, eKeyModifier mod, bool isPressed);
  static void pointerMoveStub (sWindow* window, int x, int y, int pressure, int timestamp);
  static void pointerWheelStub (sWindow* window, eKeyModifier mod, float deltaX, float deltaY);
  static void pointerEnterStub (sWindow* window, bool);

  // vars
  sWindow* m_window;

  std::function <void (sWindow* window, bool)> m_active;
  std::function <void (sWindow* window, int, int)> m_resize;
  std::function <bool (sWindow* window)> m_close;

  std::function <void (sWindow* window, eKey, eKeyModifier, bool)> m_key;
  std::function <void (sWindow* window, unsigned int)> m_char;

  std::function <void (sWindow* window, ePointerButton, eKeyModifier, bool)> m_pointer_button;
  std::function <void (sWindow* window, int, int, int, int)> m_pointer_move;
  std::function <void (sWindow* window, eKeyModifier, float, float)> m_pointer_wheel;
  std::function <void (sWindow* window, bool)> mPointerEnter;
  };
//}}}

//{{{
template <class T> inline void setActiveCallback (sWindow* window, T* obj,
                                                  void (T::*method)(sWindow* window, bool)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (window);
  stub->m_active = std::bind (method, obj, _1, _2);

  setActiveCallback (window, cStub::activeStub);
  }
//}}}
//{{{
template <class T> inline void setResizeCallback (sWindow* window, T* obj,
                                                  void (T::*method)(sWindow* window, int, int)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (window);
  stub->m_resize = std::bind (method, obj, _1, _2, _3);

  setResizeCallback (window, cStub::resizeStub);
  }
//}}}
//{{{
template <class T> inline void setCloseCallback (sWindow* window, T* obj,
                                                 bool (T::*method)(sWindow* window)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (window);
  stub->m_close = std::bind (method, obj, _1);

  setCloseCallback (window, cStub::closeStub);
  }
//}}}

//{{{
template <class T> inline void setKeyCallback (sWindow* window, T* obj,
                                               void (T::*method)(sWindow* window, eKey, eKeyModifier, bool)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance(window);
  stub->m_key = std::bind(method, obj, _1, _2, _3, _4);

  setKeyCallback (window, cStub::keyStub);
  }
//}}}
//{{{
template <class T> inline void setCharCallback (sWindow* window, T* obj,
                                                void (T::*method)(sWindow* window, unsigned int)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (window);
  stub->m_char = std::bind (method, obj, _1, _2);

  setCharCallback (window, cStub::charStub);
  }
//}}}

//{{{
template <class T> inline void setPointerButtonCallback (sWindow* window, T* obj,
                                                       void (T::*method)(sWindow* window, ePointerButton, eKeyModifier, bool)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (window);
  stub->m_pointer_button = std::bind (method, obj, _1, _2, _3, _4);

  setPointerButtonCallback (window, cStub::pointerButtonStub);
  }
//}}}
//{{{
template <class T> inline void setPointerMoveCallback (sWindow* window, T* obj,
                                                     void (T::*method)(sWindow* window, int, int, int, int)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (window);
  stub->m_pointer_move = std::bind (method, obj, _1, _2, _3, _4, _5);

  setPointerMoveCallback (window, cStub::pointerMoveStub);
  }
//}}}
//{{{
template <class T> inline void setPointerWheelCallback (sWindow* window, T* obj,
                                                      void (T::*method)(sWindow* window, eKeyModifier, float, float)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (window);
  stub->m_pointer_wheel = std::bind (method, obj, _1, _2, _3, _4);

  setPointerWheelCallback (window, cStub::pointerWheelStub);
  }
//}}}
//{{{
template <class T> inline void setPointerEnterCallback (sWindow* window, T* obj,
                                                       void (T::*method)(sWindow* window, bool)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (window);
  stub->mPointerEnter = std::bind (method, obj, _1, _2);

  setPointerEnterCallback (window, cStub::pointerEnterStub);
  }
//}}}
