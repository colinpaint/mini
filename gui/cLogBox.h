// cLogBox.h
#pragma once
//{{{  includes
#include "../date/include/date/date.h"
#include "../common/basicTypes.h"

#include "cWindow.h"
//}}}

class cLogBox : public cWindow::cBox {
public:
  //{{{
  cLogBox(cWindow& window) : cBox("logBox", window, 1.f,0.f) {
  // full screen height, pick region 1 boxHeight on left of screen

    setPin (false);
    mLastRect = {mWindow.getSize()};
    }
  //}}}
  virtual ~cLogBox() = default;

  //{{{
  virtual bool move (bool right, cPoint pos, cPoint inc) final {

    (void)right;
    (void)pos;
    (void)inc;

    mLogScroll += (int)inc.y * (mWindow.getControl() ? 100 : 1);
    if (mLogScroll < 0)
      mLogScroll = 0;

    return true;
    }
  //}}}
  //{{{
  virtual bool up (bool right, bool mouseMoved, cPoint pos) final {

    (void)right;
    (void)mouseMoved;
    (void)pos;

    if (!mouseMoved)
      togglePin();

    return true;
    }
  //}}}
  //{{{
  virtual bool wheel (int delta, cPoint pos) final  {

    (void)delta;
    (void)pos;

    if (getShow()) {
      cLog::setLogLevel (eLogLevel(cLog::getLogLevel() + (delta)));
      return true;
      }

    return false;
    }
  //}}}
  //{{{
  virtual void draw() final {

    // draw dim bgnd using lastRect, cheat saves a pre-pass
    drawRectangle (mPin ? kDimBgnd : kClearBgnd, mLastRect);

    uint32_t lastLineIndex = 0;
    int logLineNum = int(mLogScroll / int(mWindow.getConsoleHeight()));
    auto lastTimePoint = mWindow.getNowDaylight();

    // draw lines
    cLogLine logLine;
    float maxWidth = 0.f;
    float y = mWindow.getHeight() + (mLogScroll % int(mWindow.getConsoleHeight())) - 2.f;
    while ((y > 20.f) && cLog::getLine (logLine, logLineNum++, lastLineIndex)) {
      //{{{  draw timeElapsed bar beneath log text
      int64_t timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(lastTimePoint - logLine.mTimePoint).count();
      if (timeDiff < 20) {
        drawRectangle (kColors[logLine.mLogLevel], {0.f, y - 1.f, timeDiff * 10.f, y + 1.f});
        y -= mWindow.getConsoleHeight() + 2.f;
        }
      else {
        timeDiff = std::min ((int32_t)timeDiff, 4000);
        drawRectangle (kWhite, {0.f, y - 2.f, timeDiff/10.f, y + 1.f});
        y -= mWindow.getConsoleHeight() + 3.f;
        }
      //}}}

      auto timeOfDay = date::make_time (std::chrono::duration_cast<std::chrono::microseconds>(
                                         logLine.mTimePoint - floor<date::days>(logLine.mTimePoint)));

      float width = drawText (kColors[logLine.mLogLevel], { 0.f, y, (float)mWindow.getWidth(), y + mWindow.getConsoleHeight() + 4.f},
                              fmt::format ("{:02}:{:02}:{:02}.{:06} {} {}",
                                           timeOfDay.hours().count(), timeOfDay.hours().count(),
                                           timeOfDay.seconds().count(), timeOfDay.subseconds().count(),
                                           cLog::getThreadName (logLine.mThreadId), logLine.mString),
                              cWindow::kConsoleFont).x;
      maxWidth = std::max (width, maxWidth);
      lastTimePoint = logLine.mTimePoint;
      }

    // extend bgnd box to our width + a bit for next time
    mLastRect.right = maxWidth + mWindow.getConsoleHeight();
    }
  //}}}

private:
  const cColor kColors[eMaxLog] = {
    {  1.f,  1.f,  1.f, 1.f }, // LOGNOTICE white
    {  1.f, 0.5f, 0.5f, 1.f }, // LOGERROR  light red
    {  1.f,  1.f, 0.2f, 1.f }, // LOGINFO   yellowy
    { 0.5f, 0.8f, 0.5f, 1.f }, // LOGINFO1  greeny
    { 0.5f, 0.5f, 0.8f, 1.f }, // LOGINFO2  light blue
    { 0.8f, 0.1f, 0.8f, 1.f }, // LOGINFO3  magenta
    };

  int mLogScroll = 0;
  cRect mLastRect;
  };
