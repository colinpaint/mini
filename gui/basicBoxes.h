// basicBoxes.h
#pragma once
#include <iostream>
#include <functional>
#include "../common/date.h"
#include "../common/basicTypes.h"
#include "../common/cLog.h"
#include "cWindow.h"

constexpr float kLineWidth = 2.f;

//{{{
class cTextBox : public cWindow::cBox {
public:
  cTextBox (cWindow& window, float width, float height, const std::string& text,
            std::function <void()> hitCallback = []() {})
    : cBox("text", window, width, height), mText(text), mHitCallback(hitCallback) {}
  virtual ~cTextBox() = default;

  virtual bool down (bool right, cPoint pos) final {
    (void)right;
    (void)pos;
    mHitCallback();
    return true;
    }

  virtual void draw() final {
    drawTextShadow (kWhite, mText);
    }

private:
  std::string mText;
  std::function <void()> mHitCallback;
  };
//}}}
//{{{
class cTextBgndBox : public cWindow::cBox {
public:
  cTextBgndBox (cWindow& window, float width, float height, const std::string& text,
                std::function <void()> hitCallback = []() {})
    : cBox("textBgnd", window, width, height), mText(text), mHitCallback(hitCallback) {}
  virtual ~cTextBgndBox() = default;

  virtual bool down (bool right, cPoint pos) final {
    (void)right;
    (void)pos;
    mHitCallback();
    return true;
    }

  virtual void draw() final {
    drawTextRectangle (kTextGray, kBoxGray, mText);
    }

private:
  std::string mText;
  std::function <void()> mHitCallback;
  };
//}}}

//{{{
class cToggleBox : public cWindow::cBox {
public:
  cToggleBox (cWindow& window, float width, float height, const std::string& text, bool& on,
              const std::function <void (bool on)>& callback = [](bool) {})
      : cBox("toggle", window, width, height), mText(text), mOn(on), mCallback(callback) {}
  virtual ~cToggleBox() = default;

  //{{{
  virtual bool down (bool right, cPoint pos) final  {

   (void)right;
   (void)pos;

    mOn = !mOn;
    mCallback (mOn);
    return true;
    }
  //}}}

  virtual void draw() final {
    drawRectangle (mOn ? kLightBlue : kBoxGray);
    drawText (mOn ? kWhite : kTextGray, mText);
    }

private:
  const std::string mText;
  bool& mOn;
  const std::function <void (bool on)> mCallback;
  };
//}}}

//{{{
class cListBox : public cWindow::cBox {
public:
  //{{{
  cListBox (cWindow& window, float width, float height,
            const std::vector<std::string>& items,
            const std::function <void (int index)>& callback = [](int) {})
      : cBox ("list", window, width, height), mItems(items), mCallback(callback) {

    // allocating matching measure vector
    mMeasureItems.reserve (items.size());
    for (unsigned i = 0; i < items.size(); i++)
      mMeasureItems.push_back (0);
    }
  //}}}
  virtual ~cListBox() = default;

  int getSelectedIndex() { return mSelectedIndex; }
  std::string getString (int index) { return mItems[index]; }
  std::string getSelectedString() { return mItems[mSelectedIndex]; }

  //{{{
  virtual bool pick (bool inClient, cPoint pos, bool& change) final {

    bool lastPick = mPick;
    mPick = inClient && mBgndRect.inside (pos);
    if (!change && (mPick != lastPick))
      change = true;

    return mPick;
    }
  //}}}
  //{{{
  virtual bool down (bool right, cPoint pos) final  {

    (void)right;
    (void)pos;

    mMoved = false;
    mMoveInc = 0;
    mScrollInc = 0.f;

    mPressedIndex = int((mScroll + pos.y) / mLineHeight);
    int32_t pressedLine = int32_t(pos.y / mLineHeight);
    if ((pressedLine >= 0) && (pressedLine < (int32_t)mMeasureItems.size())) {
      mTextPressed = pos.x < mMeasureItems[pressedLine];
      return true;
      }

    return false;
    }
  //}}}
  //{{{
  virtual bool move (bool right, cPoint pos, cPoint inc) final {

    (void)right;
    (void)pos;

    mMoveInc += inc.y;
    if (abs(mMoveInc) > 2)
      mMoved = true;
    if (mMoved)
      incScroll (-(float)inc.y);

    return true;
    }
  //}}}
  //{{{
  virtual bool up (bool right, bool mouseMoved, cPoint pos) final {

    (void)right;
    (void)mouseMoved;
    (void)pos;

    if (mTextPressed && !mMoved) {
      mSelectedIndex = mPressedIndex;
      mCallback (mSelectedIndex);
      }

    mTextPressed = false;
    mPressedIndex = -1;
    mMoved = false;
    mMoveInc = 0;

    return true;
    }
  //}}}

