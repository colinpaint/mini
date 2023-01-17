// cFileListBox.h
#pragma once
//{{{  includes
#include <array>
#include <functional>
#include "../common/basicTypes.h"
#include "../common/cFileList.h"

#include "cWindow.h"
//}}}

class cFileListBox : public cWindow::cBox {
public:
  //{{{
  cFileListBox (cWindow& window, float width, float height, cFileList& fileList,
                const std::function <void (const std::string&)>& hitCallback = [](const std::string&) {}) :

    cBox ("fileListBox", window, width, height),
    mFileList(fileList), mHitCallback(hitCallback) {}
  //}}}
  virtual ~cFileListBox() = default;

  //{{{
  virtual bool pick (bool inClient, cPoint pos, bool& change) final {

    std::lock_guard<std::mutex> lockGuard (mMutex);

    bool lastPick = mPick;

    mPick = inClient && mBgndRect.inside (pos);
    if (!change && (mPick != lastPick))
      change = true;

    return mPick;
    }
  //}}}

  //{{{
  virtual bool onProx (bool inClient, cPoint pos) final {

    std::lock_guard<std::mutex> lockGuard (mMutex);

    uint32_t rowIndex = 0;
    for (cFileListBox::cRow& row : mRowVec) {
      if (row.mRect.inside (pos)) {
        uint32_t proxIndex = mFirstRowIndex + rowIndex;
        if (proxIndex != mProxIndex) {
          mProxIndex = proxIndex;
          changed();
          }
        mProxed = true;
        return false;
        }
      rowIndex++;
      }

    mProxed = false;

    // !!!! why !!!!
    return cBox::onProx (inClient, pos);
    }
  //}}}
  //{{{
  virtual bool onProxExit() final {

    mProxed = false;
    changed();
    return false;
    }
  //}}}
  //{{{
  virtual bool onDown (bool right, cPoint pos) final  {
    (void)right;
    (void)pos;

    mMoved = false;
    mMoveInc = 0;
    mScrollInc = 0.f;

    if (mProxed)
      mPressed = true;
    else {
      mPressed = false;
      mFileList.nextSort();
      }

    changed();
    return true;
    }
  //}}}
  //{{{
  virtual bool onMove (bool right, cPoint pos, cPoint inc) final {
    (void)right;
    (void)pos;
    (void)inc;

    mMoveInc += inc.y;

    if (abs(mMoveInc) > 2)
      mMoved = true;

    if (mMoved)
      incScroll (-(float)inc.y);

    return true;
    }
  //}}}
  //{{{
  virtual bool onUp (bool right, bool mouseMoved, cPoint pos) final {

    (void)right;
    (void)mouseMoved;
    (void)pos;

    if (mPressed && !mMoved) {
      mFileList.setIndex (mProxIndex);
      mHitCallback (mFileList.getFileItem (mProxIndex).getFullName());
      }

    mPressed = false;
    mMoved = false;
    mMoveInc = 0;
    mProxIndex = 0xFFFFFFFF;

    return true;
    }
  //}}}
  //{{{
  virtual bool onWheel (int delta, cPoint pos) final {

    (void)delta;
    (void)pos;

    incScroll (-delta / 30.f);
    return true;
    }
  //}}}

