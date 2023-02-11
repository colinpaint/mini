// cMiniWindow.cpp
//{{{  includes
#include "cWindow.h"

#include <chrono>
#include "../common/date.h"
#include "../common/basicTypes.h"
#include "../common/utils.h"
#include "../common/cLog.h"

#include "../miniFB/miniFB.h"
#include "../miniFB/sInfo.h"

using namespace std;
using namespace chrono;
//}}}

// actions
void cWindow::resized() {}
void cWindow::toggleFullScreen() {} // not yet

// protected
//{{{
bool cWindow::createWindow (const string& title, uint32_t width, uint32_t height,
                            chrono::milliseconds tickMs, bool fullScreen) {
  (void)tickMs;
  (void)fullScreen;

  #ifdef _WIN32
    TIME_ZONE_INFORMATION timeZoneInfo;
    if (GetTimeZoneInformation (&timeZoneInfo) == TIME_ZONE_ID_DAYLIGHT)
      mDayLightSeconds = -timeZoneInfo.DaylightBias * 60;
  #else
    cLog::log (LOGERROR, fmt::format ("No timezone correction for Linux yet"));
  #endif

  mWindow = openEx (title.c_str(), width, height, WF_RESIZABLE);
  if (!mWindow)
    return false;

  // create texture static resources after window, may use its openGL resources in future
  cDrawTexture::createStaticResources (getBoxHeight() * 4.0f / 5.0f);

  // create cTexture pixels on the heap
  createPixels (width, height);

  // state callbacks
  //{{{
  setActiveCallback ([&](sOpaqueInfo* opaqueInfo) {
      cLog::log (LOGINFO, fmt::format ("active {}", ((sInfo*)(opaqueInfo))->isActive));
      },
    mWindow);
  //}}}
  //{{{
  setResizeCallback ([&](struct sOpaqueInfo* opaqueInfo) {

      int width = ((sInfo*)(opaqueInfo))->windowScaledWidth;
      int height = ((sInfo*)(opaqueInfo))->windowScaledHeight;
      cLog::log (LOGINFO, fmt::format ("resize {} {}", width, height));
      uint32_t x = 0;
      if (width > getWidth()) {
        x = (width - getWidth()) >> 1;
        setWidth (width);
        }
      uint32_t y = 0;
      if (height > (int)getHeight()) {
        y = (height - getHeight()) >> 1;
        setHeight (height);
        }
      setViewport (opaqueInfo, x, y, width, height);
      },
    mWindow);
  //}}}
  //{{{
  setCloseCallback ([&](sOpaqueInfo* opaqueInfo) {
      (void)opaqueInfo;
      cLog::log (LOGINFO, fmt::format ("close"));
      return true; // false for don't close
      },
    mWindow);
  //}}}

  // keyboard callbacks
  //{{{
  setKeyCallback ([&](sOpaqueInfo* opaqueInfo) {

      if (((sInfo*)(opaqueInfo))->keyCode == KB_KEY_ESCAPE)
        close (opaqueInfo);

      if (((sInfo*)(opaqueInfo))->isPressed)
        if (!keyDown (((sInfo*)(opaqueInfo))->keyCode))
          cLog::log (LOGINFO, fmt::format ("keyboard key:{} pressed:{} mod:{}",
                                           getKeyName (((sInfo*)(opaqueInfo))->keyCode),
                                           ((sInfo*)(opaqueInfo))->isPressed,
                                           (int)((sInfo*)(opaqueInfo))->modifierKeys));
      },

    mWindow);
  //}}}
  //{{{
  setCharCallback ([&](sOpaqueInfo* opaqueInfo, uint32_t charCode) {
      (void)opaqueInfo;
      cLog::log (LOGINFO, fmt::format ("char code:{}", charCode));
      },
    mWindow);
  //}}}

  // mouse callbacks
  //{{{
  setPointerButtonCallback ([&](sOpaqueInfo* opaqueInfo, ePointerButton button, eKeyModifier mod, bool isPressed) {

      (void)mod;
      //cLog::log (LOGINFO, fmt::format ("mouseButton {} button:{} pressed:{} at:{} {} mod:{}",
      //                                 info ? (const char*)mfb_get_user_data(info) : "",
      //                                 (int)button, isPressed,
      //                                 mfb_get_mouse_x (info), mfb_get_mouse_y (info),
      //                                 (int)mod));
      if (isPressed) {
        mMousePress = true;
        mMouseMoved = false;
        mMousePressPos = cPoint ((float)getPointerX (opaqueInfo), (float)getPointerY (opaqueInfo));
        mMousePressRight = button != 0;
        mMouseLastPos = mMousePressPos;
        mMousePressUsed = mouseDown (mMousePressRight, mMousePressPos);
        if (mMousePressUsed)
          changed();
        cursorChanged();
        }
      else {
        mMouseLastPos = cPoint (getPointerX (opaqueInfo), getPointerY (opaqueInfo));
        if (mouseUp (mMousePressRight, mMouseMoved, mMouseLastPos))
          changed();
        mMousePress = false;
        mMousePressUsed = false;
        cursorChanged();
        }
      },
    mWindow);
  //}}}
  //{{{
  setPointerMoveCallback ([&](sOpaqueInfo* opaqueInfo, int x, int y, int pressure, int timestamp) {
      (void)opaqueInfo;

      //cLog::log (LOGINFO, fmt::format ("mouseMove x:{} y:{} press:{} time:{}", x, y, pressure, timestamp));
      mMousePos.x = (float)x;
      mMousePos.y = (float)y;
      if (mMousePress) {
        mMouseMoved = true;
        if (mouseMove (mMousePressRight, mMousePos, mMousePos - mMouseLastPos, pressure, timestamp))
          changed();
        mMouseLastPos = mMousePos;
        }
      else if (mouseProx (true, mMousePos))
        changed();
      cursorChanged();
      },

    mWindow);
  //}}}
  //{{{
  setPointerWheelCallback ([&](sOpaqueInfo* opaqueInfo, eKeyModifier mod, float deltaX, float deltaY) {
      // lambda
      (void)opaqueInfo;
      (void)mod;
      (void)deltaX;

      //cLog::log (LOGINFO, fmt::format ("mouseWheel {} x:{} y:{} mod:", info ? (const char*)mfb_get_user_data (info) : "",
      //                                 deltaX, deltaY, (int)mod));
      mScale *= (deltaY > 0.f) ? 1.05f : 1.f / 1.05f;
      cLog::log (LOGINFO, fmt::format ("mouseWheel problem - deltaY:{} int(deltaY):{}", deltaY, int(deltaY)));
      if (mouseWheel ((int)deltaY, mMousePos))
        changed();

      cursorChanged();
      },
    mWindow);
  //}}}
  //{{{
  setPointerEnterCallback ([&](sOpaqueInfo* opaqueInfo, bool enter) {
      // lambda
      (void)opaqueInfo;
      cLog::log (LOGINFO, fmt::format ("pointerEnter {}", enter));
      },
    mWindow);
  //}}}

  if (tickMs != 0ms) {
    //{{{  launch tick clock thread
    thread ([=, this]() {
      cLog::setThreadName ("tick");

      while (!mExit) {
        this_thread::sleep_for (tickMs);
        changed();
        }
      }).detach();
    }
    //}}}

  return true;
  }
