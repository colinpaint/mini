// miniFB.h - main interface to miniFB
#pragma once
#include "miniFBenums.h"
#include "windowData.h"
#include <functional>

#define MFB_RGB(r,g,b) (((uint32_t)r) << 16) | (((uint32_t)g) << 8) | ((uint32_t)b)

struct sMiniFBwindow* open (const char* title, unsigned width, unsigned height);
struct sMiniFBwindow* openEx (const char* title, unsigned width, unsigned height, unsigned flags);

mfb_update_state update (struct sMiniFBwindow* window, void* buffer);
mfb_update_state updateEx (struct sMiniFBwindow* window, void* buffer, unsigned width, unsigned height);
mfb_update_state updateEvents (struct sMiniFBwindow* window);

void close (struct sMiniFBwindow* window);

// gets
bool mfbIsWindowActive (struct sMiniFBwindow* window);
unsigned getWindowWidth (struct sMiniFBwindow* window);
unsigned getWindowHeight (struct sMiniFBwindow* window);

int getMouseX (struct sMiniFBwindow* window);          // Last mouse pos X
int getMouseY (struct sMiniFBwindow* window);          // Last mouse pos Y
int getMousePressure (struct sMiniFBwindow* window);   // Last mouse pressure
int64_t getMouseTimestamp (struct sMiniFBwindow* window);  // Last mouse timestamp

float getMouseScrollX (struct sMiniFBwindow* window); // Mouse wheel X as a sum. When you call this function it resets.
float getMouseScrollY (struct sMiniFBwindow* window); // Mouse wheel Y as a sum. When you call this function it resets.

const uint8_t* getMouseButtonBuffer (struct sMiniFBwindow* window); // One byte for every button. Press (1), Release 0. (up to 8 buttons)
const uint8_t* getKeyBuffer (struct sMiniFBwindow* window);          // One byte for every key. Press (1), Release 0.
const char* getKeyName (mfb_key key);

void getMonitorScale (struct sMiniFBwindow* window, float* scale_x, float* scale_y);

// sets
void setUserData (struct sMiniFBwindow* window, void* user_data);
void* getUserData (struct sMiniFBwindow* window);

bool setViewport (struct sMiniFBwindow* window, unsigned offset_x, unsigned offset_y, unsigned width, unsigned height);
bool setViewportBestFit (struct sMiniFBwindow* window, unsigned old_width, unsigned old_height);

// fps
void setTargetFps (uint32_t fps);
unsigned getTargetFps();
bool waitSync (struct sMiniFBwindow* window);

// callbacks
void setActiveCallback (struct sMiniFBwindow* window, mfb_active_func callback);
void setResizeCallback (struct sMiniFBwindow* window, mfb_resize_func callback);
void setCloseCallback (struct sMiniFBwindow* window, mfb_close_func callback);

void setKeyboardCallback (struct sMiniFBwindow* window, mfb_keyboard_func callback);
void setCharInputCallback (struct sMiniFBwindow* window, mfb_char_input_func callback);

void setMouseButtonCallback (struct sMiniFBwindow* window, mfb_mouse_button_func callback);
void setMouseMoveCallback (struct sMiniFBwindow* window, mfb_mouse_move_func callback);
void setMouseScrollCallback (struct sMiniFBwindow* window, mfb_mouse_scroll_func callback);

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
void setActiveCallback (std::function <void (struct sMiniFBwindow*, bool)> func, struct sMiniFBwindow* window);
void setResizeCallback (std::function <void (struct sMiniFBwindow*, int, int)> func, struct sMiniFBwindow* window);
void setCloseCallback (std::function <bool (struct sMiniFBwindow*)> func, struct sMiniFBwindow* window);

void setKeyboardCallback (std::function <void (struct sMiniFBwindow*, mfb_key, mfb_key_mod, bool)> func,
                          struct sMiniFBwindow* window);
void setCharInputCallback (std::function <void (struct sMiniFBwindow*, unsigned int)> func,
                           struct sMiniFBwindow* window);

void setMouseButtonCallback (std::function <void (struct sMiniFBwindow*, mfb_mouse_button, mfb_key_mod, bool)> func,
                             struct sMiniFBwindow* window);
void setMouseMoveCallback (std::function <void (struct sMiniFBwindow*, int, int)>func,
                           struct sMiniFBwindow* window);
