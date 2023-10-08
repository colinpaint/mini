// basicBoxes.h
#pragma once
#include <iostream>
#include <functional>
#include "../common/date.h"
#include "../common/basicTypes.h"
#include "../common/cLog.h"
#include "cWindow.h"

class cWindowBox : public cWindow::cBox {
public:
  //{{{
  cWindowBox (cWindow& window, float width, float height)
      : cBox("window", window, width, height) {
    setSelfSize();
    }
  //}}}
  virtual ~cWindowBox() = default;

  //{{{
  virtual bool down (bool right, cPoint pos) final {
    (void)right;
    (void)pos;
    if (pos.x < getWidth()/2)
      mWindow.toggleFullScreen();
    else
      mWindow.setExit();
    return true;
    }
  //}}}
  //{{{
  virtual void resize() final {
    layout();
    mEnable = mWindow.getFullScreen();
    }
  //}}}

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
