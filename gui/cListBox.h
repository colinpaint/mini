// basicBoxes.h
#pragma once
//{{{  includes
#include <iostream>
#include <functional>

#include "../common/basicTypes.h"
#include "../common/cLog.h"

#include "cWindow.h"
//}}}

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
  virtual bool move (bool right, cPoint pos, cPoint inc, int pressure, int timestamp) final {

    (void)right;
    (void)pos;
    (void)pressure;
    (void)timestamp;

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
