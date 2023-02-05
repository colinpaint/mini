#pragma once
#include "miniFB.h"
#include <functional>

// To be able to distinguish these C++ functions, using std::function, from C functions, using raw function pointers,
// we need to reverse params order.
// Note that FROM the compiler point of view
//   mfb_set_XXXCallback(window, &my_c_func)
// and
//   mfb_set_XXXCallback(window, [](...) {})
// have the same parameters.
void mfb_set_activeCallback (std::function <void (struct mfb_window*, bool)> func, struct mfb_window* window);
void mfb_set_resizeCallback (std::function <void (struct mfb_window*, int, int)> func, struct mfb_window* window);
void mfb_set_closeCallback (std::function <bool (struct mfb_window*)> func, struct mfb_window* window);

void mfb_set_keyboardCallback (std::function <void (struct mfb_window*, mfb_key, mfb_key_mod, bool)> func,
                               struct mfb_window* window);
void mfb_set_char_inputCallback (std::function <void (struct mfb_window*, unsigned int)> func,
                                 struct mfb_window* window);

void mfb_set_mouse_buttonCallback (std::function <void (struct mfb_window*, mfb_mouse_button, mfb_key_mod, bool)> func,
                                   struct mfb_window* window);
void mfb_set_mouse_moveCallback (std::function <void (struct mfb_window*, int, int)>func,
                                 struct mfb_window* window);
void mfb_set_mouse_scrollCallback (std::function <void (struct mfb_window*, mfb_key_mod, float, float)> func,
                                   struct mfb_window* window);

template <class T> void mfb_set_activeCallback (struct mfb_window* window, T* obj,
                                                void (T::*method)(struct mfb_window*, bool));
template <class T> void mfb_set_resizeCallback (struct mfb_window* window, T* obj,
                                                void (T::*method)(struct mfb_window*, int, int));

template <class T> void mfb_set_keyboardCallback (struct mfb_window* window, T* obj,
                                                  void (T::*method)(struct mfb_window*, mfb_key, mfb_key_mod, bool));
template <class T> void mfb_set_char_inputCallback (struct mfb_window* window, T* obj,
                                                    void (T::*method)(struct mfb_window*, unsigned int));

template <class T> void mfb_set_mouse_buttonCallback (struct mfb_window* window, T* obj,
                                                      void (T::*method)(struct mfb_window*, mfb_mouse_button, mfb_key_mod, bool));
template <class T> void mfb_set_mouse_moveCallback (struct mfb_window* window, T* obj,
                                                    void (T::*method)(struct mfb_window*, int, int));
template <class T> void mfb_set_mouse_scrollCallback (struct mfb_window* window, T* obj,
                                                      void (T::*method)(struct mfb_window*, mfb_key_mod, float, float));

// To avoid clumsy hands
//{{{
class mfbStub {
  mfbStub() : m_window(0x0) {}

  // friends
  friend void mfb_set_activeCallback (std::function <void (struct mfb_window* window, bool)> func,
                                      struct mfb_window* window);
  friend void mfb_set_resizeCallback (std::function <void (struct mfb_window*, int, int)> func,
                                      struct mfb_window* window);
  friend void mfb_set_closeCallback (std::function <bool (struct mfb_window*)> func,
                                     struct mfb_window* window);

  friend void mfb_set_keyboardCallback (std::function <void (struct mfb_window*, mfb_key, mfb_key_mod, bool)> func,
                                         struct mfb_window* window);
  friend void mfb_set_char_inputCallback (std::function <void (struct mfb_window*, unsigned int)> func,
                                          struct mfb_window* window);

  friend void mfb_set_mouse_buttonCallback (std::function <void (struct mfb_window*, mfb_mouse_button, mfb_key_mod, bool)> func,
                                            struct mfb_window* window);
  friend void mfb_set_mouse_moveCallback (std::function <void (struct mfb_window*, int, int)> func,
                                          struct mfb_window* window);
  friend void mfb_set_mouse_scrollCallback (std::function <void (struct mfb_window*, mfb_key_mod, float, float)> func,
                                            struct mfb_window* window);

  // templates
  template <class T> friend void mfb_set_activeCallback (struct mfb_window* window, T* obj,
                                                         void (T::*method)(struct mfb_window*, bool));
  template <class T> friend void mfb_set_resizeCallback (struct mfb_window* window, T* obj,
                                                         void (T::*method)(struct mfb_window*, int, int));
  template <class T> friend void mfb_set_closeCallback (struct mfb_window* window, T* obj,
                                                        bool (T::*method)(struct mfb_window* ));

  template <class T> friend void mfb_set_keyboardCallback (struct mfb_window* window, T* obj,
                                                           void (T::*method)(struct mfb_window*, mfb_key, mfb_key_mod, bool));
  template <class T> friend void mfb_set_char_inputCallback (struct mfb_window* window, T* obj,
                                                             void (T::*method)(struct mfb_window*, unsigned int));

