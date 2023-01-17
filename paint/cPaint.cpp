// cPaint.cpp
//{{{  includes
#include <deque>
#include "../common/basicTypes.h"
#include "../common/cLog.h"

#include "cPaint.h"

using namespace std;
using namespace chrono;
//}}}

// cPaintLayer
//{{{
cPaintLayer::cPaintLayer (cWindow& window, const std::string& name, const cColor& color, cPoint pos, float width)
    : cLayer(window, name, color, {0.f,0.f}), mWidth(width) {
  addPoint (pos);
  }
//}}}
//{{{
void cPaintLayer::addPoint (cPoint pos) {

   if (mLine.empty() || ((pos - mLine.back()).magnitude() > mWidth))
     mLine.push_back (pos);
   }
//}}}
//{{{
bool cPaintLayer::pick (cPoint pos) {
// !!! should be distance to line test !!!

  return mExtent.inside (pos);
  }
//}}}
//{{{
void cPaintLayer::prox (cPoint pos) {
  (void)pos;
  mProx = true;
  }
//}}}
//{{{
void cPaintLayer::proxExit() {
  mProx = false;
  }
//}}}
//{{{
void cPaintLayer::down (cPoint pos) {
  (void)pos;
  mProx = false;
  }
//}}}
//{{{
void cPaintLayer::move (cPoint pos, cPoint inc) {
  (void)pos;
  mPos += inc;
  }
//}}}
//{{{
void cPaintLayer::up (cPoint pos, bool mouseMoved) {
  (void)pos;
  (void)mouseMoved;
  }
//}}}
//{{{
void cPaintLayer::wheel (int delta, cPoint pos) {
  (void)pos;
  mWidth = mWidth * (delta > 0 ? 1.05f : 1/1.05f);
  }
//}}}
//{{{
void cPaintLayer::draw() {

  mExtent = {0,0,0,0};

  if (mLine.empty())
    return;

  cPoint lastPos;
  bool first = true;
  for (auto& pos : mLine) {
    if (first)
      first = false;
    else
      mWindow.drawLine (mProx ? kLightBlue : mColor, mPos + lastPos, mPos+pos, mWidth);
    lastPos = pos;

    mExtent |= mPos + pos;
    }

  first = true;
  for (auto& pos : mLine) {
    if (first) {
      first = false;
      }
    else {
      cPoint perp = (pos - lastPos).perp() * 16.f;
      mWindow.drawLine (kWhite, mPos + pos - perp, mPos + pos + perp, 1.f);
      }
    lastPos = pos;
    }
  }
//}}}

// cStrokeLayer
//{{{
cStrokeLayer::cStrokeLayer (cWindow& window, const std::string& name, const cColor& color, cPoint pos, float width)
    : cLayer(window, name, color, {0.f,0.f}), mWidth(width) {
  addPoint (pos);
  }
//}}}
//{{{
void cStrokeLayer::addPoint (cPoint pos) {

   if (mLine.empty() || ((pos - mLine.back()).magnitude() > mWidth))
     mLine.push_back (pos);
   }
//}}}
//{{{
bool cStrokeLayer::pick (cPoint pos) {
// !!! should be distance to line test !!!

  return mExtent.inside (pos);
  }
//}}}
//{{{
void cStrokeLayer::prox (cPoint pos) {
  (void)pos;
  mProx = true;
  }
//}}}
//{{{
void cStrokeLayer::proxExit() {
  mProx = false;
  }
//}}}
//{{{
void cStrokeLayer::down (cPoint pos) {
  (void)pos;
  mProx = false;
  }
//}}}
//{{{
void cStrokeLayer::move (cPoint pos, cPoint inc) {
  (void)pos;
  mPos += inc;
  }
//}}}
//{{{
void cStrokeLayer::up (cPoint pos, bool mouseMoved) {
  (void)pos;
  (void)mouseMoved;
  }