  //{{{
  virtual void draw() final {

    if (!mTextPressed && mScrollInc)
      incScroll (mScrollInc * 0.9f);

    drawRectangle (kClearBgnd, mBgndRect);

    int itemIndex = int(mScroll) / (int)mLineHeight;
    float maxWidth = 0.f;
    cPoint point = cPoint (mRect.left+2, mRect.top + 1.f - (int(mScroll) % (int)mLineHeight));

    uint32_t itemNum = 0;
    for (auto item : mItems) {
      float width = measureText (item).x;
      mMeasureItems[itemNum] = width;
      maxWidth = std::max (width, maxWidth);

      cColor color = (mTextPressed && !mMoved && (itemIndex == mPressedIndex)) ?
                       kYellow : (itemIndex == mSelectedIndex) ?
                         kWhite : kLightBlue;
      drawText (color, {point, getSize()}, item);

      point.y += mLineHeight;
      if (point.y > mRect.bottom)
        break;
      itemNum++;

      mBgndRect = { mRect.left, mRect.top, mRect.left + maxWidth + 4.0f, point.y };
      }
    }
  //}}}

private:
  //{{{
  void incScroll (float inc) {

    mScroll += inc;
    if (mScroll < 0.f)
      mScroll = 0.f;
    else if ((mItems.size() * mLineHeight) < mRect.getHeight())
      mScroll = 0.f;
    else if (mScroll > ((mItems.size() * mLineHeight) - mRect.getHeight()))
      mScroll = float(((int)mItems.size() * mLineHeight) - mRect.getHeight());

    mScrollInc = fabs(inc) < 0.2f ? 0 : inc;
    }
  //}}}

  const std::vector <std::string>& mItems;
  const std::function <void (int index)> mCallback;

  std::vector<float> mMeasureItems;
  int mSelectedIndex = -1;

  float mLineHeight = getBoxHeight();

  bool mTextPressed = false;
  int mPressedIndex = -1;
  bool mMoved = false;
  float mMoveInc = 0;

  float mScroll = 0.f;
  float mScrollInc = 0.f;

  cRect mBgndRect;
  };
//}}}
//{{{
class cIndexBox : public cWindow::cBox {
public:
  cIndexBox (cWindow& window, float width,
             const std::vector <std::string>& strings, uint32_t& index,
             const std::function <void (uint32_t index)>& callback = [](uint32_t) {})
      : cBox ("offset", window, width, (float)strings.size()),
        mStrings(strings), mIndex(index), mCallback(callback) {}
  virtual ~cIndexBox() = default;

  //{{{
  virtual bool wheel (int delta, cPoint pos) final {

    (void)delta;
    (void)pos;

    setIndex (mIndex + delta/120);
    return true;
    }
  //}}}
  //{{{
  virtual bool down (bool right, cPoint pos) final {

    (void)right;
    (void)pos;

    setIndex (uint32_t(pos.y / getBoxHeight()));
    return true;
    }
  //}}}

  void virtual draw() final {
    cRect r (mRect);
    r.bottom = r.top + getBoxHeight();
    for (uint32_t index = 0; index < mStrings.size(); index++) {
      drawTextRectangle ((index == mIndex) ? kBlack : kWhite,
                         (index == mIndex) ? kLightBlue : kGray,
                         r, mStrings[index]);
      r.addVertical (getBoxHeight());
      }
    }

private:
  //{{{
  void setIndex (uint32_t index) {
    mCallback (std::min (std::max (index, 0u), (uint32_t)mStrings.size()-1));
    }
  //}}}

  const std::vector <std::string> mStrings;
  uint32_t& mIndex;
  const std::function <void (uint32_t index)> mCallback;
  };
//}}}

