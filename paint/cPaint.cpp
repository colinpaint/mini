// cPaint.cpp
//{{{  includes
#include <deque>
#include "../common/basicTypes.h"
#include "../common/cLog.h"

#include "../gui/cTexture.h"
#include "cPaint.h"

using namespace std;
using namespace chrono;
//}}}

// cPaintLayer
//{{{
cPaintLayer::cPaintLayer (const std::string& name, const cColor& color, cPoint pos, float width)
    : cLayer(name, color, {0.f,0.f}), mWidth(width) {

  setRadius (width);
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
void cPaintLayer::proxLift() {
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
void cPaintLayer::draw (cWindow& window) {

  mExtent = {0,0,0,0};

  if (mLine.empty())
    return;

  bool first = true;
  for (auto& pos : mLine) {
    paint (window, kYellow, mPos + pos, first);
    mExtent |= mPos + pos;
    first = false;
    }
  }
//}}}

//{{{
uint8_t cPaintLayer::getPaintShape (float i, float j, float radius) {
  return static_cast<uint8_t>(255.f * (1.f - clamp (sqrtf((i*i) + (j*j)) - radius, 0.f, 1.f)));
  }
//}}}
//{{{
void cPaintLayer::setRadius (float radius) {

  mRadius = radius;
  mShapeRadius = static_cast<unsigned>(ceil(radius));
  mShapeSize = (2 * mShapeRadius) + 1;

  mSubPixels = 4;
  mSubPixelResolution = 1.f / mSubPixels;

  free (mShape);
  mShape = static_cast<uint8_t*>(malloc (mSubPixels * mSubPixels  * mShapeSize * mShapeSize));

  auto shape = mShape;
  for (int ySub = 0; ySub < mSubPixels; ySub++)
    for (int xSub = 0; xSub < mSubPixels; xSub++)
      for (int j = -mShapeRadius; j <= mShapeRadius; j++)
        for (int i = -mShapeRadius; i <= mShapeRadius; i++)
          *shape++ = getPaintShape (i - (xSub * mSubPixelResolution), j - (ySub * mSubPixelResolution), mRadius);
  }
//}}}
//{{{
void cPaintLayer::stamp (cWindow& window, const cColor& color, cPoint pos) {
// stamp brushShape into image, clipped by width,height to pos, update mPrevPos

  int32_t width = window.getWidth();
  int32_t height = window.getHeight();

  // x
  int32_t xInt = static_cast<int32_t>(pos.x);
  int32_t leftClipShape = -min(0, xInt - mShapeRadius);
  int32_t rightClipShape = max(0, xInt + mShapeRadius + 1 - width);
  int32_t xFirst = xInt - mShapeRadius + leftClipShape;
  float xSubPixelFrac = pos.x - xInt;

  // y
  int32_t yInt = static_cast<int32_t>(pos.y);
  int32_t topClipShape = -min(0, yInt - mShapeRadius);
  int32_t botClipShape = max(0, yInt + mShapeRadius + 1 - height);
  int32_t yFirst = yInt - mShapeRadius + topClipShape;
  float ySubPixelFrac = pos.y - yInt;

  // point to first image pix
  uint8_t* frame = (uint8_t*)window.getPixels (xFirst, yFirst);
  int32_t frameRowInc = (width - mShapeSize + leftClipShape + rightClipShape) * 4;

  int32_t xSub = static_cast<int>(xSubPixelFrac / mSubPixelResolution);
  int32_t ySub = static_cast<int>(ySubPixelFrac / mSubPixelResolution);

  // point to first clipped shape pix
  uint8_t* shape = mShape;
  shape += ((ySub * mSubPixels) + xSub) * mShapeSize * mShapeSize;
  shape += (topClipShape * mShapeSize) + leftClipShape;

  int shapeRowInc = rightClipShape + leftClipShape;

  cTexture::uPixel colorPixel (color);
  for (int32_t j = -mShapeRadius + topClipShape; j <= mShapeRadius - botClipShape; j++) {
    for (int32_t i = -mShapeRadius + leftClipShape; i <= mShapeRadius - rightClipShape; i++) {
      uint16_t foreground = *shape++;
      if (foreground > 0) {
        // stamp some foreground
        //foreground = (foreground * colorPixel.rgba.a) / 255;
        uint8_t pressure = 128;
        foreground = (foreground * pressure) / 255;
        if (foreground >= 255) {
          // all foreground
          *frame++ = colorPixel.rgba.r;
          *frame++ = colorPixel.rgba.g;
          *frame++ = colorPixel.rgba.b;
          *frame++ = colorPixel.rgba.a;
          }
        else {
          // blend foreground into background
          uint16_t background = 255 - foreground;
          uint16_t r = (colorPixel.rgba.r * foreground) + (*frame * background);
          *frame++ = (uint8_t)(r / 255);
          uint16_t g = (colorPixel.rgba.g * foreground) + (*frame * background);
          *frame++ = (uint8_t)(g / 255);
          uint16_t b = (colorPixel.rgba.b * foreground) + (*frame * background);
          *frame++ = (uint8_t)(b / 255);
          uint16_t a = (colorPixel.rgba.a * foreground) + (*frame * background);
          *frame++ = (uint8_t)(a / 255);
          }
        }
      else
        frame += 4;
      }

    // onto next row
    frame += frameRowInc;
    shape += shapeRowInc;
    }

  mPrevPos = pos;
  }
//}}}
//{{{
void cPaintLayer::paint (cWindow& window, const cColor& color, cPoint pos, bool first) {

  if (first)
    stamp (window, color, pos);
  else {
    // draw stamps from mPrevPos to pos
    cPoint diff = pos - mPrevPos;
    float length = diff.magnitude();
    float overlap = mRadius / 2.f;

    if (length >= overlap) {
      cPoint inc = diff * (overlap / length);

      unsigned numStamps = static_cast<unsigned>(length / overlap);
      for (unsigned i = 0; i < numStamps; i++)
        stamp (window, color, mPrevPos + inc);
      }
    }
  }
//}}}

//{{{  cStrokeLayer
//{{{
cStrokeLayer::cStrokeLayer (const std::string& name, const cColor& color, cPoint pos, float width)
    : cLayer(name, color, {0.f,0.f}), mWidth(width) {
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
void cStrokeLayer::proxLift() {
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
void cStrokeLayer::draw (cWindow& window) {

  mExtent = {0,0,0,0};

  if (mLine.empty())
    return;

  cPoint lastPos;
  bool first = true;
  for (auto& pos : mLine) {
    if (first)
      first = false;
    else
      window.drawLine (mProx ? kLightBlue : mColor, mPos + lastPos, mPos+pos, mWidth);
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
      window.drawLine (kWhite, mPos + pos - perp, mPos + pos + perp, 1.f);
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
void cRectangleLayer::proxLift() {
  mProx = false;
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
void cEllipseLayer::proxLift() {
  mProx = false;
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
void cTextLayer::proxLift() {
  mProx = false;
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
void cTextureLayer::proxLift() {
  mProx = false;
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
bool cPaint::proxLift() {
  if (mPickedLayer)
    mPickedLayer->proxLift();
  return true;
  }
//}}}
//{{{
bool cPaint::down (cPoint pos) {

  if (mPainting)
    mPickedLayer = addLayer (new cPaintLayer ("paint", kYellow, pos, 12.f));
  else if (mStroking)
    mPickedLayer = addLayer (new cStrokeLayer ("stroke", kGreen, pos, 4.f));
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
    layer->draw (mWindow);
  }
//}}}
