// miniFB.h - main interface to miniFB
#pragma once
#include "miniFBenums.h"
#include <functional>

#define MFB_RGB(r,g,b) (((uint32_t)r) << 16) | (((uint32_t)g) << 8) | ((uint32_t)b)

struct sOpaqueInfo;

sOpaqueInfo* open (const char* title, unsigned width, unsigned height);
sOpaqueInfo* openEx (const char* title, unsigned width, unsigned height, unsigned flags);

eUpdateState update (sOpaqueInfo* opaqueInfo, void* buffer);
eUpdateState updateEx (sOpaqueInfo* opaqueInfo, void* buffer, unsigned width, unsigned height);
eUpdateState updateEvents (sOpaqueInfo* opaqueInfo);

void close (sOpaqueInfo* opaqueInfo);

// gets
bool isWindowActive (sOpaqueInfo* opaqueInfo);
unsigned getWindowWidth (sOpaqueInfo* opaqueInfo);
unsigned getWindowHeight (sOpaqueInfo* opaqueInfo);
void getMonitorScale (sOpaqueInfo* opaqueInfo, float* scale_x, float* scale_y);

int getPointerX (sOpaqueInfo* opaqueInfo);          // Last Pointer pos X
int getPointerY (sOpaqueInfo* opaqueInfo);          // Last Pointer pos Y
int getPointerPressure (sOpaqueInfo* opaqueInfo);   // Last Pointer pressure
int64_t getPointerTimestamp (sOpaqueInfo* opaqueInfo);  // Last Pointer timestamp

float getPointerWheelX (sOpaqueInfo* opaqueInfo); // Pointer wheel X as a sum. When you call this function it resets.
float getPointerWheelY (sOpaqueInfo* opaqueInfo); // Pointer wheel Y as a sum. When you call this function it resets.

const uint8_t* getPointerButtonBuffer (sOpaqueInfo* opaqueInfo); // One byte for every button. Press (1), Release 0. (up to 8 buttons)
const uint8_t* getKeyBuffer (sOpaqueInfo* opaqueInfo);           // One byte for every key. Press (1), Release 0.
const char* getKeyName (eKey key);

void* getUserData (sOpaqueInfo* opaqueInfo);

// sets
void setUserData (sOpaqueInfo* opaqueInfo, void* user_data);
bool setViewport (sOpaqueInfo* opaqueInfo, unsigned offset_x, unsigned offset_y, unsigned width, unsigned height);
bool setViewportBestFit (sOpaqueInfo* opaqueInfo, unsigned old_width, unsigned old_height);

// fps
void setTargetFps (uint32_t fps);
unsigned getTargetFps();

bool waitSync (sOpaqueInfo* opaqueInfo);

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
typedef void(*infoFuncType)(sOpaqueInfo* opaqueInfo);
typedef bool(*closeFuncType)(sOpaqueInfo* opaqueInfo);

typedef void(*keyFuncType)(sOpaqueInfo* opaqueInfo);
typedef void(*charFuncType)(sOpaqueInfo* opaqueInfo, unsigned int code);

typedef void(*pointerButtonFuncType)(sOpaqueInfo* opaqueInfo, ePointerButton button, eKeyModifier mod, bool isPressed);
typedef void(*pointerMoveFuncType)(sOpaqueInfo* opaqueInfo, int x, int y, int pressure, int timestamp);
typedef void(*pointerWheelFuncType)(sOpaqueInfo* opaqueInfo, eKeyModifier mod, float deltaX, float deltaY);
typedef void(*pointerEnterFuncType)(sOpaqueInfo* opaqueInfo, bool enter);

void setActiveCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setResizeCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setCloseCallback (sOpaqueInfo* opaqueInfo, closeFuncType callback);

void setKeyCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setCharCallback (sOpaqueInfo* opaqueInfo, charFuncType callback);

