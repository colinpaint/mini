// basicBoxes.h
#pragma once
//{{{  includes
#include <iostream>
#include <functional>

#include "../common/basicTypes.h"
#include "../common/cLog.h"
#include "cWindow.h"
//}}}

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
  bool move (bool right, cPoint pos, cPoint inc, int pressure, int timestamp) final {

    (void)right;
    (void)pos;
    (void)inc;
    (void)pressure;
    (void)timestamp;

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