void setMouseScrollCallback (std::function <void (struct sMiniFBwindow*, mfb_key_mod, float, float)> func,
                             struct sMiniFBwindow* window);

template <class T> void setActiveCallback (struct sMiniFBwindow* window, T* obj,
                                           void (T::*method)(struct sMiniFBwindow*, bool));
template <class T> void setResizeCallback (struct sMiniFBwindow* window, T* obj,
                                           void (T::*method)(struct sMiniFBwindow*, int, int));

template <class T> void setKeyboardCallback (struct sMiniFBwindow* window, T* obj,
                                             void (T::*method)(struct sMiniFBwindow*, mfb_key, mfb_key_mod, bool));
template <class T> void setCharInputCallback (struct sMiniFBwindow* window, T* obj,
                                              void (T::*method)(struct sMiniFBwindow*, unsigned int));

template <class T> void setMouseButtonCallback (struct sMiniFBwindow* window, T* obj,
                                                void (T::*method)(struct sMiniFBwindow*, mfb_mouse_button, mfb_key_mod, bool));
template <class T> void setMouseMoveCallback (struct sMiniFBwindow* window, T* obj,
                                              void (T::*method)(struct sMiniFBwindow*, int, int));
template <class T> void setMouseScrollCallback (struct sMiniFBwindow* window, T* obj,
                                                void (T::*method)(struct sMiniFBwindow*, mfb_key_mod, float, float));

// To avoid clumsy hands
//{{{
class mfbStub {
  mfbStub() : m_window(0x0) {}

  // friends
  friend void setActiveCallback (std::function <void (struct sMiniFBwindow* window, bool)> func,
                                      struct sMiniFBwindow* window);
  friend void setResizeCallback (std::function <void (struct sMiniFBwindow*, int, int)> func,
                                      struct sMiniFBwindow* window);
  friend void setCloseCallback (std::function <bool (struct sMiniFBwindow*)> func,
                                     struct sMiniFBwindow* window);

  friend void setKeyboardCallback (std::function <void (struct sMiniFBwindow*, mfb_key, mfb_key_mod, bool)> func,
                                         struct sMiniFBwindow* window);
  friend void setCharInputCallback (std::function <void (struct sMiniFBwindow*, unsigned int)> func,
                                          struct sMiniFBwindow* window);

  friend void setMouseButtonCallback (std::function <void (struct sMiniFBwindow*, mfb_mouse_button, mfb_key_mod, bool)> func,
                                            struct sMiniFBwindow* window);
  friend void setMouseMoveCallback (std::function <void (struct sMiniFBwindow*, int, int)> func,
                                          struct sMiniFBwindow* window);
  friend void setMouseScrollCallback (std::function <void (struct sMiniFBwindow*, mfb_key_mod, float, float)> func,
                                            struct sMiniFBwindow* window);

  // templates
  template <class T> friend void setActiveCallback (struct sMiniFBwindow* window, T* obj,
                                                         void (T::*method)(struct sMiniFBwindow*, bool));
  template <class T> friend void setResizeCallback (struct sMiniFBwindow* window, T* obj,
                                                         void (T::*method)(struct sMiniFBwindow*, int, int));
  template <class T> friend void setCloseCallback (struct sMiniFBwindow* window, T* obj,
                                                        bool (T::*method)(struct sMiniFBwindow* ));

  template <class T> friend void setKeyboardCallback (struct sMiniFBwindow* window, T* obj,
                                                           void (T::*method)(struct sMiniFBwindow*, mfb_key, mfb_key_mod, bool));
  template <class T> friend void setCharInputCallback (struct sMiniFBwindow* window, T* obj,
                                                             void (T::*method)(struct sMiniFBwindow*, unsigned int));

  template <class T> friend void setMouseButtonCallback (struct sMiniFBwindow* window, T* obj,
                                                               void (T::*method)(struct sMiniFBwindow*, mfb_mouse_button, mfb_key_mod, bool));
  template <class T> friend void setMouseMoveCallback (struct sMiniFBwindow* window, T* obj,
                                                             void (T::*method)(struct sMiniFBwindow*, int, int));
  template <class T> friend void setMouseScrollCallback (struct sMiniFBwindow* window, T* obj,
                                                               void (T::*method)(struct sMiniFBwindow*, mfb_key_mod, float, float));

  // statics
  static mfbStub* GetInstance (struct sMiniFBwindow* window);

