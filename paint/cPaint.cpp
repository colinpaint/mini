// cPaint.cpp
//{{{  includes
#include <deque>
#include "../common/basicTypes.h"
#include "../common/cLog.h"

#include "cPaint.h"

using namespace std;
using namespace chrono;
//}}}

//{{{  cPaintLayer
//{{{
cPaintLayer::cPaintLayer (const std::string& name, const cColor& color, cPoint pos, float width)
    : cLayer(name, color, {0.f,0.f}), mWidth(width) {
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
  return mExtent.inside (pos);
  }
//}}}
//{{{
void cPaintLayer::prox (cPoint pos) {
  (void)pos;
  }
//}}}
//{{{
void cPaintLayer::proxExit() {
  }
//}}}
//{{{
void cPaintLayer::down (cPoint pos) {
  (void)pos;
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
void cPaintLayer::draw (cWindow& window) {

  mExtent = {0,0,0,0};

  if (mLine.empty())
    return;

  cPoint lastPos;
  bool first = true;
  for (auto& pos : mLine) {
    if (first)
      first = false;
    else
      window.drawLine (mColor, mPos + lastPos, mPos+pos, mWidth);
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
      window.drawLine (kWhite, pos - perp, pos + perp, 1.f);
      }
    lastPos = pos;
    }
  }
//}}}
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
void cRectangleLayer::draw (cWindow& window) {
  window.drawRectangle (mColor, {mPos, mPos + mLength});
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
void cEllipseLayer::draw (cWindow& window) {
  window.drawEllipse (mColor, mPos, mRadius, mWidth);
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
void cTextLayer::draw (cWindow& window) {
  mLength = window.drawText (mColor, cRect (mPos, window.getSize()), mText);
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
void cTextureLayer::draw (cWindow& window) {

  window.blitAffine (mTexture, window.getSize(), mSize, mAngle, mPos.x, mPos.y);
  mExtent = {mPos - ((mTexture.getSize() / 2.f) * mSize), mPos + ((mTexture.getSize() /2.f) * mSize)};
  }
//}}}
//}}}

// cPaint
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
    mPickedLayer = addLayer (new cPaintLayer ("paint", kYellow, pos, 4.f));
  else if (mPickedLayer)
    mPickedLayer->down (pos);

  return true;
  }
//}}}
//{{{
bool cPaint::move (cPoint pos, cPoint inc) {

  if (mPainting)
    mPickedLayer->addPoint (pos);
  else if (mPickedLayer)
    mPickedLayer->move (pos, inc);

  return true;
  }
//}}}
//{{{
bool cPaint::up (cPoint pos, bool mouseMoved){

  if (mPainting) {
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
void cPaint::draw (cWindow& window) {
  for (auto layer : mLayers)
    layer->draw (window);
  }
//}}}
