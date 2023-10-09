// basicBoxes.h
#pragma once
//{{{  includes
#include <iostream>
#include <functional>

#include "../common/date.h"
#include "../common/basicTypes.h"

#include "cWindow.h"
//}}}

class cDateBox : public cWindow::cBox {
public:
  cDateBox (cWindow& window, float width, float height, std::chrono::system_clock::time_point& timePoint) :
    cBox("date", window, width, height), mTimePoint(timePoint) {}

  virtual void draw() final {
    drawTextRectangle (kTextGray, kBoxGray,
                       date::format (std::cout.getloc(), "%a %e %B %Y",
                                     date::year_month_day {date::floor<date::days>(mTimePoint)}));
    }

private:
  std::chrono::system_clock::time_point& mTimePoint;
  };
