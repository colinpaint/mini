// MiniFB.cpp
#include "miniFBcpp.h"
#include "miniFBenums.h"
#include <vector>

//{{{
mfbStub* mfbStub::GetInstance (struct mfb_window *window) {

  //{{{
  struct stub_vector {
    std::vector<mfbStub*> instances;

    stub_vector() = default;
    //{{{
    ~stub_vector() {
      for (mfbStub* instance : instances)
        delete instance;
      }
    //}}}

    mfbStub* Get (struct mfb_window *window) {
      for(mfbStub *instance : instances) {
        if(instance->m_window == window) {
          return instance;
          }
        }
      instances.push_back (new mfbStub);
      instances.back()->m_window = window;
      return instances.back();
      }
    };
  //}}}
  static stub_vector s_instances;

  return s_instances.Get (window);
  }
//}}}

// stubs
//{{{
void mfbStub::activeStub (struct mfb_window *window, bool isActive) {

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_active (window, isActive);
  }
//}}}
//{{{
void mfbStub::resizeStub (struct mfb_window *window, int width, int height) {

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_resize (window, width, height);
  }
//}}}
//{{{
bool mfbStub::closeStub (struct mfb_window *window) {

  mfbStub* stub = mfbStub::GetInstance (window);
  return stub->m_close (window);
  }
//}}}

//{{{
void mfbStub::keyboardStub (struct mfb_window *window, mfb_key key, mfb_key_mod mod, bool isPressed) {

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_keyboard (window, key, mod, isPressed);
  }
//}}}
//{{{
void mfbStub::charInputStub (struct mfb_window *window, unsigned int code) {

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_char_input (window, code);
  }
//}}}

//{{{
void mfbStub::mouseButtonStub (struct mfb_window *window, mfb_mouse_button button, mfb_key_mod mod, bool isPressed) {

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_mouse_btn (window, button, mod, isPressed);
  }
//}}}
//{{{
void mfbStub::mouseMoveStub (struct mfb_window *window, int x, int y) {

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_mouse_move (window, x, y);
  }
//}}}
//{{{
void mfbStub::scrollStub (struct mfb_window *window, mfb_key_mod mod, float deltaX, float deltaY) {

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_scroll (window, mod, deltaX, deltaY);
  }
//}}}

// set callbacks
//{{{
void mfb_set_activeCallback (std::function<void(struct mfb_window *, bool)> func, struct mfb_window *window) {

  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance(window);
  stub->m_active = std::bind (func, _1, _2);
  mfb_set_activeCallback (window, mfbStub::activeStub);
  }
//}}}
//{{{
void mfb_set_resizeCallback (std::function<void(struct mfb_window *, int, int)> func, struct mfb_window *window) {

  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance(window);
  stub->m_resize = std::bind(func, _1, _2, _3);
  mfb_set_resizeCallback(window, mfbStub::resizeStub);
  }
//}}}
//{{{
void mfb_set_closeCallback (std::function<bool(struct mfb_window *)> func, struct mfb_window *window) {

  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance(window);
  stub->m_close = std::bind(func, _1);
  mfb_set_closeCallback(window, mfbStub::closeStub);
  }
//}}}

//{{{
void mfb_set_keyboardCallback (std::function<void(struct mfb_window *, mfb_key, mfb_key_mod, bool)> func, struct mfb_window *window) {

  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance(window);
  stub->m_keyboard = std::bind (func, _1, _2, _3, _4);
  mfb_set_keyboardCallback (window, mfbStub::keyboardStub);
  }
//}}}
//{{{
void mfb_set_char_inputCallback (std::function<void(struct mfb_window *, unsigned int)> func, struct mfb_window *window) {

  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_char_input = std::bind (func, _1, _2);
  mfb_set_char_inputCallback (window, mfbStub::charInputStub);
  }
//}}}

//{{{
void mfb_set_mouse_buttonCallback (std::function<void(struct mfb_window *, mfb_mouse_button, mfb_key_mod, bool)> func, struct mfb_window *window) {

  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_mouse_btn = std::bind (func, _1, _2, _3, _4);
  mfb_set_mouse_buttonCallback (window, mfbStub::mouseButtonStub);
  }
//}}}
//{{{
void mfb_set_mouse_moveCallback (std::function<void(struct mfb_window *, int, int)> func, struct mfb_window *window) {

  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_mouse_move = std::bind (func, _1, _2, _3);
  mfb_set_mouse_moveCallback (window, mfbStub::mouseMoveStub);
  }
//}}}
//{{{
void mfb_set_mouse_scrollCallback (std::function<void(struct mfb_window *, mfb_key_mod, float, float)> func, struct mfb_window *window) {

  using namespace std::placeholders;

  mfbStub* stub = mfbStub::GetInstance (window);
  stub->m_scroll = std::bind (func, _1, _2, _3, _4);
  mfb_set_mouse_scrollCallback (window, mfbStub::scrollStub);
  }
//}}}
