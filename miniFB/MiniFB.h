// miniFB.h - main interface to miniFB
#pragma once
#include "miniFBenums.h"
#include <functional>

#define MFB_RGB(r,g,b) (((uint32_t)r) << 16) | (((uint32_t)g) << 8) | ((uint32_t)b)

sMiniWindow* open (const char* title, unsigned width, unsigned height);
sMiniWindow* openEx (const char* title, unsigned width, unsigned height, unsigned flags);

mfb_update_state update (sMiniWindow* window, void* buffer);
mfb_update_state updateEx (sMiniWindow* window, void* buffer, unsigned width, unsigned height);
mfb_update_state updateEvents (sMiniWindow* window);

void close (sMiniWindow* window);

// gets
bool mfbIsWindowActive (sMiniWindow* window);
unsigned getWindowWidth (sMiniWindow* window);
unsigned getWindowHeight (sMiniWindow* window);

int getPointerX (sMiniWindow* window);          // Last Pointer pos X
int getPointerY (sMiniWindow* window);          // Last Pointer pos Y
int getPointerPressure (sMiniWindow* window);   // Last Pointer pressure
int64_t getPointerTimestamp (sMiniWindow* window);  // Last Pointer timestamp

float getPointerWheelX (sMiniWindow* window); // Pointer wheel X as a sum. When you call this function it resets.
float getPointerWheelY (sMiniWindow* window); // Pointer wheel Y as a sum. When you call this function it resets.

const uint8_t* getPointerButtonBuffer (sMiniWindow* window); // One byte for every button. Press (1), Release 0. (up to 8 buttons)
const uint8_t* getKeyBuffer (sMiniWindow* window);          // One byte for every key. Press (1), Release 0.
const char* getKeyName (mfb_key key);

void getMonitorScale (sMiniWindow* window, float* scale_x, float* scale_y);

// sets
void setUserData (sMiniWindow* window, void* user_data);
void* getUserData (sMiniWindow* window);

bool setViewport (sMiniWindow* window, unsigned offset_x, unsigned offset_y, unsigned width, unsigned height);
bool setViewportBestFit (sMiniWindow* window, unsigned old_width, unsigned old_height);

// fps
void setTargetFps (uint32_t fps);
unsigned getTargetFps();
bool waitSync (sMiniWindow* window);

// timer
struct sMiniFBtimer* timerCreate();
void timerDestroy (struct sMiniFBtimer* tmr);
void  timerReset (struct sMiniFBtimer* tmr);
double timerNow (struct sMiniFBtimer* tmr);
double timerDelta (struct sMiniFBtimer* tmr);
double timerGetFrequency();
double timerGetResolution();

// func callbacks
void setActiveCallback (sMiniWindow* window, mfb_active_func callback);
void setResizeCallback (sMiniWindow* window, mfb_resize_func callback);
void setCloseCallback (sMiniWindow* window, mfb_close_func callback);

void setKeyCallback (sMiniWindow* window, mfb_key_func callback);
void setCharCallback (sMiniWindow* window, mfb_char_func callback);

void setPointerButtonCallback (sMiniWindow* window, mfb_pointer_button_func callback);
void setPointerMoveCallback (sMiniWindow* window, mfb_pointer_move_func callback);
void setPointerWheelCallback (sMiniWindow* window, mfb_pointer_wheel_func callback);
void setPointerLeaveCallback (sMiniWindow* window, mfb_pointer_leave_func callback);

// lambda callbacks
void setActiveCallback (std::function <void (sMiniWindow*, bool)> func, sMiniWindow* window);
void setResizeCallback (std::function <void (sMiniWindow*, int, int)> func, sMiniWindow* window);
void setCloseCallback (std::function <bool (sMiniWindow*)> func, sMiniWindow* window);

void setKeyCallback (std::function <void (sMiniWindow*, mfb_key, mfb_key_mod, bool)> func, sMiniWindow* window);
void setCharCallback (std::function <void (sMiniWindow*, unsigned int)> func, sMiniWindow* window);

void setPointerButtonCallback (std::function <void (sMiniWindow*, mfb_pointer_button, mfb_key_mod, bool)> func, sMiniWindow* window);
void setPointerMoveCallback (std::function <void (sMiniWindow*, int, int, int, int)>func, sMiniWindow* window);
void setPointerWheelCallback (std::function <void (sMiniWindow*, mfb_key_mod, float, float)> func, sMiniWindow* window);
void setPointerLeaveCallback (std::function <void (sMiniWindow*, bool)> func, sMiniWindow* window);

