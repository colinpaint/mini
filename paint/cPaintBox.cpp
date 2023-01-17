// cPaintBox.cpp
#include "../common/basicTypes.h"
#include "../gui/cWindow.h"
#include "../gui/basicBoxes.h"

#include "cPaintBox.h"

//{{{
cPaintBox::cPaintBox (cWindow& window, float width, float height, cPaint& paint)
    : cBox("paint", window, width, height), mPaint(paint) {

  mPaintBox = mWindow.add (new cToggleBox (window, 6,1, "paint", mPaint.mPainting));
  }
//}}}

//{{{
bool cPaintBox::pick (bool inClient, cPoint pos, bool& change) {
  (void) inClient;

  if (mPaint.pick (pos)) {
    change = true;
    return true;
    }

  // !!! is this right !!!
  return mRect.inside (pos);
  }
//}}}
//{{{
bool cPaintBox::prox (bool inClient, cPoint pos) {
  (void) inClient;
  (void) pos;
  return true;
  }
//}}}
//{{{
bool cPaintBox::proxExit() {
  return true;
  }
//}}}
//{{{
bool cPaintBox::down (bool right, cPoint pos) {
  (void) right;

  mPaint.down (pos);

  return true;
  }
//}}}
//{{{
bool cPaintBox::move (bool right, cPoint pos, cPoint inc) {
  (void) right;

  mPaint.move (pos, inc);

  return true;
  }
//}}}
//{{{
bool cPaintBox::up (bool right, bool mouseMoved, cPoint pos) {
  (void) right;

  mPaint.up (pos, mouseMoved);
  return true;
  }
//}}}

//{{{
bool cPaintBox::wheel (int delta, cPoint pos) {
  return mPaint.wheel (delta, pos);
  }
//}}}

//{{{
void cPaintBox::draw() {

  mPaint.draw (mWindow);

  //cPoint pos = getTL() + mPos;

  //mWindow.addEllipse (pos + cPoint(100.f,100.f), 50.f * mScale, 4.f);
  //mWindow.drawEdges (kGreen);

  //mWindow.addTriangle (pos, pos + cPoint (50.f, 100.f) * mScale, pos + cPoint (-50.f, 110.f) * mScale);
  //mWindow.drawEdges (kYellow);

  //mWindow.addLine (pos, pos + cPoint (70.f, 70.f) * mScale);
  //mWindow.drawEdges (kWhite);

  //mWindow.addArrowHead (pos, pos + cPoint (-70.f, 70.f) * mScale);
  //mWindow.drawEdges (kGray);

  //mWindow.addEllipse (pos + cPoint(150.f,150.f), 50.f * mScale);
  //mWindow.drawEdges (kGreen);

  //mWindow.drawGradRadial (kWhite, pos + cPoint(150.f,150.f), cPoint (50.f, 50.f) * mScale);
  }
//}}}
