// cWindow.h - shared between cD2dWindow.cpp and cMiniWindow.cpp
#pragma once
//{{{  includes
#include "cDrawTexture.h"

#include <chrono>
#include <thread>
#include <deque>

// some keycodes
#include "../miniFB/miniFBenums.h"
//}}}

class cWindow : public cDrawTexture {
public:
  //{{{  statics
  // font const
  static constexpr uint32_t kMenuFont = 0;
  static constexpr uint32_t kLargeMenuFont = 1;
  static constexpr uint32_t kConsoleFont = 2;
  static constexpr uint32_t kSymbolFont = 3;
  static constexpr uint32_t kNumFonts = 4;

  static float getBoxHeight() { return mBoxHeight; }
  static cPoint getTextOffset() { return mTextOffset; }
  static float getConsoleHeight() { return mConsoleHeight; }

  static float getOutlineWidth() { return mOutlineWidth; }
  static float getRoundRadius() { return mRoundRadius; }
  static float getBorderWidth() { return mBorderWidth; }

  static int getDayLightSeconds() { return mDayLightSeconds; }
  //{{{
  static std::chrono::system_clock::time_point getNow() {
    return std::chrono::system_clock::now();
    }
  //}}}
  //{{{
  static std::chrono::system_clock::time_point getNowDaylight() {
    return getNow() + std::chrono::seconds (mDayLightSeconds);
    }
  //}}}
  //}}}
  //{{{  gets
  cPoint getSize() const { return cPoint (getWidth(), getHeight()); }

  // in boxes
  float getWidthInBoxes() const { return getWidth() / getBoxHeight(); }
  float getHeightInBoxes() const { return getHeight() / getBoxHeight(); }
  cPoint getSizeInBoxes() const { return getSize() / getBoxHeight(); }
  cPoint getCentreInBoxes() const { return getSize() / 2.f / getBoxHeight(); }

  // keys
  bool getShift() const { return mShiftKeyDown; }
  bool getControl() const { return mControlKeyDown; }

  uint64_t& getRenderUs() { return mRenderUs; }

  bool getFullScreen() const { return mFullScreen; }
  bool getExit() const { return mExit; }

  // raw mouse
  cPoint getMousePos() const { return mMousePos; }
  bool isMousePress() const { return mMousePress; }
  bool isMousePressUsed() const { return mMousePressUsed; }
  float getScale() { return mScale; }
  //}}}
  void setExit() { mExit = true; }

  // actions
  void changed() { mChanged = true; }
  void keyChanged() { mCursorDown = mCursorCountDown; }
  void cursorChanged() { mCursorDown = mCursorCountDown; }
  void resized();
  void toggleFullScreen();

  //{{{
  class cBox {
  public:
    //{{{
    cBox (const std::string& name, cWindow& window, float widthInBoxes, float heightInBoxes)
        : mName(name), mWindow(window),
          mLayoutWidth(widthInBoxes*getBoxHeight()), mLayoutHeight(heightInBoxes*getBoxHeight()),
          mRect({0,0,widthInBoxes*getBoxHeight(),heightInBoxes*getBoxHeight()}) {
      mWindow.changed();
      }
    //}}}
    virtual ~cBox() = default;

    //{{{  static gets
    static float getOutlineWidth() { return cWindow::getOutlineWidth(); }
    static float getRoundRadius() { return cWindow::getRoundRadius(); }
    static float getBorderWidth() { return cWindow::getBorderWidth(); }

    static float getBoxHeight() { return cWindow::getBoxHeight(); }
    static cPoint getTextOffset() { return cWindow::getTextOffset(); }
    //}}}
    //{{{  gets
    cWindow& getWindow() { return mWindow; }

    std::string getName() const { return mName; }

    bool getEnable() const { return mEnable; }
    bool getPick() const { return mPick; }
    bool getShow() const { return mEnable && (mPick || mPin); }
    bool getTimedOn() const { return mTimedOn; }

