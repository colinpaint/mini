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

int getMouseX (sMiniWindow* window);          // Last mouse pos X
int getMouseY (sMiniWindow* window);          // Last mouse pos Y
int getMousePressure (sMiniWindow* window);   // Last mouse pressure
int64_t getMouseTimestamp (sMiniWindow* window);  // Last mouse timestamp

float getMouseScrollX (sMiniWindow* window); // Mouse wheel X as a sum. When you call this function it resets.
float getMouseScrollY (sMiniWindow* window); // Mouse wheel Y as a sum. When you call this function it resets.

const uint8_t* getMouseButtonBuffer (sMiniWindow* window); // One byte for every button. Press (1), Release 0. (up to 8 buttons)
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

// callbacks
void setActiveCallback (sMiniWindow* window, mfb_active_func callback);
void setResizeCallback (sMiniWindow* window, mfb_resize_func callback);
void setCloseCallback (sMiniWindow* window, mfb_close_func callback);

void setKeyboardCallback (sMiniWindow* window, mfb_keyboard_func callback);
void setCharInputCallback (sMiniWindow* window, mfb_char_input_func callback);

void setMouseButtonCallback (sMiniWindow* window, mfb_mouse_button_func callback);
void setMouseMoveCallback (sMiniWindow* window, mfb_mouse_move_func callback);
void setMouseScrollCallback (sMiniWindow* window, mfb_mouse_scroll_func callback);

// timer
struct sMiniFBtimer* timerCreate();
void timerDestroy (struct sMiniFBtimer* tmr);
void  timerReset (struct sMiniFBtimer* tmr);
double timerNow (struct sMiniFBtimer* tmr);
double timerDelta (struct sMiniFBtimer* tmr);
double timerGetFrequency();
double timerGetResolution();

// To be able to distinguish these C++ functions, using std::function, from C functions, using raw function pointers,
// we need to reverse params order.
// Note that FROM the compiler point of view
//   mfb_set_XXXCallback(window, &my_c_func)
// and
//   mfb_set_XXXCallback(window, [](...) {})
// have the same parameters.
void setActiveCallback (std::function <void (sMiniWindow*, bool)> func, sMiniWindow* window);
void setResizeCallback (std::function <void (sMiniWindow*, int, int)> func, sMiniWindow* window);
void setCloseCallback (std::function <bool (sMiniWindow*)> func, sMiniWindow* window);

void setKeyboardCallback (std::function <void (sMiniWindow*, mfb_key, mfb_key_mod, bool)> func,
                          sMiniWindow* window);
void setCharInputCallback (std::function <void (sMiniWindow*, unsigned int)> func,
                           sMiniWindow* window);

void setMouseButtonCallback (std::function <void (sMiniWindow*, mfb_mouse_button, mfb_key_mod, bool)> func,
                             sMiniWindow* window);
void setMouseMoveCallback (std::function <void (sMiniWindow*, int, int, int, int)>func,
                           sMiniWindow* window);
void setMouseScrollCallback (std::function <void (sMiniWindow*, mfb_key_mod, float, float)> func,
                             sMiniWindow* window);

template <class T> void setActiveCallback (sMiniWindow* window, T* obj,
                                           void (T::*method)(sMiniWindow*, bool));
template <class T> void setResizeCallback (sMiniWindow* window, T* obj,
                                           void (T::*method)(sMiniWindow*, int, int));

template <class T> void setKeyboardCallback (sMiniWindow* window, T* obj,
                                             void (T::*method)(sMiniWindow*, mfb_key, mfb_key_mod, bool));
template <class T> void setCharInputCallback (sMiniWindow* window, T* obj,
                                              void (T::*method)(sMiniWindow*, unsigned int));

template <class T> void setMouseButtonCallback (sMiniWindow* window, T* obj,
                                                void (T::*method)(sMiniWindow*, mfb_mouse_button, mfb_key_mod, bool));
template <class T> void setMouseMoveCallback (sMiniWindow* window, T* obj,
                                              void (T::*method)(sMiniWindow*, int, int, int, int));
template <class T> void setMouseScrollCallback (sMiniWindow* window, T* obj,
                                                void (T::*method)(sMiniWindow*, mfb_key_mod, float, float));

// To avoid clumsy hands
//{{{
class mfbStub {
  mfbStub() : m_window(0x0) {}

  // friends
  friend void setActiveCallback (std::function <void (sMiniWindow* window, bool)> func,
                                      sMiniWindow* window);
  friend void setResizeCallback (std::function <void (sMiniWindow*, int, int)> func,
                                      sMiniWindow* window);
  friend void setCloseCallback (std::function <bool (sMiniWindow*)> func,
                                     sMiniWindow* window);

  friend void setKeyboardCallback (std::function <void (sMiniWindow*, mfb_key, mfb_key_mod, bool)> func,
                                         sMiniWindow* window);
  friend void setCharInputCallback (std::function <void (sMiniWindow*, unsigned int)> func,
                                          sMiniWindow* window);

