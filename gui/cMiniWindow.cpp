// cMiniWindow.cpp
//{{{  includes
#include "cWindow.h"

#include <chrono>
#include "../common/date.h"
#include "../common/basicTypes.h"
#include "../common/utils.h"
#include "../common/cLog.h"

#include "../miniFB/cMiniFB.h"

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
  (void)fullScreen;

  #ifdef _WIN32
    TIME_ZONE_INFORMATION timeZoneInfo;
    if (GetTimeZoneInformation (&timeZoneInfo) == TIME_ZONE_ID_DAYLIGHT)
      mDayLightSeconds = -timeZoneInfo.DaylightBias * 60;
  #else
    cLog::log (LOGERROR, fmt::format ("No timezone correction for Linux yet"));
  #endif

  mMiniFB = cMiniFB::create (title.c_str(), width, height, WF_RESIZABLE);
  if (!mMiniFB)
    return false;

  // create texture static resources after window, may use its openGL resources in future
  cDrawTexture::createStaticResources (getBoxHeight() * 4.0f / 5.0f);

  // create cTexture pixels on the heap
  createPixels (width, height);

  // state funcs
  //{{{
  mMiniFB->setActiveFunc ([&](cMiniFB* miniFB) {
    cLog::log (LOGINFO, fmt::format ("active {} unused", miniFB->isWindowActive()));
    });
  //}}}
  //{{{
  mMiniFB->setResizeFunc ([&](struct cMiniFB* miniFB) {
    int width = miniFB->getWindowScaledWidth();
    int height = miniFB->getWindowScaledHeight();

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
    miniFB->setViewport (x, y, width, height);
    });
  //}}}
  //{{{
  mMiniFB->setCloseFunc ([&](cMiniFB* miniFB) {
    (void)miniFB;
    cLog::log (LOGINFO, fmt::format ("close"));
    return true; // false for don't close
    });
  //}}}

  // keyboard funcs
  //{{{
  mMiniFB->setKeyFunc ([&](cMiniFB* miniFB) {
    if (miniFB->getKeyCode() == KB_KEY_ESCAPE)
    miniFB->close();

    if (miniFB->getPressed())
      if (!keyDown (miniFB->getKeyCode()))
        cLog::log (LOGINFO, fmt::format ("keyboard key:{} pressed:{} mod:{}",
                                         cMiniFB::getKeyName (miniFB->getKeyCode()), miniFB->getPressed(), (int)miniFB->getModifierKeys()));
    });
  //}}}
  //{{{
  mMiniFB->setCharFunc ([&](cMiniFB* miniFB) {
    cLog::log (LOGINFO, fmt::format ("char code:{} unused", miniFB->getCodePoint()));
    });
  //}}}

  // pointer funcs
  //{{{
  mMiniFB->setButtonFunc ([&](cMiniFB* miniFB) {
    if (miniFB->getPointerDown()) {
      mMousePress = true;
      mMouseMoved = false;
      mMousePressPos = cPoint ((float)miniFB->getPointerPosX(), (float)miniFB->getPointerPosY());
      mMousePressRight = miniFB->getPointerButtonStatus()[MOUSE_BTN_3];
      mMouseLastPos = mMousePressPos;
      mMousePressUsed = mouseDown (mMousePressRight, mMousePressPos);
      if (mMousePressUsed)
        changed();
      cursorChanged();
      }
    else {
      mMouseLastPos = cPoint (miniFB->getPointerPosX(), miniFB->getPointerPosY());
      if (mouseUp (mMousePressRight, mMouseMoved, mMouseLastPos))
        changed();
      mMousePress = false;
      mMousePressUsed = false;
      cursorChanged();
      }
    });
  //}}}
  //{{{
  mMiniFB->setMoveFunc ([&](cMiniFB* miniFB) {
    //cLog::log (LOGINFO, fmt::format ("mouseMove x:{} y:{} press:{} time:{}", x, y, pressure, timestamp));
    mMousePos.x = (float)miniFB->getPointerPosX();
    mMousePos.y = (float)miniFB->getPointerPosY();
    if (mMousePress) {
      mMouseMoved = true;
      if (mouseMove (mMousePressRight, mMousePos, mMousePos - mMouseLastPos, miniFB->getPointerPressure(), miniFB->getPointerTimestamp()))
        changed();
      mMouseLastPos = mMousePos;
      }
    else if (mouseProx (true, mMousePos))
      changed();
    cursorChanged();
    });
  //}}}
  //{{{
  mMiniFB->setWheelFunc ([&](cMiniFB* miniFB) {
    mScale *= (miniFB->getPointerWheelY() > 0.f) ? 1.05f : 1.f / 1.05f;
    cLog::log (LOGINFO, fmt::format ("mouseWheel problem - deltaY:{} int(deltaY):{}",
                                     miniFB->getPointerWheelY(), int(miniFB->getPointerWheelY())));
    if (mouseWheel ((int)miniFB->getPointerWheelY(), mMousePos))
      changed();

    cursorChanged();
    });
  //}}}
  //{{{
  mMiniFB->setEnterFunc ([&](cMiniFB* miniFB) {
    cLog::log (LOGINFO, fmt::format ("pointerEnter {} unused", miniFB->getPointerInside()));
    });
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
  while (!mExit) {
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

      mMiniFB->update (getPixels());
      frameUs = duration_cast<microseconds>(system_clock::now() - time).count();
      }
    else {
      this_thread::sleep_for (1ms);
      drawCallback (false);
      }

    if (mMiniFB->updateEvents() != STATE_OK)
      break;
    }
  }
//}}}