// templates
template <class T> void setActiveCallback (sMiniWindow* window, T* obj,
                                           void (T::*method)(sMiniWindow*, bool));
template <class T> void setResizeCallback (sMiniWindow* window, T* obj,
                                           void (T::*method)(sMiniWindow*, int, int));

template <class T> void setKeyCallback (sMiniWindow* window, T* obj,
                                        void (T::*method)(sMiniWindow*, mfb_key, mfb_key_mod, bool));
template <class T> void setCharCallback (sMiniWindow* window, T* obj,
                                         void (T::*method)(sMiniWindow*, unsigned int));

template <class T> void setPointerButtonCallback (sMiniWindow* window, T* obj,
                                                  void (T::*method)(sMiniWindow*, mfb_pointer_button, mfb_key_mod, bool));
template <class T> void setPointerMoveCallback (sMiniWindow* window, T* obj,
                                              void (T::*method)(sMiniWindow*, int, int, int, int));
template <class T> void setPointerWheelCallback (sMiniWindow* window, T* obj,
                                               void (T::*method)(sMiniWindow*, mfb_key_mod, float, float));
template <class T> void setPointerLeaveCallback (sMiniWindow* window, T* obj,
                                                 void (T::*method)(sMiniWindow*, bool));

//{{{
class mfbStub {
  mfbStub() : m_window(0x0) {}

  // friends
  friend void setActiveCallback (std::function <void (sMiniWindow* window, bool)> func, sMiniWindow* window);
  friend void setResizeCallback (std::function <void (sMiniWindow*, int, int)> func, sMiniWindow* window);
  friend void setCloseCallback (std::function <bool (sMiniWindow*)> func, sMiniWindow* window);

  friend void setKeyCallback (std::function <void (sMiniWindow*, mfb_key, mfb_key_mod, bool)> func,
                              sMiniWindow* window);
  friend void setCharCallback (std::function <void (sMiniWindow*, unsigned int)> func,
                               sMiniWindow* window);

  friend void setPointerButtonCallback (std::function <void (sMiniWindow*, mfb_pointer_button, mfb_key_mod, bool)> func,
                                      sMiniWindow* window);
  friend void setPointerMoveCallback (std::function <void (sMiniWindow*, int, int, int, int)> func,
                                    sMiniWindow* window);
  friend void setPointerWheelCallback (std::function <void (sMiniWindow*, mfb_key_mod, float, float)> func,
                                     sMiniWindow* window);
  friend void setPointerLeaveCallback (std::function <void (sMiniWindow*, bool)> func,
                                       sMiniWindow* window);

  // templates
  template <class T> friend void setActiveCallback (sMiniWindow* window, T* obj,
                                                    void (T::*method)(sMiniWindow*, bool));
  template <class T> friend void setResizeCallback (sMiniWindow* window, T* obj,
                                                    void (T::*method)(sMiniWindow*, int, int));
  template <class T> friend void setCloseCallback (sMiniWindow* window, T* obj,
                                                   bool (T::*method)(sMiniWindow* ));

  template <class T> friend void setKeyCallback (sMiniWindow* window, T* obj,
                                                 void (T::*method)(sMiniWindow*, mfb_key, mfb_key_mod, bool));
  template <class T> friend void setCharCallback (sMiniWindow* window, T* obj,
                                                  void (T::*method)(sMiniWindow*, unsigned int));

  template <class T> friend void setPointerButtonCallback (sMiniWindow* window, T* obj,
                                                         void (T::*method)(sMiniWindow*, mfb_pointer_button, mfb_key_mod, bool));
  template <class T> friend void setPointerMoveCallback (sMiniWindow* window, T* obj,
                                                       void (T::*method)(sMiniWindow*, int, int, int, int));
  template <class T> friend void setPointerWheelCallback (sMiniWindow* window, T* obj,
                                                        void (T::*method)(sMiniWindow*, mfb_key_mod, float, float));
  template <class T> friend void setPointerLeaveCallback (sMiniWindow* wndow, T* obj,
                                                          void (T::*method)(sMiniWindow*, bool));

  // statics
  static mfbStub* GetInstance (sMiniWindow* window);

  static void activeStub (sMiniWindow* window, bool isActive);
  static void resizeStub (sMiniWindow* window, int width, int height);
  static bool closeStub (sMiniWindow* window);

