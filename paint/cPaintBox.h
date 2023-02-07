// cLayersBox.h
#pragma once
#include "../common/basicTypes.h"
#include "../gui/cWindow.h"

class cPaint;
class cPaintBox : public cWindow::cBox {
public:
  cPaintBox(cWindow& window, float width, float height, cPaint& mPaint);
  virtual ~cPaintBox() = default;

  virtual bool pick (bool inClient, cPoint pos, bool& change) final;

  virtual bool prox (bool inClient, cPoint pos) final;
  virtual bool proxExit() final;
  virtual bool proxLift() final;

  virtual bool down (bool right, cPoint pos) final;
  virtual bool move (bool right, cPoint pos, cPoint inc, int pressure, int timestamp) final;
  virtual bool up (bool right, bool mouseMoved, cPoint pos) final;
  virtual bool wheel (int delta, cPoint pos) final;

  virtual void draw() final;

private:
  cPaint& mPaint;
  cBox* mPaintBox = nullptr;
  cBox* mStrokeBox = nullptr;
  };