  //{{{
  virtual void onDraw() final {

    if (!mPressed && mScrollInc)
      incScroll (mScrollInc * 0.9f);

    // calc first row index, ensure curItemIndex is visible
    mFirstRowIndex = uint32_t(mScroll / getLineHeight());

    if (mFileList.ensureItemVisible()) {
      //{{{  curItem probably changed, show it
      // strange interaction between itemIndex change and its visibility
      if (mFileList.getIndex() < mFirstRowIndex) {
        mFirstRowIndex = mFileList.getIndex();
        mScroll = mFirstRowIndex * getLineHeight();
        }
      else if ((mLastRowIndex > 0) && (mFileList.getIndex() >= mLastRowIndex)) {
        mFirstRowIndex += mFileList.getIndex() - mLastRowIndex;
        mScroll = mFirstRowIndex * getLineHeight();
        }
      mProxed = false;
      }
      //}}}

    float maxColumnWidths[cFileList::cFileItem::kFields] = { 0.f };

    float textHeight = getLineHeight() * 5.f / 6.f;
    std::lock_guard<std::mutex> lockGuard (mMutex);

    // layout visible rows
    mRowVec.clear();
    uint32_t index = mFirstRowIndex;
    cPoint point (0.f, 1.f - (mScroll - (mFirstRowIndex * getLineHeight())));
    while ((index < mFileList.size()) && (point.y < mRect.bottom)) {
      // layout row
      cRow row;
      cFileList::cFileItem fileItem = mFileList.getFileItem (index);
      for (uint32_t field = 0u; field < cFileList::cFileItem::kFields; field++) {
        row.mStrings [field] = fileItem.getFieldString (field);
        row.mWidths [field] = measureText (row.mStrings [field]).x;
        maxColumnWidths[field] = std::max (row.mWidths[field], maxColumnWidths[field]);
        }
      row.mRect = cRect(point, point + cPoint(row.mWidths[0], getLineHeight()));
      row.mColor = (mProxed && !mMoved && (index == mProxIndex)) ?
                     kYellow : mFileList.isCurIndex (index) ?
                       kWhite : kLightBlue;
      mRowVec.push_back (row);
      mLastRowIndex = index;
      point.y += getLineHeight();
      index++;
      }

    // layout fieldStops
    mColumnsWidth = 0.f;
    for (uint32_t field = 0u; field < cFileList::cFileItem::kFields; field++) {
      mColumnsWidth += maxColumnWidths[field] + textHeight/2.f;
      mColumn[field] = mColumnsWidth - 2.f;
      }

    // layout, draw bgnd
    mBgndRect = cRect (mColumnsWidth + getLineHeight()/2.f, point.y);
    drawRectangle (kClearBgnd, mBgndRect + mRect.getTL());

    // layout, draw fields
    for (cFileListBox::cRow& row : mRowVec) {
      cPoint fieldPoint = mRect.getTL() + row.mRect.getTL();
      for (uint32_t field = 0u; field < cFileList::cFileItem::kFields; field++) {
        fieldPoint.x = mRect.left + row.mRect.left + (field ? mColumn[field] - row.mWidths[field] : 2.f);
        drawText (row.mColor, cRect (fieldPoint, getSize()), row.mStrings[field]);
        }
      }
    }
  //}}}

private:
  static constexpr float kMinLineHeight = 16.f;
  float getLineHeight() { return std::min (std::max (getHeight() / mFileList.size(), kMinLineHeight), getBoxHeight()); }

  //{{{
  void incScroll (float inc) {

    mScroll += inc;
    if (mScroll < 0.f)
      mScroll = 0.f;
    else if ((mFileList.size() * getLineHeight()) < mRect.getHeight())
      mScroll = 0.f;
    else if (mScroll > ((mFileList.size() * getLineHeight()) - mRect.getHeight()))
      mScroll = float(mFileList.size() * getLineHeight() - mRect.getHeight());

    mScrollInc = fabs(inc) < 0.2f ? 0 : inc;
    }
  //}}}

  // vars
  cFileList& mFileList;
  const std::function <void (const std::string&)> mHitCallback;

  //{{{
  class cRow {
  public:
    cRect mRect;
    cColor mColor;
    std::array <std::string, cFileList::cFileItem::kFields> mStrings;
    std::array <float, cFileList::cFileItem::kFields> mWidths;
    };
  //}}}
  concurrency::concurrent_vector <cRow> mRowVec;
  std::mutex mMutex; // guard mRowVec - pick,prox,down against draw

  float mColumn [cFileList::cFileItem::kFields] = {0.f};
  float mColumnsWidth = 0.f;
  cRect mBgndRect;

  bool mProxed = false;
  bool mPressed = false;
  bool mMoved = false;

  float mMoveInc = 0;
  float mScroll = 0.f;
  float mScrollInc = 0.f;

  uint32_t mProxIndex = 0;
  uint32_t mFirstRowIndex = 0;
  uint32_t mLastRowIndex = 0;
  };
