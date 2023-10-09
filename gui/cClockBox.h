// basicBoxes.h
#pragma once
//{{{  includes
#include <iostream>
#include <functional>

#include "../common/date.h"
#include "../common/basicTypes.h"

#include "cWindow.h"
//}}}

class cClockBox : public cWindow::cBox {
public:
  cClockBox (cWindow& window, float height, bool showSubSeconds = false)
    : cBox("clock", window, height, height), mShowSubSeconds(showSubSeconds) {}

  //{{{
  virtual bool down (bool right, cPoint pos) final {
    (void)right;
    (void)pos;

    toTop();
    return true;
    }
  //}}}
  //{{{
  virtual bool move (bool right, cPoint pos, cPoint inc, int pressure, int timestamp) final {

    (void)right;
    (void)pos;
    (void)pressure;
    (void)timestamp;

    mRect.left += inc.x;
    mRect.right += inc.x;
    mRect.top += inc.y;
    mRect.bottom += inc.y;

    return true;
    }
  //}}}

  virtual void draw() final {
    float radius = getHeight() / 2;

    auto timePoint = mWindow.getNowDaylight();
    auto datePoint = floor<date::days>(timePoint);
    auto timeOfDay = date::make_time (std::chrono::duration_cast<std::chrono::milliseconds>(timePoint - datePoint));

    const float subSeconds = (float)timeOfDay.subseconds().count();
    float seconds = (float)timeOfDay.seconds().count();
    if (mShowSubSeconds)
      seconds += subSeconds / 1000.f;
    const float minutes = (float)timeOfDay.minutes().count() + (seconds / 60.f);
    const float hours = (float)timeOfDay.hours().count() + (minutes / 60.f);

    const float hoursRadius = radius * 0.6f;
    const float hoursAngle = (1.f - (hours / 6.f)) * kPi;
    drawLine (kWhite, getCentre(), getCentre() + cPoint(hoursRadius * sin (hoursAngle),
                                                        hoursRadius * cos (hoursAngle)), getOutlineWidth());
    const float minutesRadius = radius * 0.75f;
    const float minutesAngle = (1.f - (minutes/30.f)) * kPi;
    drawLine (kWhite, getCentre(), getCentre() + cPoint(minutesRadius * sin (minutesAngle),
                                                        minutesRadius * cos (minutesAngle)), getOutlineWidth());
    const float secondsRadius = radius * 0.85f;
    const float secondsAngle = (1.0f - (seconds /30.0f)) * kPi;
    drawLine (kRed, getCentre(), getCentre() + cPoint(secondsRadius * sin (secondsAngle),
                                                      secondsRadius * cos (secondsAngle)), getOutlineWidth()/2.f);
    if (mShowSubSeconds) {
      const float subSecondsRadius = radius * 0.8f;
      const float subSecondsAngle = (1.0f - (subSeconds / 500.f)) * kPi;
      drawLine (kLightBlue, getCentre(), getCentre() + cPoint(subSecondsRadius * sin (subSecondsAngle),
                                                              secondsRadius * cos (subSecondsAngle)), getOutlineWidth());
      }

    drawEllipse (kWhite, getCentre(), radius, getOutlineWidth());
    }

private:
  bool mShowSubSeconds;
  };
