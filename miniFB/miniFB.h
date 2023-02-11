// miniFB.h - main interface to miniFB
#pragma once
#include <functional>
#include "miniFBenums.h"

struct sMiniFBtimer;
struct sInfo;
typedef void(*infoFuncType)(sInfo* info);
typedef bool(*closeFuncType)(sInfo* info);
//{{{
struct sInfo {
  void* userData;
  void* platformInfo;

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
  };
//}}}

sInfo* open (const char* title, unsigned width, unsigned height);
sInfo* openEx (const char* title, unsigned width, unsigned height, unsigned flags);

eUpdateState update (sInfo* info, void* buffer);
eUpdateState updateEx (sInfo* info, void* buffer, unsigned width, unsigned height);
eUpdateState updateEvents (sInfo* info);

void close (sInfo* info);

// gets
bool isWindowActive (sInfo* info);
unsigned getWindowWidth (sInfo* info);
unsigned getWindowHeight (sInfo* info);
void getMonitorScale (sInfo* info, float* scale_x, float* scale_y);

int64_t getPointerTimestamp (sInfo* info); // last pointer timestamp
int getPointerX (sInfo* info);             // last pointer pos X
int getPointerY (sInfo* info);             // last pointer pos Y
int getPointerPressure (sInfo* info);      // last pointer pressure
float getPointerWheelX (sInfo* info);      // last pointer wheel X as a sum. When you call this function it resets.
float getPointerWheelY (sInfo* info);      // last pointer wheel Y as a sum. When you call this function it resets.

const uint8_t* getPointerButtonBuffer (sInfo* info); // One byte for every button. Press (1), Release 0. (up to 8 buttons)
const uint8_t* getKeyBuffer (sInfo* info);           // One byte for every key. Press (1), Release 0.
const char* getKeyName (eKey key);

void* getUserData (sInfo* info);

// sets
void setUserData (sInfo* info, void* user_data);
bool setViewport (sInfo* info, unsigned offset_x, unsigned offset_y, unsigned width, unsigned height);
bool setViewportBestFit (sInfo* info, unsigned old_width, unsigned old_height);

// fps
void setTargetFps (uint32_t fps);
unsigned getTargetFps();

bool waitSync (sInfo* info);

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