//}}}
//{{{
void cStrokeLayer::wheel (int delta, cPoint pos) {
  (void)pos;
  mWidth = mWidth * (delta > 0 ? 1.05f : 1/1.05f);
  }
//}}}
//{{{
void cStrokeLayer::draw() {

  mExtent = {0,0,0,0};

  if (mLine.empty())
    return;

  cPoint lastPos;
  bool first = true;
  for (auto& pos : mLine) {
    if (first)
      first = false;
    else
      mWindow.drawLine (mProx ? kLightBlue : mColor, mPos + lastPos, mPos+pos, mWidth);
    lastPos = pos;

    mExtent |= mPos + pos;
    }

  first = true;
  for (auto& pos : mLine) {
    if (first) {
      first = false;
      }
    else {
      cPoint perp = (pos - lastPos).perp() * 16.f;
      mWindow.drawLine (kWhite, mPos + pos - perp, mPos + pos + perp, 1.f);
      }
    lastPos = pos;
    }
  }
//}}}

//{{{  cRectangleLayer
//{{{
bool cRectangleLayer::pick (cPoint pos) {
  cRect r (mPos, mPos + mLength);
  return r.inside (pos);
  }
//}}}
//{{{
void cRectangleLayer::prox (cPoint pos) {
  (void)pos;
  }
//}}}
//{{{
void cRectangleLayer::proxExit() {
  }
//}}}
//{{{
void cRectangleLayer::down (cPoint pos) {
  (void)pos;
  }
//}}}
//{{{
void cRectangleLayer::move (cPoint pos, cPoint inc) {
  (void)pos;
  mPos += inc;
  }
//}}}
//{{{
void cRectangleLayer::up (cPoint pos, bool mouseMoved) {
  (void)pos;
  (void)mouseMoved;
  }
//}}}
//{{{
void cRectangleLayer::wheel (int delta, cPoint pos) {
  (void)pos;
  mLength = mLength * (delta > 0 ? 1.05f : 1/1.05f);
  }
//}}}

//{{{
void cRectangleLayer::draw() {
  mWindow.drawRectangle (mColor, {mPos, mPos + mLength});
  }
//}}}
//}}}
//{{{  cEllipseLayer
//{{{
bool cEllipseLayer::pick (cPoint pos) {
  return (mPos - pos).magnitude() < mRadius;
  }
//}}}
//{{{
void cEllipseLayer::prox (cPoint pos) {
  (void)pos;
  }
//}}}
//{{{
void cEllipseLayer::proxExit() {
  }
//}}}
//{{{
void cEllipseLayer::down (cPoint pos) {
  (void)pos;
  }
//}}}
//{{{
void cEllipseLayer::move (cPoint pos, cPoint inc) {
  (void)pos;
  mPos += inc;
  }
//}}}
//{{{
void cEllipseLayer::up (cPoint pos, bool mouseMoved) {
  (void)pos;
  (void)mouseMoved;
  }
//}}}
//{{{
void cEllipseLayer::wheel (int delta, cPoint pos) {
  (void)pos;
  mRadius += delta * 1.05f;
  }
//}}}

//{{{
void cEllipseLayer::draw() {
  mWindow.drawEllipse (mColor, mPos, mRadius, mWidth);
  }
//}}}
//}}}
//{{{  cTextLayer
//{{{
bool cTextLayer::pick (cPoint pos) {
  cRect r(mPos, mPos + mLength);
  return r.inside(pos);
}
//}}}
//{{{
void cTextLayer::prox (cPoint pos) {
  (void)pos;
  }
  //}}}
//{{{
void cTextLayer::proxExit() {
  }
//}}}
//{{{
void cTextLayer::down (cPoint pos) {
  (void)pos;
  }
//}}}
//{{{
void cTextLayer::move (cPoint pos, cPoint inc) {
  (void)pos;
  mPos += inc;
  }
//}}}
//{{{
void cTextLayer::up (cPoint pos, bool mouseMoved) {
  (void)pos;
  (void)mouseMoved;
  }