    // get mRect
    float getLeft() const { return mRect.left; }
    float getRight() const { return mRect.right; }
    float getTop() const { return mRect.top; }
    float getBottom() const { return mRect.bottom; }

    cPoint getTL() const { return mRect.getTL(); }
    cPoint getTR() const { return mRect.getTR(); }
    cPoint getBL() const { return mRect.getBL(); }
    cPoint getBR() const { return mRect.getBR(); }

    cPoint getSize() const { return mRect.getSize(); }
    float getWidth() const { return mRect.getWidth(); }
    float getHeight() const { return mRect.getHeight(); }
    cPoint getCentre() const{ return mRect.getCentre(); }

    cPoint getSizeInBoxes() const { return mRect.getSize() / getBoxHeight(); }
    float getWidthInBoxes() const { return mRect.getWidth() / getBoxHeight(); }
    float getHeightInBoxes() const { return mRect.getHeight() / getBoxHeight(); }
    cPoint getCentreInBoxes() const{ return mRect.getCentre() / getBoxHeight(); }

    cRect getRect() const { return mRect; }
    //}}}
    //{{{  sets
    //{{{
    cBox* setPos (float x, float y) {

      mLayoutX = x;
      mLayoutY = y;
      layout();

      return this;
      }
    //}}}
    cBox* setPos (cPoint pos) { return setPos (pos.x, pos.y); }

    //{{{
    cBox* setSize (float x, float y) {

      mLayoutWidth = x;
      mLayoutHeight = y;
      layout();

      return this;
      }
    //}}}
    cBox* setSize (cPoint size) { return setSize (size.x, size.y); }

    cBox* setEnable (bool enable) { mEnable = enable; return this;  }

    cBox* setPickable (bool pickable) { mPickable = pickable;  return this; }
    cBox* setUnPick() { mPick = false;  return this; }

    cBox* setPin (bool pin) { mPin = pin; return this; }
    cBox* togglePin() { mPin = !mPin;  return this; }

    void setSelfSize() { mSelfSize = true; }
    //}}}
    void toTop() { mWindow.toTop (this); }

    // overrides
    //{{{
    virtual void layout() {

      mRect.left = (mLayoutX < 0) ? mWindow.getSize().x + mLayoutX : mLayoutX;
      if (mLayoutWidth > 0)
        mRect.right = mRect.left + mLayoutWidth;
      else if (mLayoutWidth == 0)
        mRect.right = mWindow.getSize().x - mLayoutX;
      else // mLayoutWidth < 0
        mRect.right = mWindow.getSize().x + mLayoutWidth + mLayoutX;

      mRect.top = (mLayoutY < 0) ? mWindow.getSize().y + mLayoutY : mLayoutY;
      if (mLayoutHeight > 0)
        mRect.bottom = mRect.top + mLayoutHeight;
      else if (mLayoutHeight == 0)
        mRect.bottom = mWindow.getSize().y - mLayoutY;
      else // mLayoutHeight < 0
        mRect.bottom = mWindow.getSize().y + mLayoutHeight + mLayoutY;
      }
    //}}}
    virtual void resize() { layout(); }

    //{{{
    virtual bool pick (bool inClient, cPoint pos, bool& change) {

      bool lastPick = mPick;

      mPick = inClient && mRect.inside (pos) & mPickable;
      if (!change && (mPick != lastPick))
        change = true;

      return mPick;
      }
    //}}}
    virtual bool keyDown (int key) { (void)key; return false; }

    virtual bool prox (bool inClient, cPoint pos) { (void)inClient; (void)pos; return false; }
    virtual bool proxExit() { return true; }
    virtual bool proxLift() { return true; }

    virtual bool down (bool right, cPoint pos)  { (void)right; (void)pos; return true; }
    virtual bool move (bool right, cPoint pos, cPoint inc)  { (void)right; (void)pos; (void)inc; return false; }
    virtual bool up (bool right, bool mouseMoved, cPoint pos) { (void)right; (void)mouseMoved; (void)pos; return true; }