  static void keyStub (sMiniWindow* window, mfb_key key, mfb_key_mod mod, bool isPressed);
  static void charStub (sMiniWindow* window, unsigned int);

  static void pointerButtonStub (sMiniWindow* window, mfb_pointer_button button, mfb_key_mod mod, bool isPressed);
  static void pointerMoveStub (sMiniWindow* window, int x, int y, int pressure, int timestamp);
  static void pointerWheelStub (sMiniWindow* window, mfb_key_mod mod, float deltaX, float deltaY);
  static void pointerLeaveStub (sMiniWindow* window, bool);

  // vars
  sMiniWindow* m_window;

  std::function <void (sMiniWindow* window, bool)> m_active;
  std::function <void (sMiniWindow* window, int, int)> m_resize;
  std::function <bool (sMiniWindow* window)> m_close;

  std::function <void (sMiniWindow* window, mfb_key, mfb_key_mod, bool)> m_key;
  std::function <void (sMiniWindow* window, unsigned int)> m_char;

  std::function <void (sMiniWindow* window, mfb_pointer_button, mfb_key_mod, bool)> m_pointer_button;
  std::function <void (sMiniWindow* window, int, int, int, int)> m_pointer_move;
  std::function <void (sMiniWindow* window, mfb_key_mod, float, float)> m_pointer_wheel;
  std::function <void (sMiniWindow* window, bool)> m_pointer_leave;
  };
//}}}

//{{{
template <class T> inline void setActiveCallback (sMiniWindow* window, T* obj,
                                                  void (T::*method)(sMiniWindow* window, bool)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_active = std::bind (method, obj, _1, _2);

  setActiveCallback (window, mfbStub::activeStub);
  }
//}}}
//{{{
template <class T> inline void setResizeCallback (sMiniWindow* window, T* obj,
                                                  void (T::*method)(sMiniWindow* window, int, int)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_resize = std::bind (method, obj, _1, _2, _3);

  setResizeCallback (window, mfbStub::resizeStub);
  }
//}}}
//{{{
template <class T> inline void setCloseCallback (sMiniWindow* window, T* obj,
                                                 bool (T::*method)(sMiniWindow* window)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_close = std::bind (method, obj, _1);

  setCloseCallback (window, mfbStub::closeStub);
  }
//}}}

//{{{
template <class T> inline void setKeyCallback (sMiniWindow* window, T* obj,
                                               void (T::*method)(sMiniWindow* window, mfb_key, mfb_key_mod, bool)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance(window);
  stub->m_key = std::bind(method, obj, _1, _2, _3, _4);

  setKeyCallback (window, mfbStub::keyStub);
  }
//}}}
//{{{
template <class T> inline void setCharCallback (sMiniWindow* window, T* obj,
                                                void (T::*method)(sMiniWindow* window, unsigned int)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_char = std::bind (method, obj, _1, _2);

  setCharCallback (window, mfbStub::charStub);
  }
//}}}

//{{{
template <class T> inline void setPointerButtonCallback (sMiniWindow* window, T* obj,
                                                       void (T::*method)(sMiniWindow* window, mfb_pointer_button, mfb_key_mod, bool)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_pointer_button = std::bind (method, obj, _1, _2, _3, _4);

  setPointerButtonCallback (window, mfbStub::pointerButtonStub);
  }
//}}}
//{{{
template <class T> inline void setPointerMoveCallback (sMiniWindow* window, T* obj,
                                                     void (T::*method)(sMiniWindow* window, int, int, int, int)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_pointer_move = std::bind (method, obj, _1, _2, _3, _4, _5);

  setPointerMoveCallback (window, mfbStub::pointerMoveStub);
  }
//}}}
//{{{
template <class T> inline void setPointerWheelCallback (sMiniWindow* window, T* obj,
                                                      void (T::*method)(sMiniWindow* window, mfb_key_mod, float, float)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_pointer_wheel = std::bind (method, obj, _1, _2, _3, _4);

  setPointerWheelCallback (window, mfbStub::pointerWheelStub);
  }
//}}}
//{{{
template <class T> inline void setPointerLeaveCallback (sMiniWindow* window, T* obj,
                                                       void (T::*method)(sMiniWindow* window, bool)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_pointer_leave = std::bind (method, obj, _1, _2);

  setPointerLeaveCallback (window, mfbStub::pointerLeaveStub);
  }
//}}}