  static void activeStub (struct sMiniFBwindow* window, bool isActive);
  static void resizeStub (struct sMiniFBwindow* window, int width, int height);
  static bool closeStub (struct sMiniFBwindow* window);

  static void keyboardStub (struct sMiniFBwindow* window, mfb_key key, mfb_key_mod mod, bool isPressed);
  static void charInputStub (struct sMiniFBwindow* window, unsigned int);

  static void mouseButtonStub (struct sMiniFBwindow* window, mfb_mouse_button button, mfb_key_mod mod, bool isPressed);
  static void mouseMoveStub (struct sMiniFBwindow* window, int x, int y);
  static void scrollStub (struct sMiniFBwindow* window, mfb_key_mod mod, float deltaX, float deltaY);

  // vars
  struct sMiniFBwindow* m_window;

  std::function <void (struct sMiniFBwindow* window, bool)> m_active;
  std::function <void (struct sMiniFBwindow* window, int, int)> m_resize;
  std::function <bool (struct sMiniFBwindow* window)> m_close;

  std::function <void (struct sMiniFBwindow* window, mfb_key, mfb_key_mod, bool)> m_keyboard;
  std::function <void (struct sMiniFBwindow* window, unsigned int)> m_char_input;

  std::function <void (struct sMiniFBwindow* window, mfb_mouse_button, mfb_key_mod, bool)> m_mouse_btn;
  std::function <void (struct sMiniFBwindow* window, int, int)> m_mouse_move;
  std::function <void (struct sMiniFBwindow* window, mfb_key_mod, float, float)> m_scroll;
  };
//}}}

//{{{
template <class T> inline void setActiveCallback (struct sMiniFBwindow* window, T* obj,
                                                       void (T::*method)(struct sMiniFBwindow* window, bool)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_active = std::bind (method, obj, _1, _2);

  setActiveCallback (window, mfbStub::activeStub);
  }
//}}}
//{{{
template <class T> inline void setResizeCallback (struct sMiniFBwindow* window, T* obj,
                                                       void (T::*method)(struct sMiniFBwindow* window, int, int)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_resize = std::bind (method, obj, _1, _2, _3);

  setResizeCallback (window, mfbStub::resizeStub);
  }
//}}}
//{{{
template <class T> inline void setCloseCallback (struct sMiniFBwindow* window, T* obj,
                                                      bool (T::*method)(struct sMiniFBwindow* window)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_close = std::bind (method, obj, _1);

  setCloseCallback (window, mfbStub::closeStub);
  }
//}}}

//{{{
template <class T> inline void setKeyboardCallback (struct sMiniFBwindow* window, T* obj,
                                                         void (T::*method)(struct sMiniFBwindow* window, mfb_key, mfb_key_mod, bool)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance(window);
  stub->m_keyboard = std::bind(method, obj, _1, _2, _3, _4);

  setKeyboardCallback (window, mfbStub::keyboardStub);
  }
//}}}
//{{{
template <class T> inline void setCharInputCallback (struct sMiniFBwindow* window, T* obj,
                                                           void (T::*method)(struct sMiniFBwindow* window, unsigned int)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_char_input = std::bind (method, obj, _1, _2);

  setCharInputCallback (window, mfbStub::charInputStub);
  }
//}}}

//{{{
template <class T> inline void setMouseButtonCallback (struct sMiniFBwindow* window, T* obj,
                                                             void (T::*method)(struct sMiniFBwindow* window, mfb_mouse_button, mfb_key_mod, bool)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_mouse_btn = std::bind (method, obj, _1, _2, _3, _4);

  setMouseButtonCallback (window, mfbStub::mouseButtonStub);
  }
//}}}
//{{{
template <class T> inline void setMouseMoveCallback (struct sMiniFBwindow* window, T* obj,
                                                           void (T::*method)(struct sMiniFBwindow* window, int, int)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_mouse_move = std::bind (method, obj, _1, _2, _3);

  setMouseMoveCallback (window, mfbStub::mouseMoveStub);
  }
//}}}
//{{{
template <class T> inline void setMouseScrollCallback (struct sMiniFBwindow* window, T* obj,
                                                             void (T::*method)(struct sMiniFBwindow* window, mfb_key_mod, float, float)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_scroll = std::bind (method, obj, _1, _2, _3, _4);

  setMouseScrollCallback (window, mfbStub::scrollStub);
  }
//}}}