    virtual bool wheel (int delta, cPoint pos)  { (void)delta; (void)pos; return true; }

    virtual void draw() = 0;

  protected:
    void changed() { mWindow.changed(); }

    // draws
    //{{{
    void drawRectangle (const cColor& color) {
      mWindow.drawRectangle (color, mRect);
      }
    //}}}
    //{{{
    void drawRectangle (const cColor& color, const cRect& rect) {
      mWindow.drawRectangle (color, rect);
      }
    //}}}
    //{{{
    void drawRectangleUnclipped (const cColor& color, const cRect& rect) {
      mWindow.drawRectangleUnclipped (color, rect);
      }
    //}}}
    //{{{
    void drawBorder (const cColor& color, float width = 2.f) {
      mWindow.drawBorder (color, mRect, width);
      }
    //}}}
    //{{{
    void drawBorder (const cColor& color, const cRect& rect) {
      mWindow.drawBorder (color, rect);
      }
    //}}}
    //{{{
    void drawRounded (const cColor& color) {
      mWindow.drawRounded (color, mRect);
      }
    //}}}
    //{{{
    void drawRounded (const cColor& color, float roundWidth) {
      mWindow.drawRounded (color, mRect, roundWidth);
      }
    //}}}
    //{{{
    void drawRounded (const cColor& color, const cRect& rect) {
      mWindow.drawRounded (color, rect);
      }
    //}}}
    //{{{
    void drawRounded (const cColor& color, const cRect& rect, float roundWidth) {
      mWindow.drawRounded (color, rect, roundWidth);
      }
    //}}}
    //{{{
    void drawEllipse (const cColor& color, cPoint centre, cPoint radius, float width = 0.f) {
      mWindow.drawEllipse (color, centre, radius, width);
      }
    //}}}
    //{{{
    void drawLine (const cColor& color, cPoint point1, cPoint point2, float width) {
      mWindow.drawLine (color, point1, point2, width);
      }
    //}}}

    // string text
    //{{{
    cPoint measureText (const std::string& text, uint32_t font = kMenuFont) {
      return mWindow.measureText (getSize(), text, font);
      }
    //}}}
    //{{{
    cPoint measureText (cPoint size, const std::string& text, uint32_t font = kMenuFont) {
      return mWindow.measureText (size, text, font);
      }
    //}}}
    //{{{
    cPoint drawText (const std::string& text, uint32_t font = kMenuFont) {
      return mWindow.drawText (kTextGray, mRect, text, font);
      }
    //}}}
    //{{{
    cPoint drawText (const cColor& color, const std::string& text, uint32_t font = kMenuFont) {
      return mWindow.drawText (color, mRect, text, font);
      }
    //}}}
    //{{{
    cPoint drawText (const cColor& color, const cRect& rect, const std::string& text, uint32_t font = kMenuFont) {
      return mWindow.drawText (color, rect, text, font);
      }
    //}}}
    //{{{
    cPoint drawTextShadow (const cColor& color, const std::string& text, uint32_t font = kMenuFont) {
      return mWindow.drawTextShadow (color, mRect, text, font);
      }
    //}}}
    //{{{
    cPoint drawTextShadow (const cColor& color, const cRect& rect, const std::string& text, uint32_t font = kMenuFont) {
      return mWindow.drawTextShadow (color, rect, text, font);
      }
    //}}}
    //{{{
    void drawTextRectangle (const cColor& textColor, cColor bgndColor, const std::string& text, uint32_t font = kMenuFont) {

      drawRectangle (bgndColor, mRect);
      drawText (textColor, mRect + getTextOffset(), text, font);
      }
    //}}}
    //{{{
    void drawTextRectangle (const cColor& textColor, cColor bgndColor, const cRect& rect, const std::string& text, uint32_t font = kMenuFont) {

      drawRectangle (bgndColor, rect);
      drawText (textColor, rect + getTextOffset(), text, font);
      }
    //}}}

