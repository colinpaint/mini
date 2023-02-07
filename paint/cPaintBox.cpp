// cPaintBox.cpp
#include "cPaintBox.h"

#include "../gui/basicBoxes.h"
#include "cPaint.h"

//{{{
cPaintBox::cPaintBox (cWindow& window, float width, float height, cPaint& paint)
    : cBox("paint", window, width, height), mPaint(paint) {

  // !!! need proper interlock !!!
  mPaintBox = mWindow.add (new cToggleBox (window, 6,1, "paint", mPaint.mPainting, [&](bool on) { if (on) mPaint.mStroking = false; }));
  mStrokeBox = mWindow.add (new cToggleBox (window, 6,1, "stroke", mPaint.mStroking, [&](bool on) {if (on) mPaint.mPainting = false;}), 0,1);
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
  mPaint.prox (pos);
  return true;
  }
//}}}
//{{{
bool cPaintBox::proxExit() {
  mPaint.proxExit();
  return true;
  }
//}}}
//{{{
bool cPaintBox::proxLift() {
  mPaint.proxLift();
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
bool cPaintBox::move (bool right, cPoint pos, cPoint inc, int pressure, int timestamp) {
  (void) right;
  (void) pressure;
  (void) timestamp;

  mPaint.move (pos, inc, pressure, timestamp);

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

  mPaint.draw();

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
