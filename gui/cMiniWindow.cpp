// cMiniWindow.cpp
//{{{  includes
#include "cWindow.h"

#include <chrono>
#include "../common/date.h"
#include "../common/basicTypes.h"
#include "../common/utils.h"
#include "../common/cLog.h"

#include "../miniFB/MiniFB.h"

using namespace std;
using namespace chrono;
//}}}

// cWindow
//  !!! not sure i have found this case yet !!!!
// miniFB misses the buttom release when off screen and tracking the mouse
//case WM_MOUSELEAVE: {
//  if (mouseProx (false, cPoint()))
//    changed();
//  mMouseTracking =  false;

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
  setActiveCallback ([&](sMiniWindow* window, bool isActive) {
      (void)window;
      cLog::log (LOGINFO, fmt::format ("active {}", isActive));
      },
    mWindow);
  //}}}
  //{{{
  setResizeCallback ([&](struct sMiniWindow* window, int width, int height) {
      (void)window;
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
      setViewport (window, x, y, width, height);
      },
    mWindow);
  //}}}
  //{{{
  setCloseCallback ([&](sMiniWindow* window) {
      (void)window;
      cLog::log (LOGINFO, fmt::format ("close"));
      return true; // false for don't close
      },
    mWindow);
  //}}}

  // keyboard callbacks
  //{{{
  setKeyboardCallback ([&](sMiniWindow* window, mfb_key key, mfb_key_mod mod, bool isPressed) {

      if (key == KB_KEY_ESCAPE)
        close (window);

      if (isPressed)
        if (!keyDown (key))
          cLog::log (LOGINFO, fmt::format ("keyboard key:{} pressed:{} mod:{}",
                                           getKeyName (key), isPressed, (int)mod));
      },

    mWindow);
  //}}}
  //{{{
  setCharInputCallback ([&](sMiniWindow* window, uint32_t charCode) {
      (void)window;
      cLog::log (LOGINFO, fmt::format ("char code:{}", charCode));
      },
    mWindow);
  //}}}

  // mouse callbacks
  //{{{
  setPointerButtonCallback ([&](sMiniWindow* window, mfb_pointer_button button, mfb_key_mod mod, bool isPressed) {

      (void)mod;
      //cLog::log (LOGINFO, fmt::format ("mouseButton {} button:{} pressed:{} at:{} {} mod:{}",
      //                                 window ? (const char*)mfb_get_user_data(window) : "",
      //                                 (int)button, isPressed,
      //                                 mfb_get_mouse_x (window), mfb_get_mouse_y (window),
      //                                 (int)mod));
      if (isPressed) {
        mMousePress = true;
        mMouseMoved = false;
        mMousePressPos = cPoint ((float)getPointerX (window), (float)getPointerY (window));
        mMousePressRight = button != 0;
        mMouseLastPos = mMousePressPos;
        mMousePressUsed = mouseDown (mMousePressRight, mMousePressPos);
        if (mMousePressUsed)
          changed();
        cursorChanged();
        }
      else {
        mMouseLastPos = cPoint (getPointerX (window), getPointerY (window));
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
  setPointerMoveCallback ([&](sMiniWindow* window, int x, int y, int pressure, int timestamp) {
      (void)window;
      cLog::log (LOGINFO, fmt::format ("mouseMove x:{} y:{} press:{} time:{}",
                                       x, y, pressure, timestamp));

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
  setPointerWheelCallback ([&](sMiniWindow* window, mfb_key_mod mod, float deltaX, float deltaY) {
      // lambda
      (void)window;
      (void)mod;
      (void)deltaX;
      //cLog::log (LOGINFO, fmt::format ("mouseWheel {} x:{} y:{} mod:", window ? (const char*)mfb_get_user_data (window) : "",
      //                                 deltaX, deltaY, (int)mod));

      mScale *= (deltaY > 0.f) ? 1.05f : 1.f / 1.05f;
      cLog::log (LOGINFO, fmt::format ("mouseWheel problem - deltaY:{} int(deltaY):{}", deltaY, int(deltaY)));
      if (mouseWheel ((int)deltaY, mMousePos))
        changed();

      cursorChanged();
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
