// miniFB.h - main interface to miniFB
#pragma once
#include <functional>
#include "miniFBenums.h"
#include "sInfo.h"

#define MFB_RGB(r,g,b) (((uint32_t)r) << 16) | (((uint32_t)g) << 8) | ((uint32_t)b)


// func callbacks

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

int64_t getPointerTimestamp (sOpaqueInfo* opaqueInfo); // last pointer timestamp
int getPointerX (sOpaqueInfo* opaqueInfo);             // last pointer pos X
int getPointerY (sOpaqueInfo* opaqueInfo);             // last pointer pos Y
int getPointerPressure (sOpaqueInfo* opaqueInfo);      // last pointer pressure
float getPointerWheelX (sOpaqueInfo* opaqueInfo);      // last pointer wheel X as a sum. When you call this function it resets.
float getPointerWheelY (sOpaqueInfo* opaqueInfo);      // last pointer wheel Y as a sum. When you call this function it resets.

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

//{{{  callbacks
void setActiveCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setResizeCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setCloseCallback  (sOpaqueInfo* opaqueInfo, closeFuncType callback);
void setKeyCallback    (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setCharCallback   (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setButtonCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setMoveCallback   (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setWheelCallback  (sOpaqueInfo* opaqueInfo, infoFuncType callback);
void setEnterCallback  (sOpaqueInfo* opaqueInfo, infoFuncType callback);

// lambda callbacks
void setActiveCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setResizeCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setCloseCallback  (std::function <bool (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setKeyCallback    (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setCharCallback   (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setButtonCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setMoveCallback   (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setWheelCallback  (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
void setEnterCallback  (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);

//{{{
class cStub {
  cStub() {}

  // friends
  friend void setActiveCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setResizeCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setCloseCallback  (std::function <bool (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setKeyCallback    (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setCharCallback   (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setButtonCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setMoveCallback   (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setWheelCallback  (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);
  friend void setEnterCallback  (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo);

  // statics
  static cStub* getInstance (sOpaqueInfo* opaqueInfo);

  static void activeStub (sOpaqueInfo* opaqueInfo);
  static void resizeStub (sOpaqueInfo* opaqueInfo);
  static bool closeStub  (sOpaqueInfo* opaqueInfo);
  static void keyStub    (sOpaqueInfo* opaqueInfo);
  static void charStub   (sOpaqueInfo* opaqueInfo);
  static void buttonStub (sOpaqueInfo* opaqueInfo);
  static void moveStub   (sOpaqueInfo* opaqueInfo);
  static void wheelStub  (sOpaqueInfo* opaqueInfo);
  static void enterStub  (sOpaqueInfo* opaqueInfo);

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
//}}}