//}}}
//{{{
void cWindow::uiLoop (bool useChanged, bool drawPerf,
                      const cColor& bgndColor, const cColor& perfColor,
                      const function <void(bool)>& drawCallback) {

  // ensure early update
  changed();

  int64_t frameUs = 0;
  while (!mExit && (updateEvents (mWindow) == STATE_OK)) {
    if (!useChanged || mChanged) {
      system_clock::time_point time = system_clock::now();

      clear (bgndColor);
      drawCallback (true);
      drawBoxes();

      if (drawPerf) {
        int64_t renderUs = duration_cast<microseconds>(system_clock::now() - time).count();
        drawRectangle (kGreen, {0.f, (float)getHeight() - 4.f, (frameUs * getWidth())/ 100000.f, (float)getHeight()});
        drawRectangle (kYellow, {0.f, (float)getHeight() - 4.f, (renderUs * getWidth())/ 100000.f, (float)getHeight()});
        drawText (perfColor, {0.f, getHeight() - getBoxHeight(), (float)getWidth(), getBoxHeight()},
                  fmt::format ("{:05d}:{:05d}us {} chars", renderUs, frameUs, getNumFontChars()));
        }

      // update window with our texture
      update (mWindow, getPixels());
      frameUs = duration_cast<microseconds>(system_clock::now() - time).count();
      if (!useChanged)
        waitSync (mWindow);
      }
    else {
      this_thread::sleep_for (1ms);
      drawCallback (false);
      }
    }
  }
//}}}