void setPointerButtonCallback (sOpaqueInfo* opaqueInfo, pointerButtonFuncType callback);
void setPointerMoveCallback (sOpaqueInfo* opaqueInfo, pointerMoveFuncType callback);
void setPointerWheelCallback (sOpaqueInfo* opaqueInfo, pointerWheelFuncType callback);
void setPointerEnterCallback (sOpaqueInfo* opaqueInfo, pointerEnterFuncType callback);

// lambda callbacks
void setActiveCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setResizeCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setCloseCallback (std::function <bool (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);

void setKeyCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setCharCallback (std::function <void (sOpaqueInfo*, unsigned int)> func, sOpaqueInfo* opaqueInfo);

void setPointerButtonCallback (std::function <void (sOpaqueInfo*, ePointerButton, eKeyModifier, bool)> func, sOpaqueInfo* opaqueInfo);
void setPointerMoveCallback (std::function <void (sOpaqueInfo*, int, int, int, int)>func, sOpaqueInfo* opaqueInfo);
void setPointerWheelCallback (std::function <void (sOpaqueInfo*, eKeyModifier, float, float)> func, sOpaqueInfo* opaqueInfo);
void setPointerEnterCallback (std::function <void (sOpaqueInfo*, bool)> func, sOpaqueInfo* opaqueInfo);

// templates
template <class T> void setActiveCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
template <class T> void setResizeCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
template <class T> void setCloseCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));

template <class T> void setKeyCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
template <class T> void setCharCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*, unsigned int));

template <class T> void setPointerButtonCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*, ePointerButton, eKeyModifier, bool));
template <class T> void setPointerMoveCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*, int, int, int, int));
template <class T> void setPointerWheelCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*, eKeyModifier, float, float));
template <class T> void setPointerEnterCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*, bool));

//{{{
class cStub {
  cStub() : m_opaqueInfo(nullptr) {}

  // friends
  friend void setActiveCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setResizeCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setCloseCallback (std::function <bool (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);

  friend void setKeyCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setCharCallback (std::function <void (sOpaqueInfo*, unsigned int)> func, sOpaqueInfo* opaqueInfo);

  friend void setPointerButtonCallback (std::function <void (sOpaqueInfo*, ePointerButton, eKeyModifier, bool)> func, sOpaqueInfo* opaqueInfo);
  friend void setPointerMoveCallback (std::function <void (sOpaqueInfo*, int, int, int, int)> func, sOpaqueInfo* opaqueInfo);
  friend void setPointerWheelCallback (std::function <void (sOpaqueInfo*, eKeyModifier, float, float)> func, sOpaqueInfo* opaqueInfo);
  friend void setPointerEnterCallback (std::function <void (sOpaqueInfo*, bool)> func, sOpaqueInfo* opaqueInfo);

  // templates
  template <class T> friend void setActiveCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
  template <class T> friend void setResizeCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
  template <class T> friend void setCloseCallback (sOpaqueInfo* opaqueInfo, T* obj, bool (T::*method)(sOpaqueInfo* ));

  template <class T> friend void setKeyCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
  template <class T> friend void setCharCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*, unsigned int));

  template <class T> friend void setPointerButtonCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*, ePointerButton, eKeyModifier, bool));
  template <class T> friend void setPointerMoveCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*, int, int, int, int));
  template <class T> friend void setPointerWheelCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*, eKeyModifier, float, float));
  template <class T> friend void setPointerEnterCallback (sOpaqueInfo* wndow, T* obj, void (T::*method)(sOpaqueInfo*, bool));

  // statics
  static cStub* GetInstance (sOpaqueInfo* opaqueInfo);

  static void activeStub (sOpaqueInfo* opaqueInfo);
  static void resizeStub (sOpaqueInfo* opaqueInfo);
  static bool closeStub (sOpaqueInfo* opaqueInfo);

  static void keyStub (sOpaqueInfo* opaqueInfo);
  static void charStub (sOpaqueInfo* opaqueInfo, unsigned int);

  static void pointerButtonStub (sOpaqueInfo* opaqueInfo, ePointerButton button, eKeyModifier mod, bool isPressed);
  static void pointerMoveStub (sOpaqueInfo* opaqueInfo, int x, int y, int pressure, int timestamp);
  static void pointerWheelStub (sOpaqueInfo* opaqueInfo, eKeyModifier mod, float deltaX, float deltaY);
  static void pointerEnterStub (sOpaqueInfo* opaqueInfo, bool);

  // vars
  sOpaqueInfo* m_opaqueInfo;

  std::function <void (sOpaqueInfo* opaqueInfo)> m_active;
  std::function <void (sOpaqueInfo* opaqueInfo)> m_resize;
  std::function <bool (sOpaqueInfo* opaqueInfo)> m_close;

  std::function <void (sOpaqueInfo* opaqueInfo)> m_key;
  std::function <void (sOpaqueInfo* opaqueInfo, unsigned int)> m_char;

  std::function <void (sOpaqueInfo* opaqueInfo, ePointerButton, eKeyModifier, bool)> m_pointer_button;
  std::function <void (sOpaqueInfo* opaqueInfo, int, int, int, int)> m_pointer_move;
  std::function <void (sOpaqueInfo* opaqueInfo, eKeyModifier, float, float)> m_pointer_wheel;
  std::function <void (sOpaqueInfo* opaqueInfo, bool)> mPointerEnter;
  };