//{{{
class cIntBox : public cWindow::cBox {
public:
  cIntBox (cWindow& window, float width, float height, const std::string& format, int& value) :
    cBox("int", window, width, height), mFormat(format), mValue(value) {}
  virtual ~cIntBox() = default;

  virtual void draw() final {
    drawText (kWhite, fmt::vformat (mFormat, fmt::make_format_args (mValue)));
    }

private:
  const std::string mFormat;
  const int& mValue;
  };
//}}}
//{{{
class cIntBgndBox : public cWindow::cBox {
public:
  cIntBgndBox (cWindow& window, float width, float height, std::string format, int& value) :
    cBox("intBgnd", window, width, height), mFormat(format), mValue(value) {}
  virtual ~cIntBgndBox() = default;

  virtual void draw() final {
    drawTextRectangle (kTextGray, kBoxGray, fmt::vformat (mFormat, fmt::make_format_args (mValue)));
    }

private:
  const std::string mFormat;
  const int& mValue;
  };
//}}}

//{{{
class cUint32Box : public cWindow::cBox {
public:
  cUint32Box (cWindow& window, float width, float height, const std::string& format, uint32_t& value) :
    cBox("uint32", window, width, height), mFormat(format), mValue(value) {}
  virtual ~cUint32Box() = default;

  virtual void draw() final {
    drawText (kWhite, fmt::vformat(mFormat, fmt::make_format_args(mValue)));
    }

private:
  const std::string mFormat;
  const uint32_t& mValue;
  };
//}}}
//{{{
class cUint32BgndBox : public cWindow::cBox {
public:
  cUint32BgndBox (cWindow& window, float width, float height, const std::string& format, uint32_t& value) :
    cBox("uint32Bgnd", window, width, height), mFormat(format), mValue(value) {}
  virtual ~cUint32BgndBox() = default;

  virtual void draw() final {
    drawTextRectangle (kTextGray, kBoxGray, fmt::vformat(mFormat, fmt::make_format_args(mValue)));
    }

private:
  const std::string mFormat;
  const uint32_t& mValue;
  };
//}}}

//{{{
class cUint64Box : public cWindow::cBox {
public:
  cUint64Box (cWindow& window, float width, float height, const std::string& format, uint64_t& value) :
    cBox("uint64", window, width, height), mFormat(format), mValue(value) {}
  virtual ~cUint64Box() = default;

  virtual void draw() final {
    cRect r(mRect);
    std::string text = fmt::vformat(mFormat, fmt::make_format_args(mValue));
    if (mSelfSize)
      r.left += getWidth()- measureText (text).x;

    drawText (kWhite, r, fmt::vformat(mFormat, fmt::make_format_args(mValue)));
    }

private:
  const std::string mFormat;
  const uint64_t& mValue;
  };
//}}}
//{{{
class cUint64BgndBox : public cWindow::cBox {
public:
  cUint64BgndBox (cWindow& window, float width, float height, const std::string& format, uint64_t& value) :
    cBox("uint64", window, width, height), mFormat(format), mValue(value) {}
  virtual ~cUint64BgndBox() = default;

  virtual void draw() final {
    drawTextRectangle (kTextGray, kBoxGray, fmt::vformat(mFormat, fmt::make_format_args(mValue)));
    }

private:
  const std::string mFormat;
  const uint64_t& mValue;
  };
//}}}

//{{{
class cFloatBox : public cWindow::cBox {
public:
  cFloatBox (cWindow& window, float width, float height, std::string format, float& value)
      : cBox("float", window, width, height), mFormat(format), mValue(value) {
    mPin = true;
    }
  virtual ~cFloatBox() = default;

  virtual void draw() final {
    drawTextRectangle (kTextGray, kBoxGray, fmt::vformat(mFormat, fmt::make_format_args(mValue)));
    }

private:
  std::string mFormat;
  const float& mValue;
  };
//}}}
//{{{
class cFloatBgndBox : public cWindow::cBox {
public:
  cFloatBgndBox (cWindow& window, float width, float height, std::string format, float& value) :
      cBox("floatBgnd", window, width, height), mFormat(format), mValue(value)  {
    mPin = true;
    }
  virtual ~cFloatBgndBox() = default;

  virtual void draw() final {
    drawTextRectangle (kTextGray, kBoxGray, fmt::vformat(mFormat, fmt::make_format_args(mValue)));
    }

private:
  const std::string mFormat;
  const float& mValue;
  };
