// basicBoxes.h
#pragma once
#include <iostream>
#include <functional>
#include "../common/basicTypes.h"
#include "../common/cLog.h"
#include "cWindow.h"

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
