// basicBoxes.h
#pragma once
//{{{  includes
#include <iostream>
#include <functional>

#include "../date/include/date/date.h"
#include "../common/basicTypes.h"

#include "cWindow.h"
//}}}

class cCalendarBox : public cWindow::cBox {
public:
  cCalendarBox (cWindow& window, bool roundedBgnd = false)
    : cBox("calendar", window, 11, 8), mRoundedBgnd(roundedBgnd) {}

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
  //{{{
  virtual void draw() final {

    auto datePoint = date::floor<date::days>(mWindow.getNowDaylight());

    const float dayWidth = (getWidth() - (2 * getRoundRadius())) / 7;
    if (mRoundedBgnd)
      drawRounded (kBlack, getRoundRadius());
    else
      drawRounded (kBlack);

    cRect calendarRect (mRect);
    calendarRect.addBorder (getRoundRadius());

    // draw month, left justified, top row
    cRect r(calendarRect);
    date::year_month_day yearMonthDay = date::year_month_day { datePoint };
    date::year_month yearMonth = yearMonthDay.year() / date::month { yearMonthDay.month() };
    drawText (kWhite, r, date::format ("%B", yearMonth));

    // draw year, right justified, top row
    std::string yearString = date::format ("%Y", yearMonth);
    float yearWidth = measureText (yearString).x;
    r.left = calendarRect.right - yearWidth;
    drawText (kWhite, r, yearString);
    r.top += getBoxHeight();

    // draw daysOfWeek, 2nd row
    date::day today = yearMonthDay.day();
    date::weekday weekDayToday = date::weekday { yearMonth / today };
    date::weekday titleWeekDay = date::sun;

    r.left = calendarRect.left;
    do {
      std::string dayString = date::format ("%a", titleWeekDay);
      dayString.resize (2);
      drawText ((weekDayToday == titleWeekDay) ?  kWhite : kGray, r, dayString);
      r.left += dayWidth;
      } while (++titleWeekDay != date::sun);
    r.top += getBoxHeight();

    // draw lines of days, skip leading space
    date::weekday weekDay = date::weekday{yearMonth / 1};
    using date::operator""_d;
    date::day curDay = 1_d;
    date::day lastDayOfMonth = (yearMonth / date::last).day();

    int line = 1;
    r.left = calendarRect.left + (weekDay - date::sun).count() * dayWidth;
    while (curDay <= lastDayOfMonth) {
      // iterate days of week
      drawText (today == curDay ? kWhite : kGray, r, date::format ("%e", curDay));

      if (++weekDay == date::sun) {
        // line 6 folds back to first
        line++;
        r.top += line <= 5 ? getBoxHeight() : - 4* getBoxHeight();
        r.left = calendarRect.left;
        }
      else
        r.left += dayWidth;

      ++curDay;
      };
    }
  //}}}

  private:
    const bool mRoundedBgnd;
  };