//}}}

//{{{
template <class T> inline void setActiveCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->m_active = std::bind (method, obj, _1);

  setActiveCallback (opaqueInfo, cStub::infoStub);
  }
//}}}
//{{{
template <class T> inline void setResizeCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->m_resize = std::bind (method, obj, _1);

  setResizeCallback (opaqueInfo, cStub::infoStub);
  }
//}}}
//{{{
template <class T> inline void setCloseCallback (sOpaqueInfo* opaqueInfo, T* obj,
                                                 bool (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->m_close = std::bind (method, obj, _1);

  setCloseCallback (opaqueInfo, cStub::closeStub);
  }
//}}}

//{{{
template <class T> inline void setKeyCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance(opaqueInfo);
  stub->m_key = std::bind(method, obj, _1);

  setKeyCallback (opaqueInfo, cStub::keyStub);
  }
//}}}
//{{{
template <class T> inline void setCharCallback (sOpaqueInfo* opaqueInfo, T* obj,
                                                void (T::*method)(sOpaqueInfo* opaqueInfo, unsigned int)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->m_char = std::bind (method, obj, _1, _2);

  setCharCallback (opaqueInfo, cStub::charStub);
  }
//}}}

//{{{
template <class T> inline void setPointerButtonCallback (sOpaqueInfo* opaqueInfo, T* obj,
                                                       void (T::*method)(sOpaqueInfo* opaqueInfo, ePointerButton, eKeyModifier, bool)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->m_pointer_button = std::bind (method, obj, _1, _2, _3, _4);

  setPointerButtonCallback (opaqueInfo, cStub::pointerButtonStub);
  }
//}}}
//{{{
template <class T> inline void setPointerMoveCallback (sOpaqueInfo* opaqueInfo, T* obj,
                                                     void (T::*method)(sOpaqueInfo* opaqueInfo, int, int, int, int)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->m_pointer_move = std::bind (method, obj, _1, _2, _3, _4, _5);

  setPointerMoveCallback (opaqueInfo, cStub::pointerMoveStub);
  }
//}}}
//{{{
template <class T> inline void setPointerWheelCallback (sOpaqueInfo* opaqueInfo, T* obj,
                                                      void (T::*method)(sOpaqueInfo* opaqueInfo, eKeyModifier, float, float)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->m_pointer_wheel = std::bind (method, obj, _1, _2, _3, _4);

  setPointerWheelCallback (opaqueInfo, cStub::pointerWheelStub);
  }
//}}}
//{{{
template <class T> inline void setPointerEnterCallback (sOpaqueInfo* opaqueInfo, T* obj,
                                                       void (T::*method)(sOpaqueInfo* opaqueInfo, bool)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->mPointerEnter = std::bind (method, obj, _1, _2);

  setPointerEnterCallback (opaqueInfo, cStub::pointerEnterStub);
  }
//}}}