//}}}

//{{{
class cValueBox : public cWindow::cBox {
public:
  //{{{
  cValueBox (cWindow& window, float width, float height, const std::string& text, float min, float max, float& value,
             const std::function <void (float value)>& callback = [](float) {})
    : cBox("value", window, width, height),
      mText(text), mMin(min), mMax(max), mValue(value), mCallback(callback),
      mScaleX ((max - min) / 100.f), mScaleY ((max - min) / 100.f), mScaleMouse ((max - min) / 100.f) {}
  //}}}
  virtual ~cValueBox() {}

  //{{{
  cValueBox* setScale (float scale) {
    mScaleX = scale;
    mScaleY = scale;
    mScaleMouse = scale;
    return this;
    }
  //}}}
  //{{{
  cValueBox* setScale (float xScale, float yScale) {
    mScaleX = xScale;
    mScaleY = yScale;
    mScaleMouse = std::max (xScale, yScale);
    return this;
    }
  //}}}
  //{{{
  cValueBox* setScale (float xScale, float yScale, float mouseScale) {
    mScaleX = xScale;
    mScaleY = yScale;
    mScaleMouse = mouseScale;
    return this;
    }
  //}}}

  //{{{
  bool wheel (int delta, cPoint pos) final  {

    (void)delta;
    (void)pos;

    setValue (mValue + (delta * std::max(mScaleX, mScaleY)));
    return true;
    }
  //}}}
  //{{{
  bool move (bool right, cPoint pos, cPoint inc) final {

    (void)right;
    (void)pos;
    (void)inc;

    setValue (mValue + (inc.x * mScaleX)  + (inc.y * mScaleY));
    return true;
    }
  //}}}

  virtual void draw() final {
    drawRectangle (mPick ? kYellow : kLightGray, mRect);
    drawText (mPick ? kBlack : kWhite, mRect, fmt::vformat (mText, fmt::make_format_args(mValue)));
    }

private:
  //{{{
  void setValue (float value) {
    mValue = std::min (std::max (value, mMin), mMax);
    mCallback (mValue);
    }
  //}}}

  const std::string mText;
  const float mMin = 0.f;
  const float mMax = 100.f;
  float& mValue;
  const std::function <void (float index)> mCallback;
  float mScaleX = 1.f;
  float mScaleY = 1.f;
  float mScaleMouse = 1.f;
  };
//}}}

//{{{
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
  virtual bool move (bool right, cPoint pos, cPoint inc) final {

    (void)right;
    (void)pos;

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
//}}}
//{{{
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
  virtual bool move (bool right, cPoint pos, cPoint inc) final {

    (void)right;
    (void)pos;

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
//}}}
//{{{
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
//}}}

//{{{
class cWindowBox : public cWindow::cBox {
public:
  //{{{
  cWindowBox (cWindow& window, float width, float height)
      : cBox("window", window, width, height) {
    setSelfSize();
    }
  //}}}
  virtual ~cWindowBox() = default;

  virtual bool down (bool right, cPoint pos) final {
    (void)right;
    (void)pos;
    if (pos.x < getWidth()/2)
      mWindow.toggleFullScreen();
    else
      mWindow.setExit();
    return true;
    }

  virtual void resize() final {
    layout();
    mEnable = mWindow.getFullScreen();
    }

  virtual void draw() final {
    cRect r(mRect);
    //#ifdef BUILD_D2D
    //  std::wstring text = mWindow.getFullScreen() ? L"\x32\x72" : L"\x31\x72";
    //#else
      std::string text = mWindow.getFullScreen() ? "full exit" : "exit";
    //#endif
    if (mSelfSize)
      r.left += getWidth() - measureText (text, cWindow::kSymbolFont).x;
    drawTextShadow (kWhite, r, text, cWindow::kSymbolFont);
    }
  };
//}}}

//{{{
class cTextureBox : public cWindow::cBox {
public:
  cTextureBox (cWindow& window, float width, float height, cTexture& texture)
    : cBox("texture", window, width, height), mTexture(texture) {}
  virtual ~cTextureBox() = default;

  virtual void draw() final {
    blit (mTexture, {mRect.getTL(), cPoint(mTexture.getWidth(), mTexture.getHeight())});
    }

private:
  cTexture& mTexture;
  };
//}}}
