// cTiledMapViewBox.h
#pragma once
#include "../gui/cWindow.h"
class cTiledMap;

class cTiledMapOverviewBox : public cWindow::cBox {
public:
  cTiledMapOverviewBox (cWindow& window, float width, float height, cTiledMap& map);
  virtual ~cTiledMapOverviewBox() = default;

  virtual bool down (bool right, cPoint pos) final;
  virtual bool move (bool right, cPoint pos, cPoint inc, int pressure, int timestamp) final;
  virtual bool wheel (int delta, cPoint pos) final;
  virtual void draw() final;

private:
  void setLatLon (cPoint pos);

  cTiledMap& mMap;
  };
