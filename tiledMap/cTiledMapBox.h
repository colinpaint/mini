// cTiledMapBox.h
#pragma once
#include "../gui/cWindow.h"

class cTiledMap;
class cTiledMapBox : public cWindow::cBox {
public:
  cTiledMapBox(cWindow& window, float width, float height, cTiledMap& tiledMap);
  virtual ~cTiledMapBox() = default;

  virtual bool move (bool right, cPoint pos, cPoint inc, int pressure, int timestamp) final;
  virtual bool wheel (int delta, cPoint pos) final;
  virtual void draw() final;

private:
  cTiledMap& mTiledMap;
  };
