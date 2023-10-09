// basicBoxes.h
#pragma once
//{{{  includes
#include <iostream>
#include <functional>

#include "../common/basicTypes.h"
#include "../common/cLog.h"

#include "cWindow.h"
//}}}

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