  friend void setMouseButtonCallback (std::function <void (sMiniWindow*, mfb_mouse_button, mfb_key_mod, bool)> func,
                                            sMiniWindow* window);
  friend void setMouseMoveCallback (std::function <void (sMiniWindow*, int, int, int, int)> func,
                                          sMiniWindow* window);
  friend void setMouseScrollCallback (std::function <void (sMiniWindow*, mfb_key_mod, float, float)> func,
                                            sMiniWindow* window);

  // templates
  template <class T> friend void setActiveCallback (sMiniWindow* window, T* obj,
                                                         void (T::*method)(sMiniWindow*, bool));
  template <class T> friend void setResizeCallback (sMiniWindow* window, T* obj,
                                                         void (T::*method)(sMiniWindow*, int, int));
  template <class T> friend void setCloseCallback (sMiniWindow* window, T* obj,
                                                        bool (T::*method)(sMiniWindow* ));

  template <class T> friend void setKeyboardCallback (sMiniWindow* window, T* obj,
                                                           void (T::*method)(sMiniWindow*, mfb_key, mfb_key_mod, bool));
  template <class T> friend void setCharInputCallback (sMiniWindow* window, T* obj,
                                                             void (T::*method)(sMiniWindow*, unsigned int));

  template <class T> friend void setMouseButtonCallback (sMiniWindow* window, T* obj,
                                                         void (T::*method)(sMiniWindow*, mfb_mouse_button, mfb_key_mod, bool));
  template <class T> friend void setMouseMoveCallback (sMiniWindow* window, T* obj,
                                                       void (T::*method)(sMiniWindow*, int, int, int, int));
  template <class T> friend void setMouseScrollCallback (sMiniWindow* window, T* obj,
                                                         void (T::*method)(sMiniWindow*, mfb_key_mod, float, float));

  // statics
  static mfbStub* GetInstance (sMiniWindow* window);

  static void activeStub (sMiniWindow* window, bool isActive);
  static void resizeStub (sMiniWindow* window, int width, int height);
  static bool closeStub (sMiniWindow* window);

  static void keyboardStub (sMiniWindow* window, mfb_key key, mfb_key_mod mod, bool isPressed);
  static void charInputStub (sMiniWindow* window, unsigned int);

  static void mouseButtonStub (sMiniWindow* window, mfb_mouse_button button, mfb_key_mod mod, bool isPressed);
  static void mouseMoveStub (sMiniWindow* window, int x, int y, int pressure, int timestamp);
  static void scrollStub (sMiniWindow* window, mfb_key_mod mod, float deltaX, float deltaY);

  // vars
  sMiniWindow* m_window;

  std::function <void (sMiniWindow* window, bool)> m_active;
  std::function <void (sMiniWindow* window, int, int)> m_resize;
  std::function <bool (sMiniWindow* window)> m_close;

  std::function <void (sMiniWindow* window, mfb_key, mfb_key_mod, bool)> m_keyboard;
  std::function <void (sMiniWindow* window, unsigned int)> m_char_input;

  std::function <void (sMiniWindow* window, mfb_mouse_button, mfb_key_mod, bool)> m_mouse_btn;
  std::function <void (sMiniWindow* window, int, int, int, int)> m_mouse_move;
  std::function <void (sMiniWindow* window, mfb_key_mod, float, float)> m_scroll;
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
template <class T> inline void setKeyboardCallback (sMiniWindow* window, T* obj,
                                                         void (T::*method)(sMiniWindow* window, mfb_key, mfb_key_mod, bool)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance(window);
  stub->m_keyboard = std::bind(method, obj, _1, _2, _3, _4);

  setKeyboardCallback (window, mfbStub::keyboardStub);
  }
//}}}
//{{{
template <class T> inline void setCharInputCallback (sMiniWindow* window, T* obj,
                                                           void (T::*method)(sMiniWindow* window, unsigned int)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_char_input = std::bind (method, obj, _1, _2);

  setCharInputCallback (window, mfbStub::charInputStub);
  }
//}}}

//{{{
template <class T> inline void setMouseButtonCallback (sMiniWindow* window, T* obj,
                                                             void (T::*method)(sMiniWindow* window, mfb_mouse_button, mfb_key_mod, bool)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_mouse_btn = std::bind (method, obj, _1, _2, _3, _4);

  setMouseButtonCallback (window, mfbStub::mouseButtonStub);
  }
//}}}
//{{{
template <class T> inline void setMouseMoveCallback (sMiniWindow* window, T* obj,
                                                     void (T::*method)(sMiniWindow* window, int, int, int, int)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_mouse_move = std::bind (method, obj, _1, _2, _3, _4, _5);

  setMouseMoveCallback (window, mfbStub::mouseMoveStub);
  }
//}}}
//{{{
template <class T> inline void setMouseScrollCallback (sMiniWindow* window, T* obj,
                                                             void (T::*method)(sMiniWindow* window, mfb_key_mod, float, float)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_scroll = std::bind (method, obj, _1, _2, _3, _4);

  setMouseScrollCallback (window, mfbStub::scrollStub);
  }
//}}}