    //{{{
    void blit (cTexture texture, const cRect& dst) {
    // blit, clipped by box

      mWindow.blit (texture, dst, mRect);
      }
    //}}}
    //{{{
    void blitUnclipped (cTexture texture, const cRect& dst) {
    // blit, unclipped by box

      mWindow.blit (texture, dst);
      }
    //}}}

    // vars
    std::string mName;
    cWindow& mWindow;

    bool mEnable = true;
    bool mPick = false;
    bool mPickable = true;
    bool mPin = true;
    bool mSelfSize = false;
    bool mTimedOn = false;

    float mLayoutWidth;
    float mLayoutHeight;
    float mLayoutX = 0;
    float mLayoutY = 0;

    cRect mRect = { 0.f };
    };
  //}}}
  //{{{  add, remove cBox
  //{{{
  cBox* add (cBox* box) {
    mBoxes.push_back (box);
    box->setPos ({0.f, 0.f});
    return box;
    }
  //}}}
  //{{{
  cBox* add (cBox* box, float x, float y) {

    mBoxes.push_back (box);
    box->setPos (x * getBoxHeight(), y * getBoxHeight());
    return box;
    }
  //}}}

  //{{{
  cBox* addRight (cBox* box) {
    cBox* lastBox = mBoxes.back();
    mBoxes.push_back (box);
    if (lastBox)
      box->setPos (lastBox->getTR());
    return box;
    }
  //}}}
  //{{{
  cBox* addBelow (cBox* box) {

    cBox* lastBox = mBoxes.back();
    mBoxes.push_back (box);
    box->setPos (lastBox->getBL());
    return box;
    }
  //}}}
  //{{{
  cBox* addBR (cBox* box) {

    mBoxes.push_back (box);

    box->setPos (-box->getSizeInBoxes());
    return box;
    }
  //}}}

  //{{{
  cBox* addFront (cBox* box) {

    mBoxes.push_front (box);
    box->setPos (cPoint());
    return box;
    }
  //}}}
  //{{{
  cBox* addFront (cBox* box, float x, float y) {

    mBoxes.push_front (box);
    box->setPos (cPoint(x,y) * getBoxHeight());
    return box;
    }
  //}}}

  //{{{
  cBox* addBackground (cBox* box) {
    mBackgroundBoxes.push_front (box);
    return box;
    }
  //}}}

  //{{{
  void removeBox (cBox* box) {

    for (auto boxIt = mBoxes.begin(); boxIt != mBoxes.end(); ++boxIt)
      if (*boxIt == box) {
        mBoxes.erase (boxIt);
        changed();
        return;
        }
    }
  //}}}

  //{{{
  void toTop (cBox* box) {

    removeBox (box);
    mBoxes.push_back (box);
    }
  //}}}
  //}}}

protected:
  bool createWindow (const std::string& title, uint32_t width, uint32_t height,
                     std::chrono::milliseconds tickMs, bool fullScreen);
  void uiLoop (bool useChanged, bool drawPerf, const cColor& bgndColor, const cColor& perfColor,
               const std::function <void(bool)>& drawCallback = [](bool){});

  virtual bool keyDown (int key) = 0;
  virtual bool keyUp (int key) { (void)key; return false; }