  template <class T> friend void mfb_set_mouse_buttonCallback (struct mfb_window* window, T* obj,
                                                               void (T::*method)(struct mfb_window*, mfb_mouse_button, mfb_key_mod, bool));
  template <class T> friend void mfb_set_mouse_moveCallback (struct mfb_window* window, T* obj,
                                                             void (T::*method)(struct mfb_window*, int, int));
  template <class T> friend void mfb_set_mouse_scrollCallback (struct mfb_window* window, T* obj,
                                                               void (T::*method)(struct mfb_window*, mfb_key_mod, float, float));

  // statics
  static mfbStub* GetInstance (struct mfb_window* window);

  static void activeStub (struct mfb_window* window, bool isActive);
  static void resizeStub (struct mfb_window* window, int width, int height);
  static bool closeStub (struct mfb_window* window);

  static void keyboardStub (struct mfb_window* window, mfb_key key, mfb_key_mod mod, bool isPressed);
  static void charInputStub (struct mfb_window* window, unsigned int);

  static void mouseButtonStub (struct mfb_window* window, mfb_mouse_button button, mfb_key_mod mod, bool isPressed);
  static void mouseMoveStub (struct mfb_window* window, int x, int y);
  static void scrollStub (struct mfb_window* window, mfb_key_mod mod, float deltaX, float deltaY);

  // vars
  struct mfb_window* m_window;

  std::function <void (struct mfb_window* window, bool)> m_active;
  std::function <void (struct mfb_window* window, int, int)> m_resize;
  std::function <bool (struct mfb_window* window)> m_close;

  std::function <void (struct mfb_window* window, mfb_key, mfb_key_mod, bool)> m_keyboard;
  std::function <void (struct mfb_window* window, unsigned int)> m_char_input;

  std::function <void (struct mfb_window* window, mfb_mouse_button, mfb_key_mod, bool)> m_mouse_btn;
  std::function <void (struct mfb_window* window, int, int)> m_mouse_move;
  std::function <void (struct mfb_window* window, mfb_key_mod, float, float)> m_scroll;
  };
//}}}

//{{{
template <class T> inline void mfb_set_activeCallback (struct mfb_window* window, T* obj,
                                                       void (T::*method)(struct mfb_window* window, bool)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_active = std::bind (method, obj, _1, _2);

  mfb_set_activeCallback (window, mfbStub::activeStub);
  }
//}}}
//{{{
template <class T> inline void mfb_set_resizeCallback (struct mfb_window* window, T* obj,
                                                       void (T::*method)(struct mfb_window* window, int, int)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_resize = std::bind (method, obj, _1, _2, _3);

  mfb_set_resizeCallback (window, mfbStub::resizeStub);
  }
//}}}
//{{{
template <class T> inline void mfb_set_closeCallback (struct mfb_window* window, T* obj,
                                                      bool (T::*method)(struct mfb_window* window)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_close = std::bind (method, obj, _1);

  mfb_set_closeCallback (window, mfbStub::closeStub);
  }
//}}}

//{{{
template <class T> inline void mfb_set_keyboardCallback (struct mfb_window* window, T* obj,
                                                         void (T::*method)(struct mfb_window* window, mfb_key, mfb_key_mod, bool)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance(window);
  stub->m_keyboard = std::bind(method, obj, _1, _2, _3, _4);

  mfb_set_keyboardCallback(window, mfbStub::keyboardStub);
  }
//}}}
//{{{
template <class T> inline void mfb_set_char_inputCallback (struct mfb_window* window, T* obj,
                                                           void (T::*method)(struct mfb_window* window, unsigned int)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_char_input = std::bind (method, obj, _1, _2);

  mfb_set_char_inputCallback (window, mfbStub::charInputStub);
  }
//}}}

//{{{
template <class T> inline void mfb_set_mouse_buttonCallback (struct mfb_window* window, T* obj,
                                                             void (T::*method)(struct mfb_window* window, mfb_mouse_button, mfb_key_mod, bool)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_mouse_btn = std::bind (method, obj, _1, _2, _3, _4);

  mfb_set_mouse_buttonCallback (window, mfbStub::mouseButtonStub);
  }
//}}}
//{{{
template <class T> inline void mfb_set_mouse_moveCallback (struct mfb_window* window, T* obj,
                                                           void (T::*method)(struct mfb_window* window, int, int)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_mouse_move = std::bind (method, obj, _1, _2, _3);

  mfb_set_mouse_moveCallback (window, mfbStub::mouseMoveStub);
  }
//}}}
//{{{
template <class T> inline void mfb_set_mouse_scrollCallback (struct mfb_window* window, T* obj,
                                                             void (T::*method)(struct mfb_window* window, mfb_key_mod, float, float)) {
  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_scroll = std::bind (method, obj, _1, _2, _3, _4);

  mfb_set_mouse_scrollCallback (window, mfbStub::scrollStub);
  }
//}}}
