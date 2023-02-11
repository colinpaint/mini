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

void setActiveCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setResizeCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setCloseCallback (sOpaqueInfo* opaqueInfo, closeFuncType callback);
void setKeyCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setCharCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setButtonCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setMoveCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setWheelCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setEnterCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);

// lambda callbacks
void setActiveCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setResizeCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setCloseCallback  (std::function <bool (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setKeyCallback  (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setCharCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setButtonCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setMoveCallback   (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setWheelCallback  (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setEnterCallback  (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);

// templates
template <class T> void setActiveCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
template <class T> void setResizeCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
template <class T> void setCloseCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
template <class T> void setKeyCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
template <class T> void setCharCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*t));
template <class T> void setButtonCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
template <class T> void setMoveCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
template <class T> void setWheelCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*t));
template <class T> void setEnterCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*l));

//{{{
class cStub {
  cStub() {}

  // friends
  friend void setActiveCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setResizeCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setCloseCallback (std::function <bool (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setKeyCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setCharCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setButtonCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setMoveCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setWheelCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setEnterCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);

  // templates
  template <class T> friend void setActiveCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
  template <class T> friend void setResizeCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
  template <class T> friend void setCloseCallback (sOpaqueInfo* opaqueInfo, T* obj, bool (T::*method)(sOpaqueInfo* ));
  template <class T> friend void setKeyCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
  template <class T> friend void setCharCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
  template <class T> friend void setButtonCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
  template <class T> friend void setMoveCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
  template <class T> friend void setWheelCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo*));
  template <class T> friend void setEnterCallback (sOpaqueInfo* wndow, T* obj, void (T::*method)(sOpaqueInfo*));

  // statics
  static cStub* getInstance (sOpaqueInfo* opaqueInfo);

  static void activeStub (sOpaqueInfo* opaqueInfo);
  static void resizeStub (sOpaqueInfo* opaqueInfo);
  static bool closeStub (sOpaqueInfo* opaqueInfo);
  static void keyStub (sOpaqueInfo* opaqueInfo);
  static void charStub (sOpaqueInfo* opaqueInfo);
  static void buttonStub (sOpaqueInfo* opaqueInfo);
  static void moveStub (sOpaqueInfo* opaqueInfo);
  static void wheelStub (sOpaqueInfo* opaqueInfo);
  static void enterStub (sOpaqueInfo* opaqueInfo);

  // vars
  sOpaqueInfo* m_opaqueInfo = nullptr;

  std::function <void (sOpaqueInfo* opaqueInfo)> mActiveFunc;
  std::function <void (sOpaqueInfo* opaqueInfo)> mResizeFunc;
  std::function <bool (sOpaqueInfo* opaqueInfo)> mCloseFunc;
  std::function <void (sOpaqueInfo* opaqueInfo)> mKeyFunc;
  std::function <void (sOpaqueInfo* opaqueInfo)> mCharFunc;
  std::function <void (sOpaqueInfo* opaqueInfo)> mButtonFunc;
  std::function <void (sOpaqueInfo* opaqueInfo)> mMoveFunc;
  std::function <void (sOpaqueInfo* opaqueInfo)> mWheelFunc;
  std::function <void (sOpaqueInfo* opaqueInfo)> mEnterFunc;
  };
//}}}

//{{{
template <class T> inline void setActiveCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->mActiveFunc = std::bind (method, obj, _1);

  setActiveCallback (opaqueInfo, cStub::infoStu);
  }
//}}}
//{{{
template <class T> inline void setResizeCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->mResizeFunc = std::bind (method, obj, _1);

  setResizeCallback (opaqueInfo, cStub::infoStub);
  }
//}}}
//{{{
template <class T> inline void setCloseCallback (sOpaqueInfo* opaqueInfo, T* obj, bool (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->mCloseFunc = std::bind (method, obj, _1);

  setCloseCallback (opaqueInfo, cStub::closeStub);
  }
//}}}
//{{{
template <class T> inline void setKeyCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance(opaqueInfo);
  stub->mKeyFunc = std::bind(method, obj, _1);

  setKeyCallback (opaqueInfo, cStub::keyStub);
  }
//}}}
//{{{
template <class T> inline void setCharCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->mCharFunc = std::bind (method, obj, _1);

  setCharCallback (opaqueInfo, cStub::charStub);
  }
//}}}
//{{{
template <class T> inline void setButtonCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->mButtonFunc = std::bind (method, obj, _1);

  setButtonCallback (opaqueInfo, cStub::buttonStub);
  }
//}}}
//{{{
template <class T> inline void setMoveCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->mMoveFunc = std::bind (method, obj, _1);

  setMoveCallback (opaqueInfo, cStub::moveStub);
  }
//}}}
//{{{
template <class T> inline void setWheelCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->mWheelFunc = std::bind (method, obj, _1);

  setWheelCallback (opaqueInfo, cStub::wheelStub);
  }
//}}}
//{{{
template <class T> inline void setEnterCallback (sOpaqueInfo* opaqueInfo, T* obj, void (T::*method)(sOpaqueInfo* opaqueInfo)) {
  using namespace std::placeholders;

  cStub* stub = cStub::GetInstance (opaqueInfo);
  stub->mEnterFunc = std::bind (method, obj, _1);

  setEnterCallback (opaqueInfo, cStub::enterStub);
  }
//}}}