  bool mExit = false;

private:
  //{{{  mouse
  //{{{
  bool mouseProx (bool inClient, cPoint pos) {

    bool change = false;
    auto lastProxBox = mProxBox;

    // search for prox in reverse draw order
    mProxBox = nullptr;
    for (auto boxIt = mBoxes.rbegin(); boxIt != mBoxes.rend(); ++boxIt) {
      bool wasPicked = (*boxIt)->getPick();
      if (!mProxBox && (*boxIt)->getEnable() && (*boxIt)->pick (inClient, pos, change)) {
        mProxBox = *boxIt;
        change |= mProxBox->prox (inClient, pos - mProxBox->getTL());
        }
      else if (wasPicked) {
        (*boxIt)->setUnPick();
        change |= (*boxIt)->proxExit();
        }
      }

    for (auto boxIt = mBackgroundBoxes.rbegin(); boxIt != mBackgroundBoxes.rend(); ++boxIt) {
      bool wasPicked = (*boxIt)->getPick();
      if (!mProxBox && (*boxIt)->getEnable() && (*boxIt)->pick (inClient, pos, change)) {
        mProxBox = *boxIt;
        change |= mProxBox->prox (inClient, pos - mProxBox->getTL());
        }
      else if (wasPicked) {
        (*boxIt)->setUnPick();
        change |= (*boxIt)->proxExit();
        }
      }

    return change || (mProxBox != lastProxBox);
    };
  //}}}
  //{{{
  bool mouseDown (bool right, cPoint pos) {
    mPressedBox = mProxBox;
    return mPressedBox && mPressedBox->down (right, pos - mPressedBox->getTL());
    }
  //}}}
  //{{{
  bool mouseMove (bool right, cPoint pos, cPoint inc) {
    return mPressedBox && mPressedBox->move (right, pos - mPressedBox->getTL(), inc);
    }
  //}}}
  //{{{
  bool mouseUp (bool right, bool mouseMoved, cPoint pos) {
    bool change = mPressedBox && mPressedBox->up (right, mouseMoved, pos - mPressedBox->getTL());
    mPressedBox = nullptr;
    return change;
    }
  //}}}
  //{{{
  bool mouseWheel (int delta, cPoint pos) {
    return mProxBox && mProxBox->wheel (delta, pos- mProxBox->getTL());
    }
  //}}}
  //}}}
  //{{{
  void resizeBoxes() {
    for (auto& box : mBackgroundBoxes)
      box->resize();
    for (auto& box : mBoxes)
      box->resize();
    }
  //}}}
  //{{{
  void drawBoxes() {
    for (auto& box : mBackgroundBoxes)
      if (box->getShow())
        box->draw();

    for (auto& box : mBoxes)
      if (box->getShow())
        box->draw();

    mChanged = false;
    }
  //}}}

  //{{{  static const
  static constexpr float kOutlineWidth = 2.f;
  static constexpr float kRoundRadius = 4.f;
  static constexpr float kBorderWidth = 2.f;
  static constexpr float kBoxHeight = 20.f;
  static constexpr float kConsoleHeight = 12.f;

  inline static const cPoint kTextOffset = {2.f,-1.f};
  //}}}
  //{{{  static vars
  inline static float mBoxHeight = kBoxHeight;
  inline static cPoint mTextOffset = kTextOffset;
  inline static float mConsoleHeight = kConsoleHeight;

  inline static float mOutlineWidth = kOutlineWidth;
  inline static float mRoundRadius = kRoundRadius;
  inline static float mBorderWidth = kBorderWidth;

  inline static int mDayLightSeconds = 0;
  //}}}

  struct sMiniWindow* mWindow = nullptr;

  cPoint mMousePos = {0.f};
  bool mMousePress = false;
  bool mMousePressUsed = false;
  float mScale = 1.0f;

  // boxes
  cBox* mProxBox = nullptr;
  cBox* mPressedBox = nullptr;
  std::deque <cBox*> mBackgroundBoxes;
  std::deque <cBox*> mBoxes;

  // mouse
  bool mMouseTracking = false;
  bool mMouseMoved = false;
  bool mMousePressRight = false;
  cPoint mMouseLastPos;
  cPoint mMousePressPos;

  // key
  int mKeyDown = 0;
  bool mShiftKeyDown = false;
  bool mControlKeyDown = false;

  // render
  bool mChanged = true;
  bool mCursorOn = true;

  uint32_t mCursorDown = 50;
  uint32_t mCursorCountDown = 50;

  uint64_t mRenderUs = 0;
  bool mExitDone = false;

  // screen
  bool mFullScreen = false;
  };