//}}}
//{{{
void cTextLayer::wheel (int delta, cPoint pos) {
  (void)delta;
  (void)pos;
  }
//}}}

//{{{
void cTextLayer::draw() {
  mLength = mWindow.drawText (mColor, cRect (mPos, mWindow.getSize()), mText);
  }
//}}}
//}}}
//{{{  cTextureLayer
//{{{
bool cTextureLayer::pick (cPoint pos) {
  return mExtent.inside (pos);
  }
//}}}

//{{{
void cTextureLayer::prox (cPoint pos) {
  (void)pos;
  }
//}}}
//{{{
void cTextureLayer::proxExit() {
  }
//}}}
//{{{
void cTextureLayer::down (cPoint pos) {
  (void)pos;
  }
//}}}
//{{{
void cTextureLayer::move (cPoint pos, cPoint inc) {
  (void)pos;
  mPos += inc;
  }
//}}}
//{{{
void cTextureLayer::up (cPoint pos, bool mouseMoved) {
  (void)pos;
  (void)mouseMoved;
  }
//}}}
//{{{
void cTextureLayer::wheel (int delta, cPoint pos) {
  (void)pos;
  mSize *= delta > 0 ? 1.05f : 1/1.05f;
  }
//}}}

//{{{
void cTextureLayer::draw() {

  mWindow.blitAffine (mTexture, mWindow.getSize(), mSize, mAngle, mPos.x, mPos.y);
  mExtent = {mPos - ((mTexture.getSize() / 2.f) * mSize), mPos + ((mTexture.getSize() /2.f) * mSize)};
  }
//}}}
//}}}

// cPaint
//{{{
cPaint::cPaint (cWindow& window) : mWindow(window) {
  }
//}}}
//{{{
 cPaint::~cPaint() {
  // deallocate layers
  }
//}}}
//{{{
cLayer* cPaint::addLayer (cLayer* layer) {
  mLayers.push_back (layer);
  return layer;
  }
//}}}
//{{{
bool cPaint::pick (cPoint pos) {

  for (auto layerIt = mLayers.rbegin(); layerIt != mLayers.rend(); ++layerIt) {
    if ((*layerIt)->pick (pos)) {
      mPickedLayer = *layerIt;
      return true;
      }
    }

  mPickedLayer = nullptr;
  return false;
  }
//}}}
//{{{
bool cPaint::prox (cPoint pos) {
  if (mPickedLayer)
    mPickedLayer->prox (pos);
  return true;
  }
//}}}
//{{{
bool cPaint::proxExit() {
  if (mPickedLayer)
    mPickedLayer->proxExit();
  return true;
  }
//}}}
//{{{
bool cPaint::down (cPoint pos) {

  if (mPainting)
    mPickedLayer = addLayer (new cPaintLayer (mWindow, "paint", kYellow, pos, 4.f));
  else if (mStroking)
    mPickedLayer = addLayer (new cStrokeLayer (mWindow, "stroke", kGreen, pos, 4.f));
  else if (mPickedLayer)
    mPickedLayer->down (pos);

  return true;
  }
//}}}
//{{{
bool cPaint::move (cPoint pos, cPoint inc) {

  if (mPainting || mStroking)
    mPickedLayer->addPoint (pos);
  else if (mPickedLayer)
    mPickedLayer->move (pos, inc);

  return true;
  }
//}}}
//{{{
bool cPaint::up (cPoint pos, bool mouseMoved){

  if (mPainting || mStroking) {
    }
  else if (mPickedLayer)
    mPickedLayer->up (pos, mouseMoved);

  return true;
  }
//}}}
//{{{
bool cPaint::wheel (int delta, cPoint pos) {

  if (mPickedLayer)
    mPickedLayer->wheel (delta, pos);

  return true;
  }
//}}}
//{{{
void cPaint::draw() {
  for (auto layer : mLayers)
    layer->draw();
  }
//}}}
